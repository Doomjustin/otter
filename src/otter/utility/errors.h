#ifndef OTTER_UTILITY_ERRORS_H
#define OTTER_UTILITY_ERRORS_H

#include <cerrno>
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

inline std::unexpected<std::error_code> unexpected_system_error()
{
    return std::unexpected{ std::error_code{ errno, std::system_category() } };
}

inline std::unexpected<std::error_code> unexpected_system_error(std::errc ec)
{
    return std::unexpected{ std::make_error_code(ec) };
}

inline std::unexpected<std::error_code> unexpected_system_error(int error)
{
    return std::unexpected{ std::error_code{ error, std::system_category() } };
}

} // namespace otter

#endif // OTTER_UTILITY_ERRORS_H
