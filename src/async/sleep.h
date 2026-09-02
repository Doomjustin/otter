#ifndef OTTER_ASYNC_SLEEP_H
#define OTTER_ASYNC_SLEEP_H

#include <chrono>
#include <type_traits>

#include "awaiter.h"

namespace otter::async {

template<typename T>
struct is_chrono_duration_impl : std::false_type {};

template<typename Rep, typename Period>
struct is_chrono_duration_impl<std::chrono::duration<Rep, Period>> : std::true_type {};

template<typename T>
concept chrono_duration = is_chrono_duration_impl<std::remove_cvref_t<T>>::value;

class TimerAwaiter : public IOAwaiter<TimerAwaiter> {
private:
    ::__kernel_timespec timeout_;

    auto cast_time(chrono_duration auto duration) -> __kernel_timespec
    {
        using namespace std::chrono;
        auto ns = duration_cast<nanoseconds>(duration).count();
        return { ns / 1'000'000'000, ns % 1'000'000'000 };
    }

public:
    template<chrono_duration Duration>
    explicit TimerAwaiter(Duration duration)
      : IOAwaiter<TimerAwaiter>{}
    {
        if (duration < Duration::zero())
            timeout_ = { .tv_sec = 0, .tv_nsec = 0 };
        else
            timeout_ = cast_time(duration);
    }

    template<typename Clock, typename Duration>
    TimerAwaiter(std::chrono::time_point<Clock, Duration> timepoint)
      : IOAwaiter<TimerAwaiter>{}
    {
        auto now = Clock::now();

        if (now >= timepoint)
            timeout_ = { .tv_sec = 0, .tv_nsec = 0 };
        else
            timeout_ = cast_time(timepoint - now);
    }

    auto await_ready() const noexcept -> bool
    {
        return timeout_.tv_sec == 0 && timeout_.tv_nsec == 0;
    }

    auto await_resume() noexcept -> std::expected<void, std::error_code>
    {
        if (result == 0 || result == -ETIME)
            return {};

        return utility::unexpected_system_error(-result);
    }

    void prepare(::io_uring_sqe* sqe) noexcept
    {
        ::io_uring_prep_timeout(sqe, &timeout_, 0, 0);
    }
};

template<chrono_duration Duration>
auto sleep_for(Duration duration) -> TimerAwaiter
{
    return TimerAwaiter{ duration };
}

template<typename Clock, typename Duration>
auto sleep_until(std::chrono::time_point<Clock, Duration> timepoint) -> TimerAwaiter
{
    return TimerAwaiter{ timepoint };
}

} // namespace otter::async

#endif // OTTER_ASYNC_SLEEP_H