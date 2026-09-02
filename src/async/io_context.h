#ifndef OTTER_ASYNC_IO_CONTEXT_H
#define OTTER_ASYNC_IO_CONTEXT_H

#include <atomic>
#include <coroutine>
#include <cstdint>
#include <queue>
#include <utility>

#include <liburing.h>

#include <otter/utility.h>

namespace otter::async {

struct Operation {
    std::coroutine_handle<> handle;

    int result;
    std::uint32_t flags;

    Operation() = default;

    Operation(const Operation&) = delete;
    auto operator=(const Operation&) -> Operation& = delete;

    Operation(Operation&&) noexcept = default;
    auto operator=(Operation&&) noexcept -> Operation& = default;

    virtual ~Operation() = default;

    virtual void resume(int result, std::uint32_t flags)
    {
        this->result = result;
        this->flags = flags;

        std::exchange(handle, nullptr).resume();
    }
};

class IOContext {
private:
    struct CancelNode : public utility::MPSCQueueNode {
        Operation* operation;
    };

    static constexpr std::uint64_t WAKEUP_MARKER = 1ULL << 63;
    static constexpr std::uint64_t CANCEL_MARKER = 1ULL << 62;

    ::io_uring ring_;
    std::atomic<bool> should_stop_ = false;

    std::queue<std::coroutine_handle<>> ready_tasks_;
    utility::MPSCQueue<CancelNode> cancel_queue_;

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

    void stop();

    void cancel(Operation& operation);

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