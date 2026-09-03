#ifndef OTTER_ASYNC_WRITE_AWAITER_H
#define OTTER_ASYNC_WRITE_AWAITER_H

#include <cstddef>
#include <cstdint>
#include <span>

#include <liburing.h>

#include "awaiter.h"

namespace otter::async {

class WriteAwaiter : public IOAwaiter<WriteAwaiter> {
private:
    int fd_;
    std::span<const std::byte> buffer_;
    std::uint64_t offset_ = 0;

public:
    WriteAwaiter(int fd, std::span<const std::byte> buffer, std::uint64_t offset = 0)
      : fd_{ fd }
      , buffer_{ buffer }
      , offset_{ offset }
    {}

    void prepare(::io_uring_sqe* sqe)
    {
        ::io_uring_prep_write(sqe, fd_, buffer_.data(), buffer_.size(), offset_);
    }
};

} // namespace otter::async

#endif // OTTER_ASYNC_WRITE_AWAITER_H