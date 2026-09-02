#ifndef OTTER_UTILITY_DEFER_H
#define OTTER_UTILITY_DEFER_H

#include <tuple>
#include <type_traits>
#include <utility>

namespace otter {

template<typename F, typename... Args>
struct DeferredTaskFactory {
    F func;
    std::tuple<Args...> args;

    // 核心能力：每次调用 operator()，都会产生一个全新的 Task 实例！
    auto operator()() const
    {
        return std::apply(func, args);
    }
};

template<typename F, typename... Args>
auto defer(F&& f, Args&&... args)
{
    return DeferredTaskFactory<std::decay_t<F>, std::decay_t<Args>...>{
        std::forward<F>(f), std::make_tuple(std::forward<Args>(args)...)
    };
}

} // namespace otter

#endif // OTTER_UTILITY_DEFER_H