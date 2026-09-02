#ifndef OTTER_ASYNC_SPAWN_H
#define OTTER_ASYNC_SPAWN_H

#include <concepts>
#include <stop_token>
#include <type_traits>
#include <utility>

#include "detached_task.h"

namespace otter::async {

template<typename Awaitable>
auto detach(IOContext& ctx, Awaitable awaitable) -> DetachedTask
{
    co_await awaitable;
}

template<typename Awaitable>
auto detach(IOContext& ctx, std::stop_token stop_token, Awaitable awaitable) -> DetachedTask
{
    co_await awaitable;
}

template<typename Awaitable>
    requires std::movable<std::remove_cvref_t<Awaitable>>
void spawn(IOContext& ctx, Awaitable awaitable)
{
    detach(ctx, std::move(awaitable));
}

template<typename Awaitable>
    requires std::movable<std::remove_cvref_t<Awaitable>>
void spawn(IOContext& ctx, Awaitable awaitable, std::stop_token stop_token)
{
    detach(ctx, stop_token, std::move(awaitable));
}

} // namespace otter::async

#endif // OTTER_ASYNC_SPAWN_H