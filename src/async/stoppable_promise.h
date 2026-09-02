#ifndef OTTER_ASYNC_STOPPABLE_PROMISE_H
#define OTTER_ASYNC_STOPPABLE_PROMISE_H

#include <coroutine>
#include <optional>
#include <stop_token>
#include <type_traits>
#include <utility>

#include "io_context.h"
#include "this_coro.h"

namespace otter::async {

template<typename T>
concept cancellable = requires(T&& t, IOContext& context) { t.cancel(context); };

template<typename Awaitable>
concept io_awaiter =
    requires(Awaitable awaitable, std::coroutine_handle<> handle, IOContext& context) {
        { awaitable.await_ready() } -> std::convertible_to<bool>;

        {
            awaitable.await_suspend(handle, context)
        } -> std::convertible_to<std::coroutine_handle<>>;

        { awaitable.await_resume() };
    };

struct StoppablePromise {
    std::stop_token stop_token;
    IOContext* context;

    StoppablePromise() = default;

    StoppablePromise(IOContext& ctx)
      : context{ &ctx }
    {}

    StoppablePromise(IOContext& ctx, std::stop_token stop_token)
      : context{ &ctx }
      , stop_token{ std::move(stop_token) }
    {}

    template<typename Awaitable>
    class StopTokenWrapper {
    private:
        struct CancelFunc {
            StopTokenWrapper<Awaitable>* wrapper;

            void operator()()
            {
                if (wrapper) {
                    if constexpr (cancellable<Awaitable>)
                        wrapper->inner_.cancel(*(wrapper->promise_->context));
                    else
                        wrapper->context().cancel(wrapper->inner_);
                }
            }
        };

        Awaitable inner_;
        StoppablePromise* promise_;
        std::optional<std::stop_callback<CancelFunc>> stop_callback_;

    public:
        StopTokenWrapper(Awaitable&& inner, StoppablePromise* promise)
          : inner_{ std::forward<Awaitable>(inner) }
          , promise_{ promise }
        {}

        ~StopTokenWrapper()
        {
            stop_callback_.reset();
        }

        auto context() const noexcept -> IOContext&
        {
            return *(promise_->context);
        }

        auto await_ready() -> bool
        {
            return inner_.await_ready();
        }

        template<typename Promise>
        auto await_suspend(std::coroutine_handle<Promise> handle) noexcept
            -> std::coroutine_handle<>
        {
            auto inner_handle = inner_.await_suspend(handle, context());

            if (promise_->stop_token.stop_possible())
                stop_callback_.emplace(promise_->stop_token, CancelFunc{ this });

            return inner_handle;
        }

        auto await_resume()
        {
            return inner_.await_resume();
        }
    };

    template<typename Awaitable>
    auto await_transform(Awaitable&& awaitable) -> decltype(auto)
    {
        using Tag = std::remove_cvref_t<Awaitable>;

        if constexpr (std::is_same_v<Tag, this_coro::context_tag>) {
            struct Awaiter {
                IOContext* context;

                auto await_ready() const noexcept -> bool
                {
                    return true;
                }

                void await_suspend(std::coroutine_handle<> handle) noexcept {}

                auto await_resume() const noexcept -> IOContext&
                {
                    return *context;
                }
            };

            return Awaiter{ context };
        }
        else if constexpr (std::is_same_v<Tag, this_coro::stop_token_tag>) {
            struct Awaiter {
                std::stop_token stop_token;

                auto await_ready() const noexcept -> bool
                {
                    return true;
                }

                void await_suspend(std::coroutine_handle<> handle) noexcept {}

                auto await_resume() const noexcept -> std::stop_token
                {
                    return stop_token;
                }
            };

            return Awaiter{ stop_token };
        }
        else if constexpr (io_awaiter<Tag>) {
            return StopTokenWrapper<Awaitable>{ std::forward<Awaitable>(awaitable), this };
        }
        else {
            return std::forward<Awaitable>(awaitable);
        }
    }
};

} // namespace otter::async

#endif // OTTER_ASYNC_STOPPABLE_PROMISE_H