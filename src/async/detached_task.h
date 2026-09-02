#ifndef OTTER_ASYNC_DETACHED_TASK_H
#define OTTER_ASYNC_DETACHED_TASK_H

#include <coroutine>
#include <exception>

#include "stoppable_promise.h"

namespace otter::async {

struct DetachedTask {
    struct promise_type : StoppablePromise {
        template<typename Awaitable>
        promise_type(IOContext& ctx, Awaitable&& awaitable)
          : StoppablePromise{ ctx }
        {}

        template<typename Awaitable>
        promise_type(IOContext& ctx, std::stop_token stop_token, Awaitable&& awaitable)
          : StoppablePromise{ ctx, std::move(stop_token) }
        {}

        ~promise_type() noexcept
        {
            context->untrack();
        }

        auto get_return_object() noexcept -> DetachedTask
        {
            return {};
        }

        auto initial_suspend() noexcept
        {
            struct Awaiter {
                IOContext& context;

                auto await_ready() const noexcept -> bool
                {
                    return false;
                }

                void await_suspend(std::coroutine_handle<> h) const noexcept
                {
                    context.submit(h);
                }

                void await_resume() const noexcept {}
            };

            return Awaiter{ *context };
        }

        auto final_suspend() noexcept
        {
            return std::suspend_never{};
        }

        void return_void() noexcept {}

        void unhandled_exception() noexcept
        {
            // 既然是 fire-and-forget，异常无法传播给调用方，因此直接终止程序。
            // 也可以选择记录日志或其他处理方式，这里为了简化直接终止。
            std::terminate();
        }
    };
};

} // namespace otter::async

#endif // OTTER_ASYNC_DETACHED_TASK_H