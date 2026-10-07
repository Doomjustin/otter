#ifndef OTTER_UTILITY_OVERLOADS_H
#define OTTER_UTILITY_OVERLOADS_H

#include <utility>

namespace otter {

/// @brief 为 std::visit 方便地定义多个 lambda 重载。
///
/// overloads 使用继承和包展开将多个 lambda 函数聚合成一个访问者对象。
/// 与 std::visit 结合使用时，可以以更清晰的方式处理 std::variant。
///
/// 用法示例：
/// @code
///   std::variant<int, float, std::string> v{ 42 };
///
///   std::visit(overloads{
///       [](int i) { std::cout << "int: " << i; },
///       [](float f) { std::cout << "float: " << f; },
///       [](const std::string& s) { std::cout << "string: " << s; }
///   }, v);
/// @endcode
template<typename... T>
struct overloads : T... {
    using T::operator()...;

    template<typename... Args>
    constexpr decltype(auto)
    dance(Args&&... args) & noexcept(noexcept((*this)(std::forward<Args>(args)...)))
    {
        return (*this)(std::forward<Args>(args)...);
    }

    template<typename... Args>
    constexpr decltype(auto)
    dance(Args&&... args) const& noexcept(noexcept((*this)(std::forward<Args>(args)...)))
    {
        return (*this)(std::forward<Args>(args)...);
    }
};

/// @brief 以 `dance` 作为便捷入口，让访问者对象可以直接被调用。
template<typename... T, typename... Args>
constexpr decltype(auto)
dance(overloads<T...>& visitor,
      Args&&... args) noexcept(noexcept(visitor(std::forward<Args>(args)...)))
{
    return visitor(std::forward<Args>(args)...);
}

template<typename... T, typename... Args>
constexpr decltype(auto)
dance(const overloads<T...>& visitor,
      Args&&... args) noexcept(noexcept(visitor(std::forward<Args>(args)...)))
{
    return visitor(std::forward<Args>(args)...);
}

/// @brief 推导指南：支持类模板参数推导 (CTAD)，无需显式指定模板参数。
///
/// 使用示例：
/// @code
///   // 无需 overloads<...>
///   auto visitor = overloads{
///       [](int) { return 1; },
///       [](double) { return 2; }
///   };
/// @endcode
template<typename... T>
overloads(T...) -> overloads<T...>;

} // namespace otter

#endif // OTTER_UTILITY_OVERLOADS_H
