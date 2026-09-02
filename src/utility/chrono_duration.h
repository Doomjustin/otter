#ifndef OTTER_UTILITY_CHRONO_DURATION_H
#define OTTER_UTILITY_CHRONO_DURATION_H

#include <chrono>
#include <type_traits>

namespace otter {

template<typename T>
struct is_chrono_duration_impl : std::false_type {};

template<typename Rep, typename Period>
struct is_chrono_duration_impl<std::chrono::duration<Rep, Period>> : std::true_type {};

template<typename T>
concept chrono_duration = is_chrono_duration_impl<std::remove_cvref_t<T>>::value;

} // namespace otter

#endif // OTTER_UTILITY_CHRONO_DURATION_H