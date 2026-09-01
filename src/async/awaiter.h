#ifndef OTTER_ASYNC_AWAITER_H
#define OTTER_ASYNC_AWAITER_H

#include <coroutine>
#include <cstdint>
#include <utility>

namespace otter::async {

struct Awaiter {
    std::coroutine_handle<> handle;

    int result;
    std::uint32_t flags;

    Awaiter() = default;

    Awaiter(const Awaiter&) = delete;
    auto operator=(const Awaiter&) -> Awaiter& = delete;

    Awaiter(Awaiter&&) = delete;
    auto operator=(Awaiter&&) -> Awaiter& = delete;

    virtual ~Awaiter() = default;

    virtual void resume(int result, std::uint32_t flags)
    {
        this->result = result;
        this->flags = flags;
        
        std::exchange(handle, nullptr).resume();
    }
};

} // namespace otter::async

#endif // OTTER_ASYNC_AWAITER_H