#include "errors.h"

#include <utility>

namespace otter::leveldb {

namespace {

const std::error_category& leveldb_error_category()
{
    static LevelDBErrorCategory category{};
    return category;
}

} // namespace

std::string LevelDBErrorCategory::message(int ev) const
{
    switch (static_cast<ErrorCode>(ev)) {
    case ErrorCode::NotFound:
        return "Not found";
    case ErrorCode::Corruption:
        return "Corruption";
    case ErrorCode::NotSupported:
        return "Not supported";
    case ErrorCode::InvalidArgument:
        return "Invalid argument";
    case ErrorCode::IOError:
        return "I/O error";
    default:
        return "Unknown error";
    }
}

std::error_code make_error_code(ErrorCode ec) noexcept
{
    return { std::to_underlying(ec), leveldb_error_category() };
}

} // namespace otter::leveldb
