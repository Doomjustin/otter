#include "io_context.h"

#include <atomic>

#include <liburing.h>
#include <poll.h>
#include <sys/eventfd.h>

#include <otter/utility.h>

#include "awaiter.h"

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
        process_ready_tasks();
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

    unsigned count = 0;
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
        }

        if (data != WAKEUP_MARKER) {
            auto* p = reinterpret_cast<Awaiter*>(&data);
            p->resume(cqe->res, cqe->flags);
        }

        if (count > 0)
            ::io_uring_cq_advance(&ring_, count);
    }
}

void IOContext::process_ready_tasks()
{
    while (!ready_tasks_.empty()) {
        auto handle = ready_tasks_.front();
        ready_tasks_.pop();

        if (handle && !handle.done())
            handle.resume();

        if (handle && handle.done())
            handle.destroy();
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