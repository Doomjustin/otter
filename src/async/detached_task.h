#ifndef OTTER_ASYNC_DETACHED_TASK_H
#define OTTER_ASYNC_DETACHED_TASK_H

#include <coroutine>
#include <exception>

#include "stoppable_promise.h"

namespace otter::async {

struct DetachedTask {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    handle_type handle;

    struct promise_type : StoppablePromise {
        template<typename... Args>
        promise_type(IOContext& ctx, Args&&... args)
          : StoppablePromise{ ctx }
        {}

        template<typename... Args>
        promise_type(IOContext& ctx, std::stop_token stop_token, Args&&... args)
          : StoppablePromise{ ctx, std::move(stop_token) }
        {}

        auto get_return_object() noexcept -> DetachedTask
        {
            return DetachedTask{ handle_type::from_promise(*this) };
        }

        auto initial_suspend() noexcept -> std::suspend_always
        {
            return {};
        }

        auto final_suspend() noexcept -> std::suspend_never
        {
            return {};
        }

        void return_void() noexcept {}

        void unhandled_exception() noexcept
        {
            // 既然是 fire-and-forget，异常无法传播给调用方，因此直接终止程序。
            // 也可以选择记录日志或其他处理方式，这里为了简化直接终止。
            std::terminate();
        }
    };

    explicit DetachedTask(handle_type h) noexcept
      : handle{ h }
    {}
};

} // namespace otter::async

#endif // OTTER_ASYNC_DETACHED_TASK_H