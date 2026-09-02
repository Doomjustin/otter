#ifndef OTTER_UTILITY_EXCEPTIONS_H
#define OTTER_UTILITY_EXCEPTIONS_H

#include <expected>
#include <format>
#include <system_error>

namespace otter {

template<typename... Args>
void throw_system_error(int error, std::format_string<Args...> fmt, Args&&... args)
{
    throw std::system_error{ error,
                             std::generic_category(),
                             std::format(fmt, std::forward<Args>(args)...) };
}

template<typename... Args>
void throw_system_error(std::format_string<Args...> fmt, Args&&... args)
{
    throw std::system_error{ errno,
                             std::generic_category(),
                             std::format(fmt, std::forward<Args>(args)...) };
}

auto unexpected_system_error() -> std::unexpected<std::error_code>;

auto unexpected_system_error(std::errc ec) -> std::unexpected<std::error_code>;

auto unexpected_system_error(int error) -> std::unexpected<std::error_code>;

} // namespace otter

#endif // OTTER_UTILITY_EXCEPTIONS_H