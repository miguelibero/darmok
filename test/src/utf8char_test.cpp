#include <catch2/catch_test_macros.hpp>
#include <darmok/utf8.hpp>
#include <string>

using namespace darmok;

TEST_CASE( "Utf8Char encodes and decodes", "[darmok-core]" ) {
    Utf8Char chr(u8"🌍");
    REQUIRE(chr.to_u8string() == u8"🌍");
    REQUIRE(chr.code == 0x1F30D);
    REQUIRE(chr.length() == 4);
}