#ifndef OTTER_ASYNC_THIS_CORO_H
#define OTTER_ASYNC_THIS_CORO_H

namespace otter::async::this_coro {

struct context_tag {};

struct stop_token_tag {};

constexpr context_tag context{};

constexpr stop_token_tag stop_token{};

} // namespace otter::async::this_coro

#endif // OTTER_ASYNC_THIS_CORO_H