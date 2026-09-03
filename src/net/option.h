#ifndef OTTER_NET_OPTION_H
#define OTTER_NET_OPTION_H

#include <compare>
#include <concepts>

#include <sys/socket.h>

namespace otter::net {

template<typename T>
concept socket_option = requires(const T& opt) {
    { T::level } -> std::convertible_to<int>;
    { T::name } -> std::convertible_to<int>;
    { opt.data() } -> std::convertible_to<const void*>;
    { opt.size() } -> std::convertible_to<std::size_t>;
} && std::is_default_constructible_v<T>;

template<typename T>
concept flag_option = requires(const T& opt) {
    { T::get_cmd } -> std::convertible_to<int>;
    { T::set_cmd } -> std::convertible_to<int>;
    { T::bit } -> std::convertible_to<int>;
} && std::constructible_from<T, bool> && std::convertible_to<T, bool>;

template<int Level, int Name>
class BooleanOption {
public:
    static constexpr int level = Level;
    static constexpr int name = Name;
    using value_type = bool;

    explicit BooleanOption(bool value = false)
      : value_{ value ? 1 : 0 }
    {}

    [[nodiscard]]
    constexpr auto value() const noexcept -> bool
    {
        return value_ != 0;
    }

    [[nodiscard]]
    auto data() const noexcept -> const void*
    {
        return &value_;
    }

    auto data() noexcept -> void*
    {
        return &value_;
    }

    [[nodiscard]]
    constexpr auto size() const noexcept -> std::size_t
    {
        return sizeof(value_);
    }

    auto operator<=>(const BooleanOption& other) const noexcept = default;

    [[nodiscard]]
    constexpr operator bool() const noexcept
    {
        return value_ != 0;
    }

private:
    int value_{};
};

/// 适用于以数值形式配置的 socket option。
///
/// @tparam Level socket option level。
/// @tparam Name socket option name。
/// @tparam T option 存储类型，需满足 integral。
template<int Level, int Name, std::integral T = int>
class ValueOption {
public:
    static constexpr int level = Level;
    static constexpr int name = Name;
    using value_type = T;

    explicit ValueOption(T value = {})
      : value_{ value }
    {}

    [[nodiscard]] constexpr auto value() const noexcept -> T
    {
        return value_;
    }

    [[nodiscard]]
    auto data() const noexcept -> const void*
    {
        return &value_;
    }

    auto data() noexcept -> void*
    {
        return &value_;
    }

    [[nodiscard]]
    constexpr auto size() const noexcept -> std::size_t
    {
        return sizeof(value_);
    }

    auto operator<=>(const ValueOption& other) const noexcept = default;

    [[nodiscard]]
    constexpr operator bool() const noexcept
    {
        return value_ != 0;
    }

private:
    T value_{};
};

/// @brief 基于位标志的 fcntl option 封装。
///
/// 常用于通过 fcntl 读写文件描述符标志位。
///
/// @tparam GetCmd fcntl 获取命令（如 F_GETFL）。
/// @tparam SetCmd fcntl 设置命令（如 F_SETFL）。
/// @tparam Bit 目标标志位掩码（如 O_NONBLOCK）。
template<int GetCmd, int SetCmd, int Bit>
class FlagOption {
public:
    static constexpr int get_cmd = GetCmd;
    static constexpr int set_cmd = SetCmd;
    static constexpr int bit = Bit;

    explicit FlagOption(bool enabled = false)
      : value_{ enabled ? bit : 0 }
    {}

    [[nodiscard]]
    constexpr auto value() const noexcept -> bool
    {
        return (value_ & bit) != 0;
    }

    [[nodiscard]]
    auto data() const noexcept -> const void*
    {
        return &value_;
    }

    auto data() noexcept -> void*
    {
        return &value_;
    }

    [[nodiscard]]
    constexpr auto size() const noexcept -> std::size_t
    {
        return sizeof(value_);
    }

    auto operator<=>(const FlagOption& other) const noexcept = default;

    [[nodiscard]]
    constexpr operator bool() const noexcept
    {
        return (value_ & bit) != 0;
    }

private:
    int value_{};
};

/// @brief socket `SO_LINGER` 选项的轻量封装。
class LingerOption {
private:
    struct ::linger value_;

public:
    static constexpr int level = SOL_SOCKET;
    static constexpr int name = SO_LINGER;
    using value_type = struct ::linger;

    LingerOption() = default;

    explicit LingerOption(bool on, int timeout)
      : value_{ .l_onoff = on ? 1 : 0, .l_linger = timeout }
    {}

    [[nodiscard]]
    constexpr auto value() const noexcept -> const value_type&
    {
        return value_;
    }

    auto data() noexcept -> void*
    {
        return &value_;
    }

    [[nodiscard]]
    auto data() const noexcept -> const void*
    {
        return &value_;
    }

    [[nodiscard]]
    constexpr auto size() const noexcept -> std::size_t
    {
        return sizeof(value_);
    }

    auto operator==(const LingerOption& other) const noexcept -> bool
    {
        return value_.l_onoff == other.value_.l_onoff && value_.l_linger == other.value_.l_linger;
    }

    auto operator<=>(const LingerOption& other) const noexcept -> std::strong_ordering
    {
        return value_.l_onoff == other.value_.l_onoff ? value_.l_linger <=> other.value_.l_linger
                                                      : value_.l_onoff <=> other.value_.l_onoff;
    }
};

} // namespace otter::net

#endif // OTTER_NET_OPTION_H