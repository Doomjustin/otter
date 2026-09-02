#ifndef OTTER_ASYNC_OPERATION_H
#define OTTER_ASYNC_OPERATION_H

#include <coroutine>
#include <cstdint>
#include <utility>

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

} // namespace otter::async

#endif // OTTER_ASYNC_OPERATION_H