#ifndef OTTER_NET_IP_TCP_H
#define OTTER_NET_IP_TCP_H

#include <sys/socket.h>

#include <net/acceptor.h>

#include "address.h"
#include "endpoint.h"
#include "stream_socket.h"

namespace otter::net::ip {

class tcp {
private:
    int domain_ = AF_INET;

    explicit tcp(int domain)
      : domain_{ domain }
    {}

public:
    using address = Address;
    using endpoint = BasicEndpoint<tcp>;
    using socket = StreamSocket<tcp>;
    using acceptor = BasicAcceptor<tcp>;

    tcp() = default;

    [[nodiscard]]
    constexpr auto domain() const noexcept -> int
    {
        return domain_;
    }

    [[nodiscard]]
    consteval auto type() const noexcept -> int
    {
        return SOCK_STREAM;
    }

    [[nodiscard]]
    consteval auto protocol() const noexcept -> int
    {
        return IPPROTO_TCP;
    }

    static auto v4() noexcept -> tcp
    {
        return tcp{ AF_INET };
    }

    static auto v6() noexcept -> tcp
    {
        return tcp{ AF_INET6 };
    }
};

} // namespace otter::net::ip

#endif // OTTER_NET_IP_TCP_H