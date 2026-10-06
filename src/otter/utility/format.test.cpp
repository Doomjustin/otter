#include "format.h" // IWYU pragma: keep

#include <format>
#include <ostream>
#include <string>
#include <system_error>

#include <catch2/catch_test_macros.hpp>

namespace {

struct ViaToString {
    [[nodiscard]]
    auto to_string() const -> std::string
    {
        return "string-view";
    }

    [[nodiscard]]
    auto to_repr() const -> std::string
    {
        return "repr-view";
    }
};

struct ViaToRepr {
    [[nodiscard]]
    auto to_repr() const -> std::string
    {
        return "repr-only";
    }
};

struct ViaOstream {
    int value{};
};

[[maybe_unused]] // 仅仅是为了避免未使用警告，实际上会被 std::format 使用
auto operator<<(std::ostream& os, const ViaOstream& value) -> std::ostream&
{
    os << "ostream:" << value.value;
    return os;
}

enum class SampleState { Ready, Done };

struct ViaFormatAs {
    int value{};
};

[[maybe_unused]] // 仅仅是为了避免未使用警告，实际上会被 std::format 使用
auto format_as(const ViaFormatAs& value) -> std::string
{
    return std::format("format-as:{}", value.value);
}

} // namespace

TEST_CASE("format uses format_as customization", "[otter.utility.format]")
{
    CHECK(std::format("{}", ViaFormatAs{ 7 }) == "format-as:7");
}

TEST_CASE("format uses to_string before to_repr", "[otter.utility.format]")
{
    CHECK(std::format("{}", ViaToString{}) == "string-view");
}

TEST_CASE("format uses to_repr fallback", "[otter.utility.format]")
{
    CHECK(std::format("{}", ViaToRepr{}) == "repr-only");
}

TEST_CASE("format renders enums by name", "[otter.utility.format]")
{
    CHECK(std::format("{}", SampleState::Ready) == "Ready");
    CHECK(std::format("{}", SampleState::Done) == "Done");
}

TEST_CASE("format falls back to ostream for user-defined type", "[otter.utility.format]")
{
    CHECK(std::format("{}", ViaOstream{ 42 }) == "ostream:42");
}

TEST_CASE("format maps error_code through format_as", "[otter.utility.format]")
{
    const std::error_code ec = std::make_error_code(std::errc::permission_denied);
    CHECK(std::format("{}", ec) == ec.message());
}
