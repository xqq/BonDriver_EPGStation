//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_SPEED_SAMPLER_HPP
#define BONDRIVER_EPGSTATION_SPEED_SAMPLER_HPP

#include <ctime>
#include <cstddef>
#include <cstdint>

class SpeedSampler {
private:
    typedef union LargeInteger {
        struct {
            uint32_t LowPart;
            int32_t HighPart;
        };
        int64_t QuadPart;
    } LargeInteger;
public:
    SpeedSampler();
    void Reset();
    void AddBytes(size_t bytes);
    float CurrentKBps();
    float LastSecondKBps();
    float AverageKBps();
private:
    time_t GetCurrentClock();
private:
    time_t first_checkpoint_ = 0;
    time_t last_checkpoint_ = 0;

    size_t interval_bytes_ = 0;
    size_t total_bytes_ = 0;
    size_t last_second_bytes_ = 0;

    LargeInteger frequency_;
};


#endif // BONDRIVER_EPGSTATION_SPEED_SAMPLER_HPP
