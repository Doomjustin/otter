#ifndef OTTER_UTILITY_RESULT_H
#define OTTER_UTILITY_RESULT_H

#include <expected>
#include <system_error>

namespace otter {

template<typename T = void>
using Result = std::expected<T, std::error_code>;

} // namespace otter

#endif // OTTER_UTILITY_RESULT_H
