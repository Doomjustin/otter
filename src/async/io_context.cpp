#include "io_context.h"

#include <atomic>

#include <liburing.h>
#include <poll.h>
#include <sys/eventfd.h>

#include <otter/utility.h>

namespace otter::async {

IOContext::IOContext(std::uint32_t entries)
{
    if (auto res = ::io_uring_queue_init(entries, &ring_, 0); res < 0)
        utility::throw_system_error(-res, "io_uring_queue_init failed");

    wakeup_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (wakeup_fd_ < 0)
        utility::throw_system_error("created eventfd failed");

    arm_wakeup();
}

IOContext::~IOContext()
{
    ::io_uring_queue_exit(&ring_);
    if (wakeup_fd_ >= 0)
        ::close(wakeup_fd_);
}

void IOContext::run()
{
    while (!should_stop_.load(std::memory_order_relaxed)) {
        process_cancels();
        process_readies();
        process_cqes();
    }
}

auto IOContext::sqe() -> ::io_uring_sqe*
{
    auto* sqe = ::io_uring_get_sqe(&ring_);
    // Retry: 提交已有的SQE以获取新的SQE
    if (!sqe) {
        ::io_uring_submit(&ring_);
        return ::io_uring_get_sqe(&ring_);
    }

    return sqe;
}

void IOContext::stop()
{
    should_stop_.store(true, std::memory_order_relaxed);
    wakeup();
}

void IOContext::cancel(Operation& operation)
{
    cancel_queue_.push(new CancelNode{ .operation = &operation });
    wakeup();
}

void IOContext::submit(std::coroutine_handle<> task)
{
    ready_tasks_.push(task);
    wakeup();
}

void IOContext::process_cqes()
{
    auto res = ::io_uring_submit_and_wait(&ring_, 1);
    if (res < 0) {
        if (res == -EINTR)
            return;

        utility::throw_system_error(-res, "io_uring_submit_and_wait failed");
    }

    unsigned count{ 0 };
    unsigned head;
    ::io_uring_cqe* cqe{ nullptr };

    io_uring_for_each_cqe(&ring_, head, cqe)
    {
        ++count;

        auto data = ::io_uring_cqe_get_data64(cqe);

        if (data == WAKEUP_MARKER) {
            resume_wakeup();

            if (!(cqe->flags & IORING_CQE_F_MORE))
                arm_wakeup();

            continue;
        }

        // 取消本身不需要处理，直接取出来就行
        if (data == CANCEL_MARKER)
            continue;

        if (data != 0) {
            auto* p = static_cast<Operation*>(::io_uring_cqe_get_data(cqe));
            p->resume(cqe->res, cqe->flags);
        }
    }

    if (count > 0)
        ::io_uring_cq_advance(&ring_, count);
}

void IOContext::process_readies()
{
    while (!ready_tasks_.empty()) {
        auto handle = ready_tasks_.front();
        if (handle && !handle.done())
            handle.resume();

        ready_tasks_.pop();
    }
}

void IOContext::process_cancels()
{
    auto* node = cancel_queue_.pop_all();
    while (node) {
        auto* cancel_sqe = sqe();
        ::io_uring_prep_cancel(cancel_sqe, node->operation, 0);
        ::io_uring_sqe_set_data64(cancel_sqe, CANCEL_MARKER);
        ::io_uring_submit(&ring_);
        auto* next = static_cast<CancelNode*>(node->mpsc_next.load(std::memory_order_relaxed));
        delete node;
        node = next;
    }
}

void IOContext::arm_wakeup() noexcept
{
    auto* wakeup_sqe = sqe();

    ::io_uring_prep_poll_multishot(wakeup_sqe, wakeup_fd_, POLLIN);
    ::io_uring_sqe_set_data64(wakeup_sqe, WAKEUP_MARKER);
    ::io_uring_submit(&ring_);
}

void IOContext::wakeup() const noexcept
{
    std::uint64_t one = 1;
    ::write(wakeup_fd_, &one, sizeof(one));
}

void IOContext::resume_wakeup() const noexcept
{
    std::uint64_t one;
    ::read(wakeup_fd_, &one, sizeof(one));
}

} // namespace otter::async