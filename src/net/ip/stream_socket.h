#ifndef OTTER_NET_IP_STREAM_SOCKET_H
#define OTTER_NET_IP_STREAM_SOCKET_H

#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

#include <net/connect_awaiter.h>
#include <net/recv_awaiter.h>
#include <net/send_awaiter.h>
#include <net/socket.h>
#include <otter/async.h>

namespace otter::net::ip {

/// @brief 面向流协议（TCP）的 socket 封装，继承自 BasicSocket。
///
/// 在 BasicSocket 基础上增加 connect/shutdown/send/receive 及
/// TCP 专属 socket option 支持。
///
/// @tparam Protocol 协议类型，需满足 socket_protocol（如 Tcp）。
template<typename Protocol>
class StreamSocket
  : public BasicSocket<Protocol>
  , public QueryRemoteEndpoint<StreamSocket<Protocol>> {
public:
    using base_type = BasicSocket<Protocol>;
    using endpoint_type = typename Protocol::endpoint;

    enum class how : std::uint8_t { receive = SHUT_RD, send = SHUT_WR, both = SHUT_RDWR };

    using keep_alive = BooleanOption<SOL_SOCKET, SO_KEEPALIVE>;
    using keep_alive_idle = ValueOption<IPPROTO_TCP, TCP_KEEPIDLE>;
    using keep_alive_interval = ValueOption<IPPROTO_TCP, TCP_KEEPINTVL>;
    using keep_alive_count = ValueOption<IPPROTO_TCP, TCP_KEEPCNT>;
    using no_delay = BooleanOption<IPPROTO_TCP, TCP_NODELAY>;

    StreamSocket() = default;

    explicit StreamSocket(const Protocol& protocol)
      : base_type{ protocol }
    {}

    explicit StreamSocket(int fd)
      : base_type{ fd }
    {}

    void connect(const endpoint_type& peer)
    {
        if (::connect(this->native_handle(), peer.data(), peer.size()) != 0)
            throw_system_error("Failed to connect socket");
    }

    auto async_connect(const endpoint_type& peer) noexcept -> ConnectAwaiter<Protocol>
    {
        return { this->native_handle(), peer };
    }

    auto shutdown(how how) noexcept -> std::expected<void, std::error_code>
    {
        if (::shutdown(this->native_handle(), std::to_underlying(how)) == -1)
            return unexpected_system_error();

        return {};
    }

    auto async_read_some(std::span<std::byte> buffer) noexcept -> RecvAwaiter
    {
        return { this->native_handle(), buffer };
    }

    template<std::ranges::contiguous_range T>
    auto async_read_some(T& range) noexcept -> RecvAwaiter
    {
        return async_read_some(buffer(range));
    }

    auto async_write_some(std::span<const std::byte> buffer) noexcept -> SendAwaiter
    {
        return { this->native_handle(), buffer };
    }

    template<std::ranges::contiguous_range T>
    auto async_write_some(const T& range) noexcept -> SendAwaiter
    {
        return async_write_some(buffer(range));
    }
};

} // namespace otter::net::ip

#endif // OTTER_NET_IP_STREAM_SOCKET_H