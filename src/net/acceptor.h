#ifndef OTTER_NET_ACCEPTOR_H
#define OTTER_NET_ACCEPTOR_H

#include <sys/socket.h>

#include <otter/async.h>
#include <otter/utility.h>

#include "option.h"
#include "socket.h"

namespace otter::net {

template<typename Protocol>
class AcceptAwaiter : public async::IOAwaiter<AcceptAwaiter<Protocol>, typename Protocol::socket> {
public:
    using endpoint_type = typename Protocol::endpoint;
    using socket_type = typename Protocol::socket;

private:
    int fd_;
    endpoint_type* endpoint_;
    socklen_t addrlen_{};

public:
    AcceptAwaiter(int fd, endpoint_type* endpoint = nullptr) noexcept
      : fd_{ fd }
      , endpoint_{ endpoint }
    {}

    void prepare(::io_uring_sqe* sqe) noexcept
    {
        ::io_uring_prep_accept(
            sqe, fd_, endpoint_ ? endpoint_->data() : nullptr, endpoint_ ? &addrlen_ : nullptr, 0);
    }

    // 覆写 value() 以在成功时返回 socket_type，并在 endpoint_ 非空时调整其大小。
    auto value() noexcept -> socket_type
    {
        if (endpoint_)
            endpoint_->resize(addrlen_);

        return socket_type{ this->result };
    }
};

template<typename Protocol>
class BasicAcceptor : public BasicSocket<Protocol> {
private:
    static constexpr auto MAX_LISTEN_CONNECTIONS = SOMAXCONN;

public:
    using socket_type = typename Protocol::socket;
    using endpoint_type = typename Protocol::endpoint;
    using base_type = BasicSocket<Protocol>;

    using reuse_address = BooleanOption<SOL_SOCKET, SO_REUSEADDR>;

    using reuse_port = BooleanOption<SOL_SOCKET, SO_REUSEPORT>;

    BasicAcceptor() = default;

    BasicAcceptor(const endpoint_type& endpoint, bool enable_reuse_port = false)
      : base_type{ endpoint.protocol() }
    {
        this->option(reuse_address{ true });

        if (enable_reuse_port)
            this->option(reuse_port{ true });

        this->bind(endpoint);
        listen(MAX_LISTEN_CONNECTIONS);
    }

    void listen(int backlog = MAX_LISTEN_CONNECTIONS)
    {
        auto res = ::listen(this->native_handle(), backlog);
        if (res == -1)
            throw_system_error("Failed to listen on socket");
    }

    auto async_accept() noexcept -> AcceptAwaiter<Protocol>
    {
        return AcceptAwaiter<Protocol>{ native_handle() };
    }

    auto async_accept(endpoint_type& endpoint) noexcept -> AcceptAwaiter<Protocol>
    {
        return AcceptAwaiter<Protocol>{ native_handle(), &endpoint };
    }

    [[nodiscard]]
    constexpr auto native_handle() const noexcept -> int
    {
        return base_type::native_handle();
    }
};

} // namespace otter::net

#endif // OTTER_NET_ACCEPTOR_H