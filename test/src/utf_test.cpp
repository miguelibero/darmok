#include <catch2/catch_test_macros.hpp>
#include <darmok/utf.hpp>
#include <string>

using namespace darmok;

TEST_CASE( "Utf8Char encodes and decodes", "[utf8-char]" )
{
    UtfChar chr(u8"🌍");
    REQUIRE(chr.toUtf8String() == u8"🌍");
    REQUIRE(chr.code == 0x1F30D);
    REQUIRE(chr.length() == 4);
}