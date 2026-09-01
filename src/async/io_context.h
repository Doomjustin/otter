#ifndef OTTER_ASYNC_IO_CONTEXT_H
#define OTTER_ASYNC_IO_CONTEXT_H

#include <atomic>
#include <coroutine>
#include <cstdint>
#include <queue>

#include <liburing.h>

namespace otter::async {

class IOContext {
private:
    static constexpr std::uint64_t WAKEUP_MARKER = 1ULL << 63;

    ::io_uring ring_;
    std::atomic<bool> should_stop_ = false;

    std::queue<std::coroutine_handle<>> ready_tasks_;

    int wakeup_fd_ = -1;

public:
    explicit IOContext(std::uint32_t entries = 1024);

    IOContext(const IOContext&) = delete;
    auto operator=(const IOContext&) -> IOContext& = delete;

    IOContext(IOContext&&) = delete;
    auto operator=(IOContext&&) -> IOContext& = delete;

    ~IOContext();

    void submit(std::coroutine_handle<> task);

    void run();

    auto sqe() -> ::io_uring_sqe*;

    void stop()
    {
        should_stop_.store(true, std::memory_order_relaxed);
        wakeup();
    }

private:
    void process_cqes();

    void process_ready_tasks();

    void arm_wakeup() noexcept;

    void wakeup() const noexcept;

    void resume_wakeup() const noexcept;
};

} // namespace otter::async

#endif // OTTER_ASYNC_IO_CONTEXT_H