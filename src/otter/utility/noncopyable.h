#ifndef OTTER_UTILITY_NONCOPYABLE_H
#define OTTER_UTILITY_NONCOPYABLE_H

namespace otter {

class NonCopyable {
protected:
    NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

} // namespace otter

#endif // OTTER_UTILITY_NONCOPYABLE_H
