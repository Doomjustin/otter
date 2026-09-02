#ifndef OTTER_ASYNC_RETRY_H
#define OTTER_ASYNC_RETRY_H

#include <chrono>

#include <otter/utility.h>

#include "sleep.h"
#include "task.h"

namespace otter::async {

// WARNING: 这个方案未完成，问题太多，和awaitable无法组合

struct RetryAdapter {
    size_t max_retries;
    std::chrono::milliseconds backoff; // 可选：退避等待时间
};

inline auto retry(size_t times, std::chrono::milliseconds backoff = std::chrono::milliseconds(0))
{
    return RetryAdapter{ .max_retries = times, .backoff = backoff };
}

template<typename T>
struct is_expected : std::false_type {};

template<typename T, typename E>
struct is_expected<std::expected<T, E>> : std::true_type {};

template<typename T>
concept IsExpected = is_expected<T>::value;

template<typename Factory>
    requires std::invocable<Factory>
auto operator|(Factory&& factory, RetryAdapter adapter)
    -> Task<typename std::invoke_result_t<Factory>::value_type>
{
    using ReturnType = typename std::invoke_result_t<Factory>::value_type;
    using namespace std::chrono_literals;

    size_t attempts = 0;
    while (true) {
        if constexpr (IsExpected<ReturnType>) {
            auto res = co_await factory();

            if (res.has_value() || ++attempts >= adapter.max_retries)
                co_return res;
            if (adapter.backoff > 0ms)
                co_await sleep_for(adapter.backoff);
        }
        else {
            bool has_error = false;

            try {
                co_return co_await factory();
            }
            catch (...) {
                if (++attempts >= adapter.max_retries)
                    throw;
                has_error = true;
            }

            if (has_error && adapter.backoff > 0ms)
                co_await sleep_for(adapter.backoff);
        }
    }
}

} // namespace otter::async

#endif // OTTER_ASYNC_RETRY_H