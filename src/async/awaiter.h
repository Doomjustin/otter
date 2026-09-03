#ifndef OTTER_ASYNC_AWAITER_H
#define OTTER_ASYNC_AWAITER_H

#include <coroutine>
#include <cstdint>
#include <expected>
#include <type_traits>

#include <otter/utility.h>

#include "io_context.h"

namespace otter::async {

struct DummyAwaiter : Operation {
    void resume(int result, std::uint32_t flags) override {}
};

template<typename Derived, typename ResumeType = void>
struct IOAwaiter : Operation {
public:
    auto await_ready() const noexcept -> bool
    {
        return false;
    }

    auto await_suspend(std::coroutine_handle<> handle, IOContext& ctx) noexcept
        -> std::coroutine_handle<>
    {
        this->handle = handle;

        if (auto* sqe = ctx.sqe()) {
            static_cast<Derived*>(this)->prepare(sqe);
            ::io_uring_sqe_set_data(sqe, this);
            ctx.track();
            return std::noop_coroutine();
        }

        result = -EAGAIN;
        return handle;
    }

    auto await_resume() noexcept -> std::expected<ResumeType, std::error_code>
    {
        if (result < 0)
            return unexpected_system_error(-result);

        if constexpr (std::is_void_v<ResumeType>)
            return {};
        else
            return static_cast<Derived*>(this)->value();
    }

    auto value() const noexcept -> ResumeType
    {
        return static_cast<ResumeType>(result);
    }
};

} // namespace otter::async

#endif // OTTER_ASYNC_AWAITER_H