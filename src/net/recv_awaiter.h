#ifndef OTTER_NET_RECV_AWAITER_H
#define OTTER_NET_RECV_AWAITER_H

#include <cstddef>
#include <span>

#include <liburing.h>

#include <otter/async.h>

namespace otter::net {

class RecvAwaiter : public async::IOAwaiter<RecvAwaiter, std::size_t> {
private:
    int fd_;
    std::span<std::byte> buffer_;
    int flags_;

public:
    RecvAwaiter(int fd, std::span<std::byte> buffer, int flags = 0)
      : fd_{ fd }
      , buffer_{ buffer }
      , flags_{ flags }
    {}

    void prepare(::io_uring_sqe* sqe) const noexcept
    {
        ::io_uring_prep_recv(sqe, fd_, buffer_.data(), buffer_.size(), flags_);
    }
};

} // namespace otter::net

#endif // OTTER_NET_RECV_AWAITER_H