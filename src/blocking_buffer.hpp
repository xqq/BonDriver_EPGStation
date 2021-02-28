//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_BLOCKING_BUFFER_HPP
#define BONDRIVER_EPGSTATION_BLOCKING_BUFFER_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <vector>
#include <mutex>
#include <condition_variable>
#include "noncopyable.hpp"

class BlockingBuffer {
public:
    explicit BlockingBuffer(size_t chunk_size);
    BlockingBuffer(size_t chunk_size, size_t max_chunk_count);
    BlockingBuffer(size_t chunk_size, size_t max_chunk_count, size_t min_chunk_count);
    ~BlockingBuffer();
    size_t Read(uint8_t* buffer, size_t expected_bytes);
    size_t Write(const uint8_t* buffer, size_t bytes);
    size_t WriteChunk(const std::vector<uint8_t>& vec);
    size_t WriteChunk(std::vector<uint8_t>&& vec);
    void WaitUntilData();
    void WaitUntilEmpty();
    void NotifyExit();
    bool IsExit();
    size_t ReadableBytes();
    void Clear();
private:
    class Chunk {
    public:
        explicit Chunk(size_t chunk_size);
        explicit Chunk(std::vector<uint8_t>&& vec);
        Chunk(Chunk&&) = default;
        Chunk& operator=(Chunk&&) = default;
        size_t Read(uint8_t* buffer, size_t expected_bytes);
        size_t Write(const uint8_t* buffer, size_t bytes);
        size_t RemainReadable() const;
        size_t RemainWritable() const;
    private:
        size_t chunk_size_;
        ptrdiff_t read_pos_;
        ptrdiff_t write_pos_;
        std::vector<uint8_t> vec_;
    private:
        DISALLOW_COPY_AND_ASSIGN(Chunk);
    };
private:
    size_t chunk_size_;
    bool has_chunk_count_limit_;
    size_t max_chunk_count_;
    size_t min_chunk_count_;
    bool is_exit_;
    std::deque<Chunk> deque_;
    std::mutex mutex_;
    std::condition_variable consume_cv_;
    std::condition_variable produce_cv_;
private:
    DISALLOW_COPY_AND_ASSIGN(BlockingBuffer);
};


#endif // BONDRIVER_EPGSTATION_BLOCKING_BUFFER_HPP
