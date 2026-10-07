#ifndef OTTER_UTILITY_NAMED_TYPE_H
#define OTTER_UTILITY_NAMED_TYPE_H

#include <cmath>
#include <concepts>
#include <cstddef>
#include <type_traits>

#include "meta/string.h"

namespace otter {

/// @brief 强类型包装器，通过 Skills 模板参数实现能力定制。
///
/// NamedType 提供类型安全的值语义包装。通过组合不同的 Skill，
/// 可灵活定制类型的运算能力。
///
/// 示例：
/// @code
///   using Meter = NamedType<double, "Meter", Arithmetic, Comparable>;
///   auto distance = Meter{ 3.14 };
///   auto doubled = distance * Meter{ 2.0 };
/// @endcode
template<typename T, meta::String Name, template<typename> class... Skills>
    requires std::is_arithmetic_v<T>
class NamedType : public Skills<NamedType<T, Name, Skills...>>... {
public:
    using ValueType = T;

    explicit constexpr NamedType(const T& v) noexcept
      : value_{ v }
    {}

    explicit constexpr NamedType(T&& v) noexcept
      : value_{ std::move(v) }
    {}

    [[nodiscard]]
    constexpr T& get() noexcept
    {
        return value_;
    }

    [[nodiscard]]
    constexpr const T& get() const noexcept
    {
        return value_;
    }

    [[nodiscard]]
    T& operator*() noexcept
    {
        return value_;
    }

    [[nodiscard]]
    const T& operator*() const noexcept
    {
        return value_;
    }

private:
    T value_;
};

template<typename Derived>
struct dividable {
    constexpr Derived& operator/=(const Derived& other) noexcept
    {
        static_cast<Derived*>(this)->get() /= other.get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator/(Derived lhs, const Derived& rhs) noexcept
    {
        lhs /= rhs;
        return lhs;
    }
};

template<typename Derived>
struct decrementable {
    constexpr Derived& operator--() noexcept
    {
        --static_cast<Derived*>(this)->get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    constexpr Derived operator--(int) noexcept
    {
        Derived temp = *static_cast<Derived*>(this);
        --static_cast<Derived*>(this)->get();
        return temp;
    }
};

template<typename Derived>
struct incrementable {
    constexpr Derived& operator++() noexcept
    {
        ++static_cast<Derived*>(this)->get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    constexpr Derived operator++(int) noexcept
    {
        Derived temp = *static_cast<Derived*>(this);
        ++static_cast<Derived*>(this)->get();
        return temp;
    }
};

template<typename Derived>
struct addable {
    constexpr Derived& operator+=(const Derived& other) noexcept
    {
        static_cast<Derived*>(this)->get() += other.get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator+(Derived lhs, const Derived& rhs) noexcept
    {
        lhs += rhs;
        return lhs;
    }
};

template<typename Derived>
struct subtractable {
    constexpr Derived& operator-=(const Derived& other) noexcept
    {
        static_cast<Derived*>(this)->get() -= other.get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator-(Derived lhs, const Derived& rhs) noexcept
    {
        lhs -= rhs;
        return lhs;
    }
};

template<typename Derived>
struct multipliable {
    constexpr Derived& operator*=(const Derived& other) noexcept
    {
        static_cast<Derived*>(this)->get() *= other.get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator*(Derived lhs, const Derived& rhs) noexcept
    {
        lhs *= rhs;
        return lhs;
    }
};

template<typename Derived>
struct remainder_assignable {
    constexpr Derived& operator%=(const Derived& other) noexcept
        requires std::integral<typename Derived::ValueType>
    {
        static_cast<Derived*>(this)->get() %= other.get();
        return *static_cast<Derived*>(this);
    }

    constexpr Derived& operator%=(const Derived& other) noexcept
        requires std::floating_point<typename Derived::ValueType>
    {
        static_cast<Derived*>(this)->get() =
            std::fmod(static_cast<Derived*>(this)->get(), other.get());
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator%(Derived lhs, const Derived& rhs) noexcept
        requires std::integral<typename Derived::ValueType> ||
                 std::floating_point<typename Derived::ValueType>
    {
        lhs %= rhs;
        return lhs;
    }
};

template<typename Derived>
struct arithmetic
  : decrementable<Derived>
  , incrementable<Derived>
  , addable<Derived>
  , subtractable<Derived>
  , multipliable<Derived>
  , dividable<Derived>
  , remainder_assignable<Derived> {};

template<typename Derived>
    requires std::integral<typename Derived::ValueType>
struct bitwise_and_assignable {
    constexpr Derived& operator&=(const Derived& other) noexcept
    {
        static_cast<Derived*>(this)->get() &= other.get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator&(Derived lhs, const Derived& rhs) noexcept
        requires std::integral<typename Derived::ValueType>
    {
        lhs &= rhs;
        return lhs;
    }
};

template<typename Derived>
    requires std::integral<typename Derived::ValueType>
struct bitwise_or_assignable {
    constexpr Derived& operator|=(const Derived& other) noexcept
    {
        static_cast<Derived*>(this)->get() |= other.get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator|(Derived lhs, const Derived& rhs) noexcept
        requires std::integral<typename Derived::ValueType>
    {
        lhs |= rhs;
        return lhs;
    }
};

template<typename Derived>
    requires std::integral<typename Derived::ValueType>
struct bitwise_xor_assignable {
    constexpr Derived& operator^=(const Derived& other) noexcept
    {
        static_cast<Derived*>(this)->get() ^= other.get();
        return *static_cast<Derived*>(this);
    }

    [[nodiscard]]
    friend constexpr Derived operator^(Derived lhs, const Derived& rhs) noexcept
        requires std::integral<typename Derived::ValueType>
    {
        lhs ^= rhs;
        return lhs;
    }
};

template<typename Derived>
struct bitwise
  : bitwise_and_assignable<Derived>
  , bitwise_or_assignable<Derived>
  , bitwise_xor_assignable<Derived> {};

template<typename Derived>
struct comparable {
    [[nodiscard]]
    friend constexpr auto operator<=>(const Derived& lhs, const Derived& rhs)
    {
        return lhs.get() <=> rhs.get();
    }

    [[nodiscard]]
    friend constexpr auto operator==(const Derived& lhs, const Derived& rhs) -> bool
    {
        return lhs.get() == rhs.get();
    }
};

template<typename Derived>
struct hashable {
    [[nodiscard]]
    std::size_t hash() const noexcept
    {
        using HashType = typename Derived::ValueType;
        return std::hash<HashType>{}(static_cast<const Derived*>(this)->get());
    }
};

template<typename Derived>
struct printable {
    friend auto operator<<(std::ostream& os, const Derived& obj) -> std::ostream&
    {
        return os << obj.get();
    }
};

template<template<typename> class Skill>
struct skill_t;

template<template<typename> class Skill, template<typename> class... Remaining>
concept contains_skill = (std::is_same_v<skill_t<Skill>, skill_t<Remaining>> || ...);

template<template<typename> class... Skills>
consteval bool is_unique_skills_impl()
{
    if constexpr (sizeof...(Skills) <= 1)
        return true;
    else
        return []<template<typename> class First, template<typename> class... Rest>() consteval {
            return (!(contains_skill<First, Rest>) && ...) && is_unique_skills_impl<Rest...>();
        }.template operator()<Skills...>();
}

template<template<typename> class... Skills>
concept unique_skills = is_unique_skills_impl<Skills...>();

template<typename T, meta::String Name, template<typename> class... Skills>
    requires unique_skills<arithmetic, comparable, hashable, printable, Skills...>
using Numeric = NamedType<T, Name, arithmetic, comparable, hashable, printable, Skills...>;

} // namespace otter

/// @brief std::hash 特化，支持 NamedType 用于关联容器
template<typename T, otter::meta::String Name, template<typename> class... Skills>
struct std::hash<otter::NamedType<T, Name, Skills...>> {
    std::size_t operator()(const otter::NamedType<T, Name, Skills...>& name_type) const noexcept
    {
        return name_type.hash();
    }
};

#endif // OTTER_UTILITY_NAMED_TYPE_H
