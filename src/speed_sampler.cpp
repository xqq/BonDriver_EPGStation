//
// @author magicxqq <xqq@xqq.im>
//

#ifdef _WIN32
    #include <Windows.h>
#endif
#include "speed_sampler.hpp"

SpeedSampler::SpeedSampler() {
    QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency_));
}

void SpeedSampler::Reset() {
    first_checkpoint_ = 0;
    last_checkpoint_ = 0;
    total_bytes_ = 0;
    interval_bytes_ = 0;
    last_second_bytes_ = 0;
}

void SpeedSampler::AddBytes(size_t bytes) {
    if (first_checkpoint_ == 0) {
        // first time
        first_checkpoint_ = GetCurrentClock();
        last_checkpoint_ = first_checkpoint_;

        interval_bytes_ += bytes;
        total_bytes_ += bytes;
    } else if (GetCurrentClock() - last_checkpoint_ < 1000) {
        // 0 < duration < 1000
        interval_bytes_ += bytes;
        total_bytes_ += bytes;
    } else {
        // duration >= 1000
        last_second_bytes_ = interval_bytes_;

        interval_bytes_ = bytes;
        total_bytes_ += bytes;

        last_checkpoint_ = GetCurrentClock();
    }
}

float SpeedSampler::CurrentKBps() {
    AddBytes(0);

    double elapsed_seconds = (GetCurrentClock() - last_checkpoint_) / 1000.0f;
    if (elapsed_seconds == 0.0f) {
        elapsed_seconds = 1.0f;
    }

    double value = (interval_bytes_ / elapsed_seconds) / 1024.0f;

    return static_cast<float>(value);
}

float SpeedSampler::LastSecondKBps() {
    AddBytes(0);

    if (last_second_bytes_ > 0) {
        return last_second_bytes_ / 1024.0f;
    }

    // last_second_bytes_ == 0
    if (GetCurrentClock() - last_checkpoint_ >= 500) {
        // if time interval since last checkpoint has exceeded 500ms
        // the speed is nearly accurate
        return CurrentKBps();
    }

    // We don't know
    return 0.0f;
}

float SpeedSampler::AverageKBps() {
    double elapsed_seconds = (GetCurrentClock() - first_checkpoint_) / 1000.0f;
    return (total_bytes_ / elapsed_seconds) / 1024.0f;
}

time_t SpeedSampler::GetCurrentClock() {
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return current.QuadPart * 1000 / frequency_.QuadPart;
}
