//
// @author magicxqq <xqq@xqq.im>
//

#include <vector>
#include <cpprest/http_client.h>
#include "log.hpp"
#include "scope_guard.hpp"
#include "utils.hpp"
#include "stream_loader.hpp"

using namespace web::http;
using namespace web::http::client;

StreamLoader::StreamLoader(size_t chunk_size, size_t max_chunk_count, size_t min_chunk_count) :
        chunk_size_(chunk_size), blocking_buffer_(chunk_size, max_chunk_count, min_chunk_count),
        has_response_received_(false), has_reached_eof_(false), request_failed_(false) { }

StreamLoader::~StreamLoader() {
    if (has_response_received_ && !has_reached_eof_ && !cancel_token_source_.get_token().is_canceled()) {
        Abort();
    }
}

bool StreamLoader::Open(const std::string &base_url, const std::string &path_query, std::optional<BasicAuth> basic_auth) {
    http_client_config client_config;

    if (basic_auth.has_value()) {
        web::credentials credentials(UTF8ToPlatformString(basic_auth->user), UTF8ToPlatformString(basic_auth->password));
        client_config.set_credentials(credentials);
    }

    http_client client(UTF8ToPlatformString(base_url), client_config);

    client.request(methods::GET, UTF8ToPlatformString(path_query), cancel_token_source_.get_token())
    .then([this](const http_response& response) {
        ON_SCOPE_EXIT {
            // Acquire mutex for WaitForResponse()
            std::lock_guard lock(response_mutex_);
            has_response_received_ = true;
            // Notify WaitForResponse()
            response_cv_.notify_all();
        };

        // If cancelled, return empty task
        if (cancel_token_source_.get_token().is_canceled()) {
            return pplx::create_task([] {});
        }

        // status_code == 200, OK
        if (response.status_code() == status_codes::OK) {
            pplx::streams::istream body_stream = response.body();
            if (body_stream.is_valid()) {
                return Pump(body_stream, chunk_size_);
            } else {
                Log::ErrorF(LOG_FILE_MESSAGE("StreamLoader::Open(): Invalid body_stream"));
                request_failed_ = true;
            }
        } else {
            Log::ErrorF("StreamLoader::Open(): Invalid status code: %d", response.status_code());
            request_failed_ = true;
        }

        return pplx::create_task([] {});
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

pplx::task<void> StreamLoader::Pump(pplx::streams::istream body_stream, size_t chunk_size) {
    if (cancel_token_source_.get_token().is_canceled()) {
        return pplx::create_task([] {});
    }

    StreamLoader* self = this;

    pplx::task<void> task = pplx::create_task([=] {
        pplx::streams::container_buffer<std::vector<uint8_t>> container_buf;
        pplx::task<size_t> read_task = body_stream.read(container_buf, chunk_size);

        size_t bytes_read = 0;

        try {
            bytes_read = read_task.get();
        } catch (const std::exception& ex) {
            Log::ErrorF(LOG_FILE_MESSAGE(ex.what()));
            return pplx::create_task([] {});
        }

        // Log::InfoF("Pump(): chunk_size = %zu, bytes_read = %zu", chunk_size, bytes_read);

        if (body_stream.is_eof() || bytes_read == 0) {
            Log::InfoF(LOG_FILE_MESSAGE("bytes_read = 0 (EOF), pulling completed"));
            self->has_reached_eof_ = true;

            return pplx::create_task([] {});
        } else {
            // TODO: Write to blocking_buffer_
            auto& vec = container_buf.collection();
            blocking_buffer_.WriteChunk(std::move(vec));

            return Pump(body_stream, chunk_size);
        }
    });

    return task;
}

void StreamLoader::Abort() {
    cancel_token_source_.cancel();
    blocking_buffer_.NotifyExit();
}

size_t StreamLoader::Read(uint8_t* buffer, size_t expected_bytes) {
    // Log::InfoF("Read(): before: expected_bytes = %zu, readable = %zu", expected_bytes, blocking_buffer_.ReadableBytes());

    size_t bytes_read = blocking_buffer_.Read(buffer, expected_bytes);

    // Log::InfoF("Read(): after: bytes_read = %zu, readable = %zu", bytes_read, blocking_buffer_.ReadableBytes());

    return bytes_read;
}

size_t StreamLoader::RemainReadable() {
    return blocking_buffer_.ReadableBytes();
}
