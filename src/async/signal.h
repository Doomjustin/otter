#ifndef OTTER_ASYNC_SIGNAL_H
#define OTTER_ASYNC_SIGNAL_H

#include <csignal>

#include <liburing.h>
#include <sys/signalfd.h>

#include <otter/utility.h>

#include "awaiter.h"

namespace otter::async {

class Signal {
public:
    explicit constexpr Signal(int signal)
      : signal_{ signal }
    {}

    auto operator==(const Signal&) const noexcept -> bool = default;

    constexpr operator int() const noexcept
    {
        return signal_;
    }

    [[nodiscard]]
    constexpr auto value() const noexcept
    {
        return signal_;
    }

private:
    int signal_;
};

struct signals {
    signals() = delete;

    template<typename... Sig>
        requires(std::same_as<Sig, Signal> && ...)
    static void block(Sig... signal)
    {
        sigset_t mask;
        ::sigemptyset(&mask);
        (::sigaddset(&mask, signal), ...);

        if (auto err = ::pthread_sigmask(SIG_BLOCK, &mask, nullptr); err != 0)
            throw_system_error(err, "Failed to block signals");
    }

    static constexpr auto interrupt = Signal{ SIGINT };
    static constexpr auto terminate = Signal{ SIGTERM };
    static constexpr auto quit = Signal{ SIGQUIT };
    static constexpr auto hangup = Signal{ SIGHUP };
    static constexpr auto child = Signal{ SIGCHLD };
    static constexpr auto user1 = Signal{ SIGUSR1 };
    static constexpr auto user2 = Signal{ SIGUSR2 };
    static constexpr auto alarm = Signal{ SIGALRM };
    static constexpr auto broken_pipe = Signal{ SIGPIPE };
    static constexpr auto continue_ = Signal{ SIGCONT };
    static constexpr auto terminal_stop = Signal{ SIGTSTP };
    static constexpr auto window_change = Signal{ SIGWINCH };
};

class SignalAwaiter : public IOAwaiter<SignalAwaiter, Signal> {
private:
    int fd_;
    ::signalfd_siginfo info_{};

public:
    SignalAwaiter(int fd) noexcept
      : fd_{ fd }
    {}

    void prepare(::io_uring_sqe* sqe) noexcept
    {
        ::io_uring_prep_read(sqe, fd_, &info_, sizeof(info_), 0);
    }

    auto value() const noexcept -> Signal
    {
        // 错误值由 IOAwaiter 基类处理，这里仅返回成功读取的信号即可。
        return Signal{ static_cast<int>(info_.ssi_signo) };
    }
};

class SignalSet {
private:
    int fd_{ -1 };
    sigset_t mask_;

public:
    template<typename... Signals>
        requires(std::same_as<Signals, Signal> && ...)
    SignalSet(Signals... sigal)
    {
        ::sigemptyset(&mask_);
        (::sigaddset(&mask_, sigal), ...);

        if (auto err = ::pthread_sigmask(SIG_BLOCK, &mask_, nullptr); err != 0)
            throw_system_error(err, "Failed to block signals");

        fd_ = ::signalfd(-1, &mask_, SFD_NONBLOCK | SFD_CLOEXEC);
        if (fd_ == -1)
            throw_system_error("Failed to create signalfd");
    }

    SignalSet(const SignalSet&) = delete;
    auto operator=(const SignalSet&) -> SignalSet& = delete;

    SignalSet(SignalSet&& other) noexcept
      : fd_{ std::exchange(other.fd_, -1) }
      , mask_{ other.mask_ }
    {}

    auto operator=(SignalSet&& other) noexcept -> SignalSet& = delete;

    ~SignalSet()
    {
        if (fd_ != -1)
            ::close(fd_);
    }

    auto async_wait() const noexcept -> SignalAwaiter
    {
        return SignalAwaiter{ fd_ };
    }
};

inline auto format_as(const Signal& signal) -> std::string_view
{
    switch (signal) {
    case signals::interrupt:
        return "interrupt";
    case signals::terminate:
        return "terminate";
    case signals::quit:
        return "quit";
    case signals::hangup:
        return "hangup";
    case signals::child:
        return "child";
    case signals::user1:
        return "user1";
    case signals::user2:
        return "user2";
    case signals::alarm:
        return "alarm";
    case signals::broken_pipe:
        return "broken_pipe";
    case signals::continue_:
        return "continue";
    case signals::terminal_stop:
        return "terminal_stop";
    case signals::window_change:
        return "window_change";
    default:
        return "unknown";
    }
}

} // namespace otter::async

#endif // OTTER_ASYNC_SIGNAL_H