#ifndef OTTER_ASYNC_READ_AWAITER_H
#define OTTER_ASYNC_READ_AWAITER_H

#include <cstddef>
#include <cstdint>
#include <span>

#include <liburing.h>

#include "awaiter.h"

namespace otter::async {

class ReadAwaiter : public IOAwaiter<ReadAwaiter> {
private:
    int fd_;
    std::span<std::byte> buffer_;
    std::uint64_t offset_;

public:
    ReadAwaiter(int fd, std::span<std::byte> buffer, std::uint64_t offset = 0)
      : fd_{ fd }
      , buffer_{ buffer }
      , offset_{ offset }
    {}

    void prepare(io_uring_sqe* sqe)
    {
        ::io_uring_prep_read(sqe, fd_, buffer_.data(), buffer_.size(), offset_);
    }
};

} // namespace otter::async

#endif // OTTER_ASYNC_READ_AWAITER_H