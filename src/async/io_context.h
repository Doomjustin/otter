#ifndef OTTER_ASYNC_IO_CONTEXT_H
#define OTTER_ASYNC_IO_CONTEXT_H

#include <atomic>
#include <cstdint>

#include <liburing.h>

namespace otter::async {

class IOContext {
private:
    static constexpr std::uint64_t WAKEUP_FLAG = 1ULL << 63;
    static constexpr std::uint64_t CANCEL_FLAG = 2ULL << 62;

    ::io_uring ring_;
    std::atomic<std::size_t> tracking_works_ = 0;
    std::atomic<bool> should_stop_ = false;

public:
};

} // namespace otter::async

#endif // OTTER_ASYNC_IO_CONTEXT_H