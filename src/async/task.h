#ifndef OTTER_ASYNC_TASK_H
#define OTTER_ASYNC_TASK_H

#include <coroutine>
#include <optional>
#include <utility>

#include "stoppable_promise.h"

namespace otter::async {

template<typename T>
class TaskReturnType {
protected:
    std::optional<T> result_;

public:
    template<typename U>
        requires std::convertible_to<U&&, T>
    void return_value(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>)
    {
        result_.emplace(std::forward<U>(value));
    }

    void return_value(std::in_place_t tag) noexcept(std::is_nothrow_default_constructible_v<T>)
    {
        result_.emplace();
    }

    /// @brief 取出已保存的返回值。
    /// @return 以右值形式返回保存的结果。
    auto result() noexcept -> T&&
    {
        return std::move(*result_);
    }
};

/// @brief Task<void> 的返回值策略特化。
template<>
class TaskReturnType<void> {
public:
    /// @brief 处理 void 返回。
    void return_void() noexcept {}
};

template<typename T = void>
class Task {
public:
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

private:
    handle_type handle_;

public:
    struct promise_type
      : TaskReturnType<T>
      , StoppablePromise {

        union {
            std::exception_ptr exception;
        };

        bool has_exception{ false };
        std::coroutine_handle<> next;

        promise_type() {}

        ~promise_type()
        {
            if (has_exception)
                exception.~exception_ptr();
        }

        auto get_return_object() noexcept -> Task
        {
            return Task{ handle_type::from_promise(*this) };
        }

        auto initial_suspend() noexcept -> std::suspend_always
        {
            return {};
        }

        auto final_suspend() noexcept
        {
            struct Awaiter {
                promise_type* promise;

                constexpr auto await_ready() const noexcept -> bool
                {
                    return false;
                }

                auto await_suspend(std::coroutine_handle<> handle) noexcept
                {
                    return promise->next ? promise->next : std::noop_coroutine();
                }

                void await_resume() noexcept {}
            };

            return Awaiter{ this };
        }

        void unhandled_exception() noexcept
        {
            new (&exception) std::exception_ptr(std::current_exception());
            has_exception = true;
        }
    };

    Task() = default;

    Task(handle_type handle)
      : handle_{ handle }
    {}

    Task(const Task&) = delete;
    auto operator=(const Task&) -> Task& = delete;

    Task(Task&& other) noexcept
      : handle_{ std::exchange(other.handle_, {}) }
    {}

    auto operator=(Task&& other) noexcept -> Task&
    {
        if (this == &other)
            return *this;

        if (handle_)
            handle_.destroy();

        handle_ = std::exchange(other.handle_, {});
        return *this;
    }

    ~Task()
    {
        if (handle_)
            handle_.destroy();
    }

    constexpr auto await_ready() const noexcept -> bool
    {
        return false;
    }

    template<typename Promise>
    auto await_suspend(std::coroutine_handle<Promise> parent) -> std::coroutine_handle<>
    {
        handle_.promise().next = parent;

        if constexpr (requires { parent.promise().context; })
            handle_.promise().context = parent.promise().context;

        if constexpr (requires { parent.promise().stop_token; })
            handle_.promise().stop_token = parent.promise().stop_token;

        return handle_;
    }

    auto await_resume() const
    {
        if (handle_.promise().has_exception)
            std::rethrow_exception(handle_.promise().exception);

        if constexpr (!std::is_void_v<T>)
            return handle_.promise().result();
        else
            return;
    }

    auto handle() const noexcept
    {
        return handle_;
    }

    auto release() noexcept
    {
        return std::exchange(handle_, nullptr);
    }

    auto resume()
    {
        handle_.resume();
    }

    auto operator()()
    {
        resume();
    }
};

} // namespace otter::async

#endif // OTTER_ASYNC_TASK_H