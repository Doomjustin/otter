#ifndef OTTER_ASYNC_SEND_AWAITER_H
#define OTTER_ASYNC_SEND_AWAITER_H

#include <cstddef>
#include <span>

#include <liburing.h>

#include "awaiter.h"

namespace otter::async {

class SendAwaiter : public IOAwaiter<SendAwaiter, std::size_t> {
private:
    int fd_;
    std::span<const std::byte> buffer_;
    int flags_;

public:
    SendAwaiter(int fd, std::span<const std::byte> buffer, int flags = 0)
      : fd_{ fd }
      , buffer_{ buffer }
      , flags_{ flags }
    {}

    void prepare(::io_uring_sqe* sqe) const noexcept
    {
        ::io_uring_prep_send(sqe, fd_, buffer_.data(), buffer_.size(), flags_);
    }
};

} // namespace otter::async

#endif // OTTER_ASYNC_SEND_AWAITER_H