#include <catch2/catch_test_macros.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>

#include <nlohmann/json.hpp>

using namespace darmok;

TEST_CASE( "data can be serialized", "[data]" )
{
    Data data;
    DataOutputStream out{ data };
    out << "{\"key\":42}";
    auto dataView = data.view(0, out.tellp());
    auto json = nlohmann::json::parse(DataInputStream{ dataView });
    REQUIRE(json["key"] == 42);
}
