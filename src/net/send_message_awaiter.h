#ifndef OTTER_NET_SEND_MESSAGE_AWAITER_H
#define OTTER_NET_SEND_MESSAGE_AWAITER_H

#include <liburing.h>

#include <otter/async.h>

namespace otter::net {

template<typename Protocol>
class SendMessageAwaiter : public async::IOAwaiter<SendMessageAwaiter<Protocol>, std::size_t> {
private:
    int fd_;
    std::span<const std::byte> buffer_;
    int flags_;

public:
    SendMessageAwaiter(int fd, std::span<const std::byte> buffer, int flags = 0)
      : fd_{ fd }
      , buffer_{ buffer }
      , flags_{ flags }
    {}

    void prepare(::io_uring_sqe* sqe) const noexcept
    {
        ::io_uring_prep_send(sqe, fd_, buffer_.data(), buffer_.size(), flags_);
    }
};

} // namespace otter::net

#endif // OTTER_NET_SEND_MESSAGE_AWAITER_H