//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_STREAM_LOADER_HPP
#define BONDRIVER_EPGSTATION_STREAM_LOADER_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <optional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cpprest/http_client.h>
#include <cpprest/producerconsumerstream.h>
#include "config.hpp"

class StreamLoader {
public:
    StreamLoader(size_t block_size, size_t buffer_size);
    ~StreamLoader();
    bool Open(const std::string& base_url, const std::string& path_query, std::optional<BasicAuth> basic_auth = std::nullopt);
    void Abort();
    bool WaitForResponse();
    size_t Read(uint8_t* buffer, size_t expected_bytes);
private:
    pplx::task<void> Pump(pplx::streams::istream body_stream, size_t chunk_size);
private:
    pplx::streams::producer_consumer_buffer<uint8_t> buffer_;
    pplx::streams::istream istream_;
    pplx::streams::ostream ostream_;
    size_t buffer_size_;
    size_t block_size_;
    std::mutex buffer_mutex_;

    std::mutex response_mutex_;
    std::condition_variable response_cv_;
    bool has_response_received_;
    bool has_reached_eof_;
    bool request_failed_;

    pplx::cancellation_token_source cancel_token_source_;
};


#endif // BONDRIVER_EPGSTATION_STREAM_LOADER_HPP
