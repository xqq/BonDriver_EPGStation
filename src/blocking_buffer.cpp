//
// @author magicxqq <xqq@xqq.im>
//

#include <cassert>
#include "blocking_buffer.hpp"


BlockingBuffer::BlockingBuffer(size_t chunk_size)
    : chunk_size_(chunk_size), has_chunk_count_limit_(false),
    max_chunk_count_(SIZE_MAX), min_chunk_count_(0), is_exit_(false) {}

BlockingBuffer::BlockingBuffer(size_t chunk_size, size_t max_chunk_count)
    : chunk_size_(chunk_size), has_chunk_count_limit_(true),
    max_chunk_count_(max_chunk_count), min_chunk_count_(0), is_exit_(false) {}

BlockingBuffer::BlockingBuffer(size_t chunk_size, size_t max_chunk_count, size_t min_chunk_count)
    : chunk_size_(chunk_size), has_chunk_count_limit_(true),
    max_chunk_count_(max_chunk_count), min_chunk_count_(min_chunk_count), is_exit_(false) {}

BlockingBuffer::~BlockingBuffer() {
    if (!is_exit_) {
        NotifyExit();
    }
}

size_t BlockingBuffer::Read(uint8_t* buffer, size_t expected_bytes) {
    // I am the data consumer
    assert(buffer != nullptr);
    assert(expected_bytes > 0);
    assert(!is_exit_);

    std::unique_lock locker(mutex_);

    if (has_chunk_count_limit_ && deque_.size() < min_chunk_count_) {
        // consumer standby, waiting notify message from the producer
        consume_cv_.wait(locker, [this] {
            return deque_.size() >= min_chunk_count_ || is_exit_;
        });
    }

    uint8_t* out = buffer;
    size_t bytes_read = 0;
    ptrdiff_t remain_unread = static_cast<ptrdiff_t>(expected_bytes);

    while (remain_unread > 0) {
        if (deque_.empty() && !is_exit_) {
            // Wait for producing
            consume_cv_.wait(locker, [this] {
                return !deque_.empty() || is_exit_;
            });

            if (is_exit_ && deque_.empty()) {
                break;
            }
        }

        auto& front_chunk = deque_.front();

        size_t request_bytes = std::min(static_cast<size_t>(remain_unread), front_chunk.RemainReadable());
        size_t chunk_read = front_chunk.Read(out, request_bytes);

        bytes_read += chunk_read;
        remain_unread -= chunk_read;
        out += chunk_read;

        if (front_chunk.RemainReadable() == 0) {
            deque_.pop_front();
        }
    }

    // Notify the data producer to produce data
    produce_cv_.notify_one();
    return bytes_read;
}

size_t BlockingBuffer::Write(const uint8_t *buffer, size_t bytes) {
    // I am the data producer
    assert(buffer != nullptr);
    assert(bytes > 0);
    assert(!is_exit_);

    std::unique_lock locker(mutex_);

    if (has_chunk_count_limit_ && deque_.size() >= max_chunk_count_) {
        consume_cv_.notify_one();
        // producer standby, waiting notify message from the consumer
        produce_cv_.wait(locker, [this] {
            return deque_.size() < max_chunk_count_ || is_exit_;
        });
    }

    const uint8_t* in = buffer;
    size_t bytes_written = 0;
    ptrdiff_t remain_unwrite = static_cast<ptrdiff_t>(bytes);

    while (remain_unwrite > 0) {
        if (deque_.size() >= max_chunk_count_) {
            consume_cv_.notify_one();
            // Wait for consuming
            produce_cv_.wait(locker, [this] {
                return deque_.size() < max_chunk_count_ || is_exit_;
            });
        }

        if (deque_.empty() || deque_.back().RemainWritable() == 0) {
            // No existing chunk, or back chunk is full
            deque_.emplace_back(chunk_size_);
        }

        auto& back_chunk = deque_.back();

        size_t attempt_bytes = std::min(static_cast<size_t>(remain_unwrite), back_chunk.RemainWritable());
        size_t chunk_written = back_chunk.Write(in, attempt_bytes);

        bytes_written += chunk_written;
        remain_unwrite -= chunk_written;
        in += chunk_written;
    }

    // Notify the consumer to consume data
    consume_cv_.notify_one();
    return bytes_written;
}

size_t BlockingBuffer::WriteChunk(const std::vector<uint8_t>& vec) {
    // I am the data producer
    assert(vec.size() == chunk_size_);

    std::unique_lock locker(mutex_);

    if (has_chunk_count_limit_ && deque_.size() >= max_chunk_count_) {
        consume_cv_.notify_one();
        // producer standby, waiting notify message from the consumer
        produce_cv_.wait(locker, [this] {
            return deque_.size() < max_chunk_count_ || is_exit_;
        });
    }

    size_t bytes = vec.size();
    std::vector<uint8_t> vec_clone = vec;
    deque_.emplace_back(std::move(vec_clone));

    // Notify the consumer to consume data
    consume_cv_.notify_one();
    return bytes;
}

size_t BlockingBuffer::WriteChunk(std::vector<uint8_t>&& vec) {
    // I am the data producer
    assert(vec.size() == chunk_size_);

    std::unique_lock locker(mutex_);

    if (has_chunk_count_limit_ && deque_.size() >= max_chunk_count_) {
        consume_cv_.notify_one();
        // producer standby, waiting notify message from the consumer
        produce_cv_.wait(locker, [this] {
            return deque_.size() < max_chunk_count_ || is_exit_;
        });
    }

    size_t bytes = vec.size();
    deque_.emplace_back(std::move(vec));

    // Notify the consumer to consume data
    consume_cv_.notify_one();
    return bytes;
}

void BlockingBuffer::WaitUntilData() {
    std::unique_lock locker(mutex_);

    if (has_chunk_count_limit_) {
        consume_cv_.wait(locker, [this] {
            return deque_.size() >= min_chunk_count_ || is_exit_;
        });
    } else {
        consume_cv_.wait(locker, [this] {
            return !deque_.empty() || is_exit_;
        });
    }
}

void BlockingBuffer::WaitUntilEmpty() {
    std::unique_lock locker(mutex_);

    produce_cv_.wait(locker, [this] {
        return deque_.empty() || is_exit_;
    });
}

void BlockingBuffer::NotifyExit() {
    std::lock_guard guard(mutex_);

    is_exit_ = true;
    consume_cv_.notify_all();
    produce_cv_.notify_all();
}

bool BlockingBuffer::IsExit() {
    std::lock_guard guard(mutex_);
    return is_exit_;
}

size_t BlockingBuffer::ReadableBytes() {
    std::lock_guard guard(mutex_);

    size_t readable = 0;

    for (const Chunk& chunk : deque_) {
        readable += chunk.RemainReadable();
    }

    return readable;
}

void BlockingBuffer::Clear() {
    std::lock_guard guard(mutex_);

    deque_.clear();
}


BlockingBuffer::Chunk::Chunk(size_t chunk_size)
    : chunk_size_(chunk_size), read_pos_(0), write_pos_(0), vec_(chunk_size) {}

BlockingBuffer::Chunk::Chunk(std::vector<uint8_t>&& vec)
    : chunk_size_(vec.size()), read_pos_(0), write_pos_(vec.size()), vec_(std::move(vec)) {}

size_t BlockingBuffer::Chunk::Read(uint8_t* buffer, size_t expected_bytes) {
    assert(buffer != nullptr);
    assert(expected_bytes > 0);
    assert(read_pos_ <= write_pos_);

    size_t remain = RemainReadable();

    if (remain < expected_bytes) {
        return 0;
    }

    uint8_t* read_ptr = vec_.data() + read_pos_;
    memcpy(buffer, read_ptr, expected_bytes);
    read_pos_ += expected_bytes;

    return expected_bytes;
}

size_t BlockingBuffer::Chunk::Write(const uint8_t *buffer, size_t bytes) {
    assert(buffer != nullptr);
    assert(bytes > 0);
    assert(read_pos_ <= write_pos_);

    size_t remain = RemainWritable();

    if (remain < bytes) {
        return 0;
    }

    uint8_t* write_ptr = vec_.data() + write_pos_;
    memcpy(write_ptr, buffer, bytes);
    write_pos_ += bytes;

    return bytes;
}

size_t BlockingBuffer::Chunk::RemainReadable() const {
    return write_pos_ - read_pos_;
}

size_t BlockingBuffer::Chunk::RemainWritable() const {
    return static_cast<ptrdiff_t>(chunk_size_) - write_pos_;
}
