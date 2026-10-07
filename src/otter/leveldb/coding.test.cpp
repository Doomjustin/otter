#include "coding.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>

namespace {

auto as_u8(std::byte value) -> std::uint8_t
{
    return std::to_integer<std::uint8_t>(value);
}

} // namespace

TEST_CASE("varint computes encoded length", "[coding][varint]")
{
    CHECK(otter::leveldb::varint::length<std::uint64_t>(0) == 1);
    CHECK(otter::leveldb::varint::length<std::uint64_t>(127) == 1);
    CHECK(otter::leveldb::varint::length<std::uint64_t>(128) == 2);
    CHECK(otter::leveldb::varint::length<std::uint64_t>(16383) == 2);
    CHECK(otter::leveldb::varint::length<std::uint64_t>(16384) == 3);
}

TEST_CASE("varint encodes and decodes with byte iterators", "[coding][varint]")
{
    constexpr std::uint64_t value = 300;
    std::vector<std::byte> output(8);

    const auto end = otter::leveldb::varint::encode(output.begin(), value);
    const auto written = static_cast<std::size_t>(end - output.begin());
    CHECK(written == 2);
    CHECK(as_u8(output[0]) == 0xAC);
    CHECK(as_u8(output[1]) == 0x02);

    const auto [decoded, iter] = otter::leveldb::varint::decode<std::uint64_t>(output.cbegin());
    CHECK(decoded == value);
    CHECK(static_cast<std::size_t>(iter - output.begin()) == written);
}

TEST_CASE("varint encodes and decodes with char iterators", "[coding][varint]")
{
    constexpr std::uint32_t value = 0xFFFF;
    std::vector<char> output(8, '\0');

    const auto end = otter::leveldb::varint::encode(output.begin(), value);
    const auto written = static_cast<std::size_t>(end - output.begin());
    CHECK(written == 3);

    const auto [decoded, iter] = otter::leveldb::varint::decode<std::uint32_t>(output.cbegin());
    CHECK(decoded == value);
    CHECK(static_cast<std::size_t>(iter - output.begin()) == written);
}

TEST_CASE("fixed encodes and decodes with byte iterators", "[coding][fixed]")
{
    constexpr std::uint32_t value = 0x12345678;
    std::vector<std::byte> output(sizeof(value));

    const auto end = otter::leveldb::fixed::encode(output.begin(), value);
    CHECK(static_cast<std::size_t>(end - output.begin()) == sizeof(value));
    CHECK(as_u8(output[0]) == 0x78);
    CHECK(as_u8(output[1]) == 0x56);
    CHECK(as_u8(output[2]) == 0x34);
    CHECK(as_u8(output[3]) == 0x12);

    const auto [decoded, iter] = otter::leveldb::fixed::decode<std::uint32_t>(output.cbegin());
    CHECK(decoded == value);
    CHECK(static_cast<std::size_t>(iter - output.begin()) == sizeof(value));
}

TEST_CASE("fixed encodes and decodes with char iterators", "[coding][fixed]")
{
    constexpr std::uint16_t value = 0xABCD;
    std::vector<char> output(sizeof(value), '\0');

    const auto end = otter::leveldb::fixed::encode(output.begin(), value);
    CHECK(static_cast<std::size_t>(end - output.begin()) == sizeof(value));

    const auto [decoded, iter] = otter::leveldb::fixed::decode<std::uint16_t>(output.cbegin());
    CHECK(decoded == value);
    CHECK(static_cast<std::size_t>(iter - output.begin()) == sizeof(value));
}

TEST_CASE("pack and unpack preserve byte order", "[coding][pack]")
{
    using otter::leveldb::pack;
    using otter::leveldb::pack_bytes;
    using otter::leveldb::unpack;

    constexpr std::uint32_t packed =
        pack(pack(std::uint32_t{ 0x12 }, pack_bytes<1>(std::uint32_t{ 0x34 })),
             pack_bytes<2>(std::uint32_t{ 0x5678 }));

    CHECK(packed == 0x12345678U);

    const auto [low16, rest16] = unpack<std::uint16_t, 2>(packed);
    CHECK(low16 == 0x5678U);
    CHECK(rest16 == 0x1234U);

    const auto [low8, rest8] = unpack<std::uint8_t, 1>(rest16);
    CHECK(low8 == 0x34U);
    CHECK(rest8 == 0x12U);
}
