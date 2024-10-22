//
// @author magicxqq <xqq@xqq.im>
//

#include <cassert>
#include <future>
#include <functional>
#include <cpr/cpr.h>
#include "log.hpp"
#include "scope_guard.hpp"
#include "string_utils.hpp"
#include "stream_loader.hpp"

using namespace std::placeholders;

StreamLoader::StreamLoader(size_t chunk_size, size_t max_chunk_count, size_t min_chunk_count) :
        chunk_size_(chunk_size), blocking_buffer_(chunk_size, max_chunk_count, min_chunk_count) {
    assert(!has_requested_ && "Once requested StreamLoader cannot be reused!");
}

StreamLoader::~StreamLoader() {
    if (IsPolling()) {
        Abort();
    }
}

bool StreamLoader::Open(const std::string& base_url,
                        const std::string& path_query,
                        std::optional<BasicAuth> basic_auth,
                        std::optional<std::string> user_agent,
                        std::optional<std::string> proxy,
                        std::optional<std::map<std::string, std::string>> headers) {
    std::string url = base_url + path_query;
    Log::InfoF("StreamLoader::Open(): Opening %s", url.c_str());

    session_.SetUrl(cpr::Url{url});

    if (basic_auth) {
        session_.SetAuth(cpr::Authentication{basic_auth->user, basic_auth->password});
    }

    if (user_agent) {
        session_.SetUserAgent({user_agent.value()});
    }

    if (proxy) {
        session_.SetProxies({{"http", proxy.value()},
                             {"https", proxy.value()}});
    }

    if (headers) {
        for (const auto& pair : headers.value()) {
            cpr::Header header{{pair.first, pair.second}};
            session_.UpdateHeader(header);
        }
    }

    session_.SetHeaderCallback(cpr::HeaderCallback(std::bind(&StreamLoader::OnHeaderCallback, this, _1)));
    session_.SetWriteCallback(cpr::WriteCallback(std::bind(&StreamLoader::OnWriteCallback, this, _1)));

    auto holder = session_.GetCurlHolder();
    CURL* curl = holder->handle;
    curl_easy_setopt(curl, CURLOPT_OPENSOCKETFUNCTION, &StreamLoader::OnOpenSocketCallback);
    curl_easy_setopt(curl, CURLOPT_OPENSOCKETDATA, this);

    has_requested_ = true;

    async_response_ = std::async(std::launch::async, [this] {
        cpr::Response response = session_.Get();
        bool has_error = false;

        if (socket_ == INVALID_SOCKET) {
            Log::Info("StreamLoader::Open(): curl socket has been force closed by Abort()");
        } else if (response.error && response.error.code != cpr::ErrorCode::REQUEST_CANCELLED) {
            has_error = true;
            Log::ErrorF("StreamLoader::Open(): curl failed with error_code: %d, msg = %s",
                        response.error.code,
                        response.error.message.c_str());
        } else if (response.status_code >= 400) {
            has_error = true;
            Log::ErrorF("StreamLoader::Open(): Invalid status code: %d, body = %s",
                        response.status_code,
                        response.text.c_str());
        }

        if (has_error) {
            std::lock_guard lock(response_mutex_);
            has_response_received_ = true;
            request_failed_ = true;
            response_cv_.notify_all();
        }

        if (!has_error && !has_requested_abort_) {
            Log::InfoF(LOG_FILE_MESSAGE("curl_easy_perform returned, pulling completed"));
            has_reached_eof_ = true;
        }

        return response;
    });

    return true;
}

StreamLoader::WaitResult StreamLoader::WaitForResponse(std::chrono::milliseconds timeout) {
    std::unique_lock lock(response_mutex_);

    if (!has_requested_) {
        return WaitResult::kWaitFailed;
    }

    if (has_response_received_ && !request_failed_) {
        return WaitResult::kResultOK;
    }

    bool pred = response_cv_.wait_for(lock, timeout, [this] {
        return has_response_received_;
    });

    if (!pred) {
        return WaitResult::kWaitTimeout;
    }

    // [has_response_received_] should be true here

    if (request_failed_) {
        return WaitResult::kResultFailed;
    }

    return WaitResult::kResultOK;
}

StreamLoader::WaitResult StreamLoader::WaitForData() {
    if (!has_requested_) {
        return WaitResult::kWaitFailed;
    }

    if (request_failed_) {
        return WaitResult::kResultFailed;
    }

    blocking_buffer_.WaitUntilData();

    if (request_failed_) {
        return WaitResult::kResultFailed;
    }

    return WaitResult::kResultOK;
}

curl_socket_t StreamLoader::OnOpenSocketCallback(StreamLoader* self, curlsocktype purpose, curl_sockaddr* addr) {
    SOCKET sock = socket(addr->family, addr->socktype, addr->protocol);
    self->socket_ = sock;
    return sock;
}

void StreamLoader::ForceShutdown() {
    if (socket_ != INVALID_SOCKET) {
        shutdown(socket_, SD_BOTH);
        closesocket(socket_);
        socket_ = INVALID_SOCKET;
    }
}

bool StreamLoader::OnHeaderCallback(std::string data) {
    if (has_requested_abort_) {
        // return false to cancel the transfer
        return false;
    }

    if (has_response_received_) {
        // status_code received, ignore subsequent callback
        return true;
    }

    ON_SCOPE_EXIT {
        // Acquire mutex for WaitForResponse()
        std::lock_guard lock(response_mutex_);
        has_response_received_ = true;
        // Notify WaitForResponse()
        response_cv_.notify_all();
    };

    auto holder = session_.GetCurlHolder();
    CURL* curl = holder->handle;

    long status_code = 0;
    CURLcode ret = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);

    // 40x error, set request_failed_ to false and cancel transfer
    if (ret == CURLcode::CURLE_OK && status_code >= 400) {
        Log::ErrorF("StreamLoader::OnHeaderCallback(): Invalid status code: %d", status_code);
        request_failed_ = true;
        return false;
    }

    Log::InfoF("StreamLoader::OnHeaderCallback(): Received response code: %d, start polling", status_code);
    // 20x OK, notify WaitForResponse and continue transfer
    return true;
}

bool StreamLoader::OnWriteCallback(std::string data) {
    if (has_requested_abort_) {
        // return false to cancel the transfer
        return false;
    }

    speed_sampler_.AddBytes(data.size());

    blocking_buffer_.Write(reinterpret_cast<uint8_t*>(data.data()), data.size());

    return true;
}

void StreamLoader::Abort() {
    Log::InfoF("StreamLoader::Abort(): Aborting");
    has_requested_abort_ = true;
    blocking_buffer_.NotifyExit();

    if (!has_response_received_) {
        // If server hasn't returned any response, force kill the underlying socket
        ForceShutdown();
    }

    async_response_.wait();
}

size_t StreamLoader::Read(uint8_t* buffer, size_t expected_bytes) {
    size_t bytes_read = blocking_buffer_.Read(buffer, expected_bytes);
    return bytes_read;
}

std::pair<uint8_t*, size_t> StreamLoader::ReadChunkAndRetain() {
    return blocking_buffer_.ReadChunkAndRetain();
}

size_t StreamLoader::RemainReadable() {
    return blocking_buffer_.ReadableBytes();
}

bool StreamLoader::IsPolling() {
    return has_requested_ && !request_failed_ && !has_reached_eof_ && !has_requested_abort_;
}

float StreamLoader::GetCurrentSpeedKByte() {
    return speed_sampler_.LastSecondKBps();
}
