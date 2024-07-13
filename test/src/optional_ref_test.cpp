#include <catch2/catch_test_macros.hpp>
#include <darmok/optional_ref.hpp>

using namespace darmok;

TEST_CASE( "optional ref keeps a reference", "[optional-ref]" )
{
    int v = 42;
    OptionalRef<int> ref;
    REQUIRE(!ref);
    REQUIRE(ref.empty());
    REQUIRE(ref.ptr() == nullptr);
    ref = v;
    REQUIRE(ref);
    REQUIRE(!ref.empty());
    REQUIRE(ref.value() == v);
    REQUIRE(*ref == v);
    REQUIRE(ref.ptr() == &v);

    v = 33;
    REQUIRE(ref.value() == v);
    *ref = 44;
    REQUIRE(ref.value() == 44);
    REQUIRE(v == 44);
}