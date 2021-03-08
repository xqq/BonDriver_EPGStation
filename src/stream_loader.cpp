//
// @author magicxqq <xqq@xqq.im>
//

#include <vector>
#include <future>
#include <functional>
#include <cpr/cpr.h>
#include "log.hpp"
#include "scope_guard.hpp"
#include "utils.hpp"
#include "stream_loader.hpp"

using namespace std::placeholders;

StreamLoader::StreamLoader(size_t chunk_size, size_t max_chunk_count, size_t min_chunk_count) :
        chunk_size_(chunk_size), blocking_buffer_(chunk_size, max_chunk_count, min_chunk_count) {}

StreamLoader::~StreamLoader() {
    if (has_response_received_ && !has_reached_eof_ && !has_requested_abort_) {
        Abort();
    }
}

bool StreamLoader::Open(const std::string& base_url, const std::string& path_query, std::optional<BasicAuth> basic_auth) {
    session_.SetUrl(cpr::Url{base_url + path_query});

    if (basic_auth) {
        session_.SetAuth(cpr::Authentication{basic_auth->user, basic_auth->password});
    }

    session_.SetHeaderCallback(cpr::HeaderCallback(std::bind(&StreamLoader::OnHeaderCallback, this, _1)));
    session_.SetWriteCallback(cpr::WriteCallback(std::bind(&StreamLoader::OnWriteCallback, this, _1)));

    async_response_ = std::async(std::launch::async, [this] {
        cpr::Response response = session_.Get();
        bool has_error = false;

        if (response.error && response.error.code != cpr::ErrorCode::REQUEST_CANCELLED) {
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

bool StreamLoader::WaitForResponse() {
    std::unique_lock lock(response_mutex_);
    if (has_response_received_ && !request_failed_) {
        return true;
    }

    response_cv_.wait(lock, [this] {
        return has_response_received_;
    });

    // If [has_response_received_] and [not request_failed_], that's succeed
    return !request_failed_;
}

bool StreamLoader::WaitForData() {
    if (request_failed_) {
        return false;
    }

    blocking_buffer_.WaitUntilData();

    return !request_failed_;
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

    ON_SCOPE_EXIT{
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
    if (ret == CURLE_OK && status_code >= 400) {
        Log::ErrorF("StreamLoader::OnHeaderCallback(): Invalid status code: %d", status_code);
        request_failed_ = true;
        return false;
    }

    // 20x OK, notify WaitForResponse and continue transfer
    return true;
}

bool StreamLoader::OnWriteCallback(std::string data) {
    if (has_requested_abort_) {
        // return false to cancel the transfer
        return false;
    }

    blocking_buffer_.Write(reinterpret_cast<uint8_t*>(data.data()), data.size());
    return true;
}

void StreamLoader::Abort() {
    has_requested_abort_ = true;
    blocking_buffer_.NotifyExit();
    async_response_.wait();
}

size_t StreamLoader::Read(uint8_t* buffer, size_t expected_bytes) {
    size_t bytes_read = blocking_buffer_.Read(buffer, expected_bytes);
    return bytes_read;
}

size_t StreamLoader::RemainReadable() {
    return blocking_buffer_.ReadableBytes();
}
