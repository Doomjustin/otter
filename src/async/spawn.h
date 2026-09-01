#ifndef OTTER_ASYNC_SPAWN_H
#define OTTER_ASYNC_SPAWN_H

#include <coroutine>
#include <stop_token>

#include "detached_task.h"

namespace otter::async {

template<typename Awaitable>
auto detach(IOContext& ctx, Awaitable awaitable) -> DetachedTask
{
    co_await std::move(awaitable);
}

template<typename Awaitable>
auto detach(IOContext& ctx, std::stop_token stop_token, Awaitable awaitable) -> DetachedTask
{
    co_await std::move(awaitable);
}

template<typename Awaitable>
void spawn(IOContext& ctx, Awaitable awaitable)
{
    // auto detached_task = detach(ctx, std::move(awaitable));
    // std::coroutine_handle<> handle;
    // if constexpr (requires { awaitable.handle(); })
    //     handle = awaitable.handle();
    // else if constexpr (requires { awaitable.handle; })
    //     handle = awaitable.handle;

    // if constexpr (requires { awaitable.release(); })
    auto handle = awaitable.release();
    ctx.submit(handle);
}

// template<typename Awaitable>
// void spawn(IOContext& ctx, Awaitable awaitable, std::stop_token stop_token)
// {
//     // auto detached_task = detach(ctx, stop_token, std::move(awaitable));
//     if constexpr (requires { awaitable.release(); })
//         awaitable.release();

//     ctx.submit(awaitable.handle());
// }

} // namespace otter::async

#endif // OTTER_ASYNC_SPAWN_H