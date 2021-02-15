//
// @author magicxqq <xqq@xqq.im>
//

#include <vector>
#include <cpprest/http_client.h>
#include <cpprest/producerconsumerstream.h>
#include "log.hpp"
#include "scope_guard.hpp"
#include "utils.hpp"
#include "stream_loader.hpp"

using namespace web::http;
using namespace web::http::client;

StreamLoader::StreamLoader(size_t block_size, size_t buffer_size) :
        buffer_(block_size), buffer_size_(buffer_size), block_size_(block_size),
        has_response_received_(false), has_reached_eof_(false), request_failed_(false) {
    ostream_ = buffer_.create_ostream();
    istream_ = buffer_.create_istream();
    Log::InfoF("StreamLoader(): producer_consumer_buffer: buffer_size() = %zu, size() = %zu", buffer_.buffer_size(), buffer_.size());

}

StreamLoader::~StreamLoader() {
    ostream_.close();
    istream_.close();
    buffer_.close();
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
            std::lock_guard<std::mutex> lock(response_mutex_);
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
                Log::InfoF("Open(): producer_consumer_buffer: buffer_size() = %zu, size() = %zu", buffer_.buffer_size(), buffer_.size());
                return Pump(body_stream, block_size_);
            } else {
                Log::ErrorF(LOG_FILE_MESSAGE("Invalid body_stream"));
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
    std::unique_lock<std::mutex> lock(response_mutex_);
    if (has_response_received_ && !request_failed_) {
        return true;
    }

    response_cv_.wait(lock, [this] {
        return has_response_received_;
    });

    // If [has_response_received_] and [not request_failed_], that's succeed
    return !request_failed_;
}

pplx::task<void> StreamLoader::Pump(pplx::streams::istream body_stream, size_t chunk_size) {
    Log::InfoF("Pump(): chunk_size = %zu", chunk_size);
    if (cancel_token_source_.get_token().is_canceled()) {
        return pplx::create_task([] {});
    }

    StreamLoader* self = this;

    pplx::task<void> task = body_stream.read(ostream_.streambuf(), chunk_size)
    .then([=](const pplx::task<size_t>& previous_task) {
        if (self->cancel_token_source_.get_token().is_canceled()) {
            return pplx::create_task([] {});
        }

        size_t bytes_read = 0;
        try {
            bytes_read = previous_task.get();
        } catch (const std::exception& ex) {
            Log::ErrorF(LOG_FILE_MESSAGE(ex.what()));
        }

        Log::InfoF("Pump(): bytes_read = %zu, buffer_size() = %zu, size() = %zu", bytes_read, buffer_.buffer_size(), buffer_.size());

        if (bytes_read > 0) {
            return Pump(body_stream, chunk_size);
        } else {
            // bytes_read == 0, EOF
            Log::InfoF(LOG_FILE_MESSAGE("bytes_read = 0 (EOF), pulling completed"));
            self->has_reached_eof_ = true;
            return self->ostream_.close();
        }
    });

    return task;
}

void StreamLoader::Abort() {
    cancel_token_source_.cancel();
}

size_t StreamLoader::Read(uint8_t* buffer, size_t expected_bytes) {
    Log::InfoF("Read(): before: expected_bytes = %zu, buffer_size() = %zu, size() = %zu", expected_bytes, buffer_.buffer_size(), buffer_.size());
    concurrency::streams::container_buffer<std::vector<uint8_t>> container_buf;

    size_t bytes_read = 0;
    try {
        bytes_read = istream_.read(container_buf, expected_bytes).get();
    } catch (const std::exception& ex) {
        Log::InfoF("StreamLoader::Read(): istream async read exception: %s", ex.what());
    }

    Log::InfoF("Read(): after: bytes_read = %zu, buffer_size() = %zu, size() = %zu", buffer_.buffer_size(), buffer_.size());
    memcpy(buffer, container_buf.collection().data(), bytes_read);

    return bytes_read;
}
