#ifndef OTTER_NET_IP_UDP_H
#define OTTER_NET_IP_UDP_H

#include <sys/socket.h>

#include <net/acceptor.h>

#include "address.h"
#include "datagram_socket.h"
#include "endpoint.h"

namespace otter::net::ip {

class udp {
private:
    int domain_ = AF_INET;

    explicit udp(int domain)
      : domain_{ domain }
    {}

public:
    using address = Address;
    using endpoint = BasicEndpoint<udp>;
    using socket = DatagramSocket<udp>;
    using acceptor = BasicAcceptor<udp>;

    udp() = default;

    [[nodiscard]]
    constexpr auto domain() const noexcept -> int
    {
        return domain_;
    }

    [[nodiscard]]
    consteval auto type() const noexcept -> int
    {
        return SOCK_DGRAM;
    }

    [[nodiscard]]
    consteval auto protocol() const noexcept -> int
    {
        return IPPROTO_UDP;
    }

    static auto v4() noexcept -> udp
    {
        return udp{ AF_INET };
    }

    static auto v6() noexcept -> udp
    {
        return udp{ AF_INET6 };
    }
};

} // namespace otter::net::ip

#endif // OTTER_NET_IP_UDP_H