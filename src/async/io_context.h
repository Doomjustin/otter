#ifndef OTTER_ASYNC_IO_CONTEXT_H
#define OTTER_ASYNC_IO_CONTEXT_H

#include <atomic>
#include <coroutine>
#include <cstdint>
#include <queue>

#include <liburing.h>

#include <otter/utility.h>

#include "operation.h"

namespace otter::async {

class IOContext {
private:
    struct CancelNode : public MPSCQueueNode {
        Operation* operation;
    };

    static constexpr std::uint64_t WAKEUP_MARKER = 1ULL << 63;
    static constexpr std::uint64_t CANCEL_MARKER = 1ULL << 62;

    ::io_uring ring_;
    std::atomic<bool> should_stop_ = false;
    int wakeup_fd_ = -1;
    std::atomic<std::uint64_t> tracked_operations_ = 0;

    std::queue<std::coroutine_handle<>> ready_tasks_;
    MPSCQueue<CancelNode> cancel_queue_;

public:
    explicit IOContext(std::uint32_t entries = 1024);

    IOContext(const IOContext&) = delete;
    auto operator=(const IOContext&) -> IOContext& = delete;

    IOContext(IOContext&&) = delete;
    auto operator=(IOContext&&) -> IOContext& = delete;

    ~IOContext();

    // 如果没有正在进行的操作，run 会停止
    void run();

    // 强行停止，目前的实现不会停止正在进行的操作，而是强制停止
    void stop();

    void submit(std::coroutine_handle<> task);

    void cancel(Operation& operation);

    void track();

    void untrack();

    auto sqe() -> ::io_uring_sqe*;

private:
    void process_cqes();

    void process_readies();

    void process_cancels();

    void arm_wakeup() noexcept;

    void wakeup() const noexcept;

    void resume_wakeup() const noexcept;
};

} // namespace otter::async

#endif // OTTER_ASYNC_IO_CONTEXT_H