#ifndef OTTER_NET_RECEIVE_MESSAGE_AWAITER_H
#define OTTER_NET_RECEIVE_MESSAGE_AWAITER_H

#include <liburing.h>

#include <otter/async.h>

namespace otter::net {

template<typename Protocol>
class ReceiveMessageAwaiter
  : public async::IOAwaiter<ReceiveMessageAwaiter<Protocol>, std::size_t> {
public:
    using endpoint_type = typename Protocol::endpoint;

private:
    int fd_;
    std::span<std::byte> buffer_;
    endpoint_type* endpoint_;
    int flags_;

public:
    ReceiveMessageAwaiter(int fd,
                          std::span<std::byte> buffer,
                          endpoint_type* endpoint = nullptr,
                          int flags = 0)
      : fd_{ fd }
      , buffer_{ buffer }
      , endpoint_{ endpoint }
      , flags_{ flags }
    {}

    void prepare(::io_uring_sqe* sqe) const noexcept
    {
        iovec iov{ .iov_base = buffer_.data(), .iov_len = buffer_.size() };
        msghdr msg{};
        msg.msg_iov = &iov;
        msg.msg_iovlen = 1;
        msg.msg_name = endpoint_ ? endpoint_->data() : nullptr;
        msg.msg_namelen = endpoint_ ? endpoint_->capacity() : 0;

        ::io_uring_prep_recvmsg(sqe, fd_, &msg, flags_);
    }
};

} // namespace otter::net

#endif // OTTER_NET_RECEIVE_MESSAGE_AWAITER_H