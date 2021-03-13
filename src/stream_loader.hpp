//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_STREAM_LOADER_HPP
#define BONDRIVER_EPGSTATION_STREAM_LOADER_HPP

#include <cstdint>
#include <cstddef>
#include <string>
#include <utility>
#include <optional>
#include <future>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <cpr/response.h>
#include <cpr/session.h>
#include "blocking_buffer.hpp"
#include "config.hpp"
#include "speed_sampler.hpp"

class StreamLoader {
public:
    StreamLoader(size_t chunk_size, size_t max_chunk_count, size_t min_chunk_count);
    ~StreamLoader();
    bool Open(const std::string& base_url,
              const std::string& path_query,
              std::optional<BasicAuth> basic_auth = std::nullopt,
              std::optional<std::string> user_agent = std::nullopt,
              std::optional<std::string> proxy = std::nullopt);
    void Abort();
    bool WaitForResponse();
    bool WaitForData();
    size_t Read(uint8_t* buffer, size_t expected_bytes);
    std::pair<uint8_t*, size_t> ReadChunkAndRetain();
    size_t RemainReadable();
    bool IsPolling();
    float GetCurrentSpeedKByte();
private:
    bool OnHeaderCallback(std::string data);
    bool OnWriteCallback(std::string data);
private:
    size_t chunk_size_;
    BlockingBuffer blocking_buffer_;

    bool has_requested_ = false;
    bool has_response_received_ = false;
    bool has_reached_eof_ = false;
    bool request_failed_ = false;
    std::atomic<bool> has_requested_abort_ = false;

    SpeedSampler speed_sampler_;

    cpr::Session session_;
    std::future<cpr::Response> async_response_;

    std::mutex response_mutex_;
    std::condition_variable response_cv_;
};


#endif // BONDRIVER_EPGSTATION_STREAM_LOADER_HPP
