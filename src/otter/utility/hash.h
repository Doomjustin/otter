#ifndef OTTER_UTILITY_HASH_H
#define OTTER_UTILITY_HASH_H

#include <cstddef>
#include <functional>
#include <string_view>

namespace otter {

struct use_std_has_t {
    std::string_view value;
};

struct use_fnv1_hash_t {
    std::string_view value;
};

constexpr auto use_std_hash(const std::string_view value) noexcept
{
    return use_std_has_t{ value };
}

constexpr auto use_fnv_1a(const std::string_view value) noexcept
{
    return use_fnv1_hash_t{ value };
}

[[nodiscard]]
inline std::size_t hash(use_std_has_t value) noexcept
{
    using Hasher = std::hash<std::string_view>;
    return Hasher{}(value.value);
}

[[nodiscard]]
constexpr std::size_t hash(use_fnv1_hash_t value) noexcept
{
    constexpr auto fnv_offset_basis = 14695981039346656037uz;
    constexpr auto fnv_prime = 1099511628211uz;

    auto result = fnv_offset_basis;
    for (const unsigned char byte : value.value) {
        result ^= byte;
        result *= fnv_prime;
    }

    return result;
}

/// @brief std::string 的哈希函数对象，使用 FNV-1a 算法。
/// @details
/// 提供 is_transparent 类型定义，允许在 std::unordered_map 中
/// 直接使用 std::string_view 作为查询键，无需构造临时 std::string 对象。
///
/// 用法示例：
/// @code
///   std::unordered_map<std::string, int, otter::string_hasher, std::equal_to<>> map;
///   map["key"] = 42;
///
///   // 可直接使用 std::string_view 查询，无需转换
///   if (map.find("key") != map.end())
///       std::cout << map.at("key");
/// @endcode
struct string_hasher {
    using is_transparent = void;

    std::size_t operator()(const std::string_view sv) const noexcept
    {
        return static_cast<std::size_t>(hash(use_fnv_1a(sv)));
    }
};

/// @brief std::pmr::string 的哈希函数对象，使用 FNV-1a 算法。
/// @details
/// 与 string_hasher 类似，提供 is_transparent 类型定义，
/// 允许在 std::pmr::unordered_map 中直接使用 std::string_view 作为查询键。
struct pmr_string_hasher {
    using is_transparent = void;

    std::size_t operator()(const std::string_view sv) const noexcept
    {
        return static_cast<std::size_t>(hash(use_fnv_1a(sv)));
    }
};

} // namespace otter

#endif // OTTER_UTILITY_HASH_H
