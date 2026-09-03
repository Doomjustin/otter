#ifndef OTTER_NET_CONNECT_AWAITER_H
#define OTTER_NET_CONNECT_AWAITER_H

#include <liburing.h>

#include <otter/async.h>

namespace otter::net {

template<typename Protocol>
class ConnectAwaiter : public async::IOAwaiter<ConnectAwaiter<Protocol>> {
private:
    int fd_;
    typename Protocol::endpoint endpoint_;

public:
    ConnectAwaiter(int fd, const typename Protocol::endpoint& endpoint)
      : fd_{ fd }
      , endpoint_{ endpoint }
    {}

    void prepare(::io_uring_sqe* sqe) const noexcept
    {
        ::io_uring_prep_connect(sqe, fd_, endpoint_.data(), endpoint_.size());
    }
};

} // namespace otter::net

#endif // OTTER_NET_CONNECT_AWAITER_H