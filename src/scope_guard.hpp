//
// @author magicxqq <xqq@xqq.im>
//

#ifndef BONDRIVER_EPGSTATION_SCOPE_GUARD_HPP
#define BONDRIVER_EPGSTATION_SCOPE_GUARD_HPP

#include <functional>
#include "noncopyable.hpp"

class ScopeGuard {
private:
    using Deleter = std::function<void()>;
public:
    template <typename F>
    explicit ScopeGuard(F&& fn) noexcept : callback_(std::forward<F>(fn)), dismissed_(false) {}

    ScopeGuard(ScopeGuard&& other) noexcept : callback_(std::move(other.callback_)), dismissed_(other.dismissed_) {}

    ~ScopeGuard() {
        if (!dismissed_) {
            callback_();
        }
    }

    void Dismiss() noexcept {
        dismissed_ = true;
    }
private:
    Deleter callback_;
    bool dismissed_;
private:
    DISALLOW_COPY_AND_ASSIGN(ScopeGuard);
};

struct ScopeGuardDriver {};

template <typename F>
auto operator+(ScopeGuardDriver, F&& fn) {
    return ScopeGuard(std::forward<F>(fn));
}

#define CONCATENATE_IMPL(part1, part2) part1##part2
#define CONCATENATE(part1, part2) CONCATENATE_IMPL(part1, part2)
#define ANONYMOUS_VAR(tag) CONCATENATE(tag, __LINE__)

#define ON_SCOPE_EXIT \
    auto ANONYMOUS_VAR(_exit_) = ScopeGuardDriver() + [&]() noexcept

#define MAKE_SCOPE_GUARD \
    ScopeGuardDriver() + [&]() noexcept

#endif // BONDRIVER_EPGSTATION_SCOPE_GUARD_HPP
