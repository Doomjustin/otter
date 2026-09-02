#ifndef OTTER_ASYNC_TIMEOUT_AWAITER_H
#define OTTER_ASYNC_TIMEOUT_AWAITER_H

#include <coroutine>
#include <stop_token>
#include <type_traits>
#include <utility>

#include <liburing.h>

#include "io_context.h"
#include "spawn.h"
#include "task.h"

namespace otter::async {

template<typename Awaitable, chrono_duration Duration>
class TimeoutWrapper {
private:
    struct CancelFn {
        TimeoutWrapper* owner;

        void operator()() noexcept
        {
            if constexpr (cancellable<Awaitable>)
                owner->inner_.cancel(owner->context_);
            else
                owner->context_->cancel(owner->inner_);
        }
    };

    std::stop_source stop_source_;
    IOContext* context_{ nullptr };
    bool is_timeout_{ false };
    bool is_canceled_{ false };

    Awaitable inner_;
    Duration timeout_;

    std::unique_ptr<std::stop_callback<CancelFn>> stop_callback_;

public:
    using value_type = decltype(inner_.await_resume());

    TimeoutWrapper(Awaitable&& awaitable, Duration timeout)
      : inner_{ std::forward<Awaitable>(awaitable) }
      , timeout_{ timeout }
    {}

    constexpr auto await_ready() const noexcept -> bool
    {
        return inner_.await_ready();
    }

    auto await_suspend(std::coroutine_handle<> handle, IOContext& context) noexcept
        -> std::coroutine_handle<>
    {
        this->context_ = &context;

        stop_callback_ = std::make_unique<std::stop_callback<CancelFn>>(stop_source_.get_token(),
                                                                        CancelFn{ this });
        spawn(context, timer(this), stop_source_.get_token());

        if constexpr (requires { inner_.await_suspend(handle, context); })
            return inner_.await_suspend(handle, context);
        else
            return inner_.await_suspend(handle);
    }

    auto await_resume() noexcept -> decltype(auto)
    {
        // using Result = decltype(inner_.await_resume());

        if (is_timeout_)
            return value_type{ std::unexpect, std::make_error_code(std::errc::timed_out) };

        if (is_canceled_)
            return value_type{ std::unexpect, std::make_error_code(std::errc::operation_canceled) };

        return inner_.await_resume();
    }

    void cancel(IOContext& context) noexcept
    {
        is_canceled_ = true;
        stop_source_.request_stop();
    }

private:
    static auto timer(TimeoutWrapper* combinator) -> Task<>
    {
        co_await sleep_for(combinator->timeout_);

        if (combinator->is_canceled_)
            co_return;

        combinator->is_timeout_ = true;
        combinator->stop_source_.request_stop();
    }
};

template<typename Awaitable, chrono_duration Duration>
auto timeout(Awaitable&& awaitable, Duration timeout) -> decltype(auto)
{
    return TimeoutWrapper<Awaitable, Duration>{ std::forward<Awaitable>(awaitable), timeout };
}

template<chrono_duration Duration>
struct TimeoutAdaptor {
    Duration duration;
};

template<chrono_duration Duration>
auto timeout(Duration duration) -> TimeoutAdaptor<std::decay_t<Duration>>
{
    return TimeoutAdaptor<std::decay_t<Duration>>{ std::move(duration) };
}

template<typename Awaitable, chrono_duration Duration>
auto operator|(Awaitable&& awaitable, TimeoutAdaptor<Duration> adaptor) -> decltype(auto)
{
    return timeout(std::forward<Awaitable>(awaitable), adaptor.duration);
}

template<typename Factory, chrono_duration Duration>
    requires std::invocable<Factory>
auto operator|(Factory&& factory, TimeoutAdaptor<Duration> adaptor)
{
    return [factory = std::forward<Factory>(factory), adaptor]() mutable {
        return timeout(factory(), adaptor.duration);
    };
}

} // namespace otter::async

#endif // OTTER_ASYNC_TIMEOUT_AWAITER_H