#ifndef OTTER_ASYNC_TIMEOUT_AWAITER_H
#define OTTER_ASYNC_TIMEOUT_AWAITER_H

#include <coroutine>
#include <cstdint>

#include <liburing.h>

#include "io_context.h"
#include "operation.h"

namespace otter::async {

template<typename Awaitable>
class TimeoutAwaiter : public Operation {
private:
    struct Timer : public Operation {
        TimeoutAwaiter<Awaitable>* owner;

        void resume(int result, std::uint32_t flags) override
        {
            if (result == -ETIME)
                owner->is_timeout_ = true;

            if (--owner->pending_cqes_ == 0)
                std::exchange(owner->handle, nullptr).resume();
        }
    };

    Awaitable inner_;
    ::__kernel_timespec timeout_;

    Timer dummy_;
    int pending_cqes_{ 2 };
    bool is_timeout_{ false };

    void release(::io_uring_sqe* sqe) noexcept
    {
        if (sqe) {
            ::io_uring_prep_nop(sqe);
            ::io_uring_sqe_set_data(sqe, nullptr);
        }
    }

public:
    template<utility::chrono_duration Duration>
    TimeoutAwaiter(Awaitable&& awaitable, Duration timeout)
      : inner_{ std::move(awaitable) }
    {
        using namespace std::chrono;

        auto ns = duration_cast<nanoseconds>(timeout).count();
        timeout_.tv_sec = ns / 1'000'000'000;
        timeout_.tv_nsec = ns % 1'000'000'000;
    }

    constexpr auto await_ready() const noexcept -> bool
    {
        return false;
    }

    auto await_suspend(std::coroutine_handle<> handle, IOContext& context) noexcept
        -> std::coroutine_handle<>
    {
        this->handle = handle;
        dummy_.owner = this;

        auto* inner_sqe = context.sqe();
        auto* timeout_sqe = context.sqe();

        while (!inner_sqe || !timeout_sqe) {
            release(inner_sqe);
            release(timeout_sqe);

            this->result = -EAGAIN;
            return handle;
        }

        inner_.prepare(inner_sqe);
        inner_sqe->flags |= IOSQE_IO_LINK;
        inner_.parent = this;
        ::io_uring_prep_link_timeout(timeout_sqe, &timeout_, 0);

        context.track();
        context.track();

        return std::noop_coroutine();
    }

    auto await_resume() noexcept -> decltype(auto)
    {
        using Result = decltype(inner_.await_resume());

        if (is_timeout_)
            return Result{ std::unexpect, std::make_error_code(std::errc::timed_out) };

        return inner_.await_resume();
    }

    void resume(int result, std::uint32_t flags) override
    {
        this->result = result;
        this->flags = flags;

        if (--pending_cqes_ == 0)
            std::exchange(handle, nullptr).resume();
    }

    void cancel(IOContext& context) noexcept
    {
        // timeout_remove 比默认的 ASYNC_CANCEL 更适合，
        // 不过默认的 ASYNC_CANCEL 也能正常工作（因为timeout_remove 失败时会退化为普通取消）。
        // 所以这里其实可以不要，这里想用cancel测试一下切面是否生效。
        if (auto* sqe = context.sqe()) {
            ::io_uring_prep_timeout_remove(sqe, this, 0);
            ::io_uring_sqe_set_data(sqe, this);
        }
    }
};

template<typename Awaitable, chrono_duration Duration>
class TimeoutWrapper {
private:
    /// @brief stop_source_ 回调：将 stop 请求转发到 `inner_.cancel(context_)`。
    struct CancelFn {
        TimeoutWrapper* owner;

        void operator()() noexcept
        {
            owner->inner_.cancel(owner->context_);
        }
    };

    std::stop_source stop_source_;
    IOContext* context_{ nullptr };
    bool is_timeout_{ false };
    bool is_canceled_{ false };

    Awaitable inner_;
    Duration timeout_;

    /// @details `std::stop_callback` 不可移动，因此以 `unique_ptr` 持有。
    std::unique_ptr<std::stop_callback<CancelFn>> stop_callback_;

    /// @brief 独立 timer 任务：等待 timeout 后触发 stop 请求。
    /// @param[in] combinator 当前组合器对象。
    /// @return 可由 `co_spawn` 调度的 `Task<>`。
    static auto timer(TimeoutWrapper* combinator) -> Task<>
    {
        co_await sleep_for(combinator->timeout_);

        if (combinator->is_canceled_)
            co_return;

        combinator->is_timeout_ = true;
        combinator->stop_source_.request_stop();
    }

public:
    /// @brief 构造 TimeoutWrapper。
    /// @param[in] awaitable 被包装的 inner awaiter。
    /// @param[in] timeout 超时时长。
    TimeoutWrapper(Awaitable&& awaitable, Duration timeout)
      : inner_{ std::forward<Awaitable>(awaitable) }
      , timeout_{ timeout }
    {}

    constexpr auto await_ready() const noexcept -> bool
    {
        return inner_.await_ready();
    }

    /// @brief 注册 stop 回调并启动 timer 任务。
    /// @param[in] handle 当前 coroutine continuation。
    /// @param[in,out] context 执行取消与调度的 IOContext。
    /// @return inner 对应的 `await_suspend` 返回值。
    auto await_suspend(std::coroutine_handle<> handle, IOContext& context) noexcept
        -> std::coroutine_handle<>
    {
        this->context_ = &context;

        stop_callback_ = std::make_unique<std::stop_callback<CancelFn>>(stop_source_.get_token(),
                                                                        CancelFn{ this });
        co_spawn(context, stop_source_.get_token(), timer(this));

        if constexpr (requires { inner_.await_suspend(handle, context); })
            return inner_.await_suspend(handle, context);
        else
            return inner_.await_suspend(handle);
    }

    /// @brief 按 win-first 语义返回最终结果。
    /// @return timeout 时返回 `unexpected(timed_out)`；cancel 时返回
    /// `unexpected(operation_canceled)`；否则透传 `inner_.await_resume()`。
    auto await_resume() noexcept -> decltype(auto)
    {
        using Result = decltype(inner_.await_resume());

        if (is_timeout_)
            return Result{ std::unexpect, std::make_error_code(std::errc::timed_out) };

        if (is_canceled_)
            return Result{ std::unexpect, std::make_error_code(std::errc::operation_canceled) };

        return inner_.await_resume();
    }

    /// @brief 响应外部 cancel 请求并触发 stop 流程。
    /// @param[in,out] context 为统一签名保留。
    /// @return 无。
    void cancel(IOContext& context) noexcept
    {
        is_canceled_ = true;
        stop_source_.request_stop();
    }
};

template<typename Awaitable, utility::chrono_duration Duration>
auto timeout(Awaitable&& awaitable, Duration timeout) -> decltype(auto)
{
    if constexpr (std::derived_from<Awaitable, Operation>)
        return TimeoutAwaiter<Awaitable>{ std::forward<Awaitable>(awaitable), timeout };
    else
        return TimeoutWrapper<Awaitable, Duration>{ std::forward<Awaitable>(awaitable), timeout };
}

} // namespace otter::async

#endif // OTTER_ASYNC_TIMEOUT_AWAITER_H