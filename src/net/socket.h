#ifndef OTTER_NET_SOCKET_H
#define OTTER_NET_SOCKET_H

#include <cassert>

#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <otter/utility.h>

#include "option.h"
#include "query_endpoint.h"

namespace otter::net {

template<typename T>
concept socket_protocol = requires(const T& t) {
    typename T::endpoint;

    { t.domain() } -> std::convertible_to<int>;
    { t.type() } -> std::convertible_to<int>;
    { t.protocol() } -> std::convertible_to<int>;
};

/// @brief 面向协议类型的基础 socket 封装。
///
/// 提供 socket 生命周期管理（RAII）、bind、option 读写与 native handle访问能力。
//  对象可 move，不可 copy。
///
/// @tparam Protocol 协议类型，需满足 socket_protocol。
template<socket_protocol Protocol>
class BasicSocket : public QueryLocalEndpoint<BasicSocket<Protocol>> {
private:
    static constexpr int INVALID_SOCKET = -1;

    int fd_ = INVALID_SOCKET;

public:
    using protocol_type = Protocol;
    using endpoint_type = typename Protocol::endpoint;

    using error = BooleanOption<SOL_SOCKET, SO_ERROR>;
    using receive_buffer_size = ValueOption<SOL_SOCKET, SO_RCVBUF>;
    using send_buffer_size = ValueOption<SOL_SOCKET, SO_SNDBUF>;
    using linger = LingerOption;
    using non_blocking = FlagOption<F_GETFL, F_SETFL, O_NONBLOCK>;
    using close_on_exec = FlagOption<F_GETFD, F_SETFD, FD_CLOEXEC>;

    BasicSocket() = default;

    explicit BasicSocket(const Protocol& protocol)
      : fd_{ create(protocol) }
    {}

    BasicSocket(const BasicSocket&) = delete;
    auto operator=(const BasicSocket&) -> BasicSocket& = delete;

    BasicSocket(BasicSocket&& other) noexcept
      : fd_{ std::exchange(other.fd_, INVALID_SOCKET) }
    {}

    auto operator=(BasicSocket&& other) noexcept -> BasicSocket&
    {
        if (this == &other)
            return *this;

        close();

        fd_ = std::exchange(other.fd_, INVALID_SOCKET);
        return *this;
    }

    virtual ~BasicSocket()
    {
        close();
    }

    [[nodiscard]]
    constexpr auto is_valid() const noexcept -> bool
    {
        return fd_ != INVALID_SOCKET;
    }

    void open(const Protocol& protocol)
    {
        if (is_valid())
            throw std::runtime_error{ "Socket is already open" };

        fd_ = create(protocol);
    }

    void bind(const endpoint_type& endpoint)
    {
        assert(is_valid());

        auto res = ::bind(this->native_handle(), endpoint.data(), endpoint.size());
        if (res == -1)
            throw_system_error("Failed to bind socket");
    }

    auto close() noexcept -> std::expected<void, std::error_code>
    {
        if (is_valid()) {
            auto res = ::close(fd_);
            fd_ = INVALID_SOCKET;

            if (res == -1)
                return unexpected_system_error();
        }

        return {};
    }

    [[nodiscard]]
    constexpr auto native_handle() const noexcept -> int
    {
        return fd_;
    }

    template<socket_option Option>
    void option(const Option& value)
    {
        auto res = ::setsockopt(fd_, Option::level, Option::name, value.data(), value.size());
        if (res == -1)
            throw_system_error("Failed to set socket option[{}]", Option::name);
    }

    template<socket_option Option>
    auto option() const -> Option
    {
        Option option{};
        auto size = static_cast<socklen_t>(option.size());
        auto res = ::getsockopt(fd_, Option::level, Option::name, option.data(), &size);
        if (res == -1)
            throw_system_error("Failed to get socket option[{}]", Option::name);

        if (size != option.size())
            throw std::runtime_error{ "Unexpected socket option size" };

        return option;
    }

    template<flag_option Option>
    void option(const Option& value)
    {
        auto current_flags = ::fcntl(fd_, Option::get_cmd);
        if (current_flags == -1)
            throw_system_error("Failed to get socket flags");

        auto new_flags = value ? (current_flags | Option::bit) : (current_flags & ~Option::bit);
        if (::fcntl(fd_, Option::set_cmd, new_flags) == -1)
            throw_system_error("Failed to set socket flags");
    }

    template<flag_option Option>
    auto option() const -> Option
    {
        auto current_flags = ::fcntl(fd_, Option::get_cmd);
        if (current_flags == -1)
            throw_system_error("Failed to get socket flags");

        if (current_flags & Option::bit)
            return Option{ true };

        return Option{ false };
    }

protected:
    explicit BasicSocket(int fd)
      : fd_{ fd }
    {}

private:
    static auto create(const Protocol& protocol) -> int
    {
        auto res = ::socket(protocol.domain(), protocol.type(), protocol.protocol());
        if (res == -1)
            throw_system_error("Failed to create socket");

        return res;
    }
};

} // namespace otter::net

#endif // OTTER_NET_SOCKET_H