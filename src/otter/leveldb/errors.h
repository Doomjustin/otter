#ifndef OTTER_LEVELDB_ERRORS_H
#define OTTER_LEVELDB_ERRORS_H

#include <string>
#include <system_error>
#include <type_traits>

namespace otter::leveldb {

enum class ErrorCode {
    NotFound,
    Corruption,
    NotSupported,
    InvalidArgument,
    IOError,
};

class LevelDBErrorCategory final : public std::error_category {
public:
    [[nodiscard]]
    const char* name() const noexcept override
    {
        return "otter.leveldb_error";
    }

    [[nodiscard]]
    std::string message(int ev) const override;
};

// 此函数用于支持std的隐式转换操作，类似一种AOP
std::error_code make_error_code(ErrorCode ec) noexcept;

} // namespace otter::leveldb

namespace std {

// 注册以让std能识别出来
template<>
struct is_error_code_enum<otter::leveldb::ErrorCode> : std::true_type {};

} // namespace std

#endif // OTTER_LEVELDB_ERRORS_H
