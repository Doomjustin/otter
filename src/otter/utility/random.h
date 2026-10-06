#ifndef OTTER_UTILITY_RANDOM_H
#define OTTER_UTILITY_RANDOM_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <random>
#include <ranges>

namespace otter {

struct random {
    random() = delete;

    /// @brief 设置全局随机数种子，用于使随机序列可复现。
    static void seed(std::uint32_t value)
    {
        engine().seed(value);
    }

    /// @brief 生成伯努利随机值（真/假）。
    static auto bernoulli(double percentage = 0.5) -> bool
    {
        assert(percentage >= 0.0 && percentage <= 1.0);

        std::bernoulli_distribution dist{ percentage };
        return dist(engine());
    }

    /// @brief 生成区间 [low, high) 内的均匀随机整数。
    template<std::integral T = int>
    static auto uniform(T low, T high) -> T
    {
        assert(high > low);

        std::uniform_int_distribution<T> dist{ low, high - 1 };
        return dist(engine());
    }

    /// @brief 生成区间 [0, high) 内的均匀随机整数。
    template<std::integral T = int>
    static auto uniform(T high) -> T
    {
        assert(high > T{});

        std::uniform_int_distribution<T> dist{ T{}, high - 1 };
        return dist(engine());
    }

    /// @brief 生成区间 [low, high) 内的均匀随机浮点数。
    template<std::floating_point T = double>
    static auto uniform(T low, T high) -> T
    {
        assert(high > low);

        std::uniform_real_distribution<T> dist{ low, high };
        return dist(engine());
    }

    /// @brief 生成区间 [0, high) 内的均匀随机浮点数。
    template<std::floating_point T = double>
    static auto uniform(T high) -> T
    {
        assert(high > T{});

        std::uniform_real_distribution<T> dist{ T{}, high };
        return dist(engine());
    }

    /// @brief 生成几何分布随机值（首次成功前的失败次数）。
    template<std::integral T = int>
    static auto geometric_failure(double percentage) -> T
    {
        assert(percentage >= 0.0 && percentage <= 1.0);

        std::geometric_distribution<T> dist(percentage);
        return dist(engine());
    }

    /// @brief 生成二项分布随机值（n 次试验中成功次数）。
    template<std::integral T = int>
    static auto binomial(T n, double percentage) -> T
    {
        assert(percentage >= 0.0 && percentage <= 1.0);

        std::binomial_distribution<T> dist{ n, percentage };
        return dist(engine());
    }

    /// @brief 生成正态分布随机值。
    template<std::floating_point T = double>
    static auto normal(T mean = 0.0, T stddev = 1.0) -> T
    {
        assert(stddev > T{});

        std::normal_distribution<T> dist{ mean, stddev };
        return dist(engine());
    }

    /// @brief 生成指数分布随机值（平均等待时间为 1/lambda）。
    template<std::floating_point T = double>
    static auto exponential(T lambda = 1.0) -> T
    {
        assert(lambda > T{});

        std::exponential_distribution<T> dist{ lambda };
        return dist(engine());
    }

    /// @brief 原地随机打乱容器中的元素。
    template<std::ranges::random_access_range Range>
    static void shuffle(Range&& range)
    {
        std::ranges::shuffle(range, engine());
    }

    /// @brief 从容器中随机选择一个元素。
    template<std::ranges::forward_range Range>
    static auto choice(Range&& range) -> std::ranges::range_reference_t<Range>
    {
        assert(!std::ranges::empty(range));

        auto index = uniform(std::ranges::size(range));
        auto it = std::ranges::begin(range);
        std::advance(it, index);
        return *it;
    }

    /// @brief 从容器中随机无放回抽样指定数量的元素。
    template<std::ranges::forward_range Range>
    static auto sample(Range&& range, std::size_t count)
        -> std::vector<std::ranges::range_value_t<Range>>
    {
        assert(count <= std::ranges::size(range));

        using ValueType = std::ranges::range_value_t<Range>;
        std::vector<ValueType> result;
        result.reserve(count);

        std::ranges::sample(range, std::back_inserter(result), count, engine());
        return result;
    }

private:
    static auto engine() -> std::mt19937&
    {
        thread_local std::mt19937 engine{ std::random_device{}() };
        return engine;
    }
};

} // namespace otter

#endif // OTTER_UTILITY_RANDOM_H
