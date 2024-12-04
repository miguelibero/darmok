#include <catch2/catch_test_macros.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>

#include <cereal/archives/binary.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/json.hpp>

using namespace darmok;

template<typename OutputArchive, typename InputArchive>
Data saveAndLoadData(const DataView& dataView)
{
    Data serializedData;
    DataOutputStream stream(serializedData);
    {
        OutputArchive archive(stream);
        archive(dataView);
    }
    auto pos = stream.tellp();
    Data data;
    {
        DataInputStream stream(serializedData.view(0, pos));
        InputArchive archive(stream);
        archive(data);
    }
    return data;
}

TEST_CASE( "data can be serialized in binary", "[data]" )
{
    static const int array[] = { 42, 666 };
    auto dataView = DataView::fromStatic(array);

    auto data = saveAndLoadData<cereal::BinaryOutputArchive, cereal::BinaryInputArchive>(dataView);

    REQUIRE(data.view() == dataView);
}

TEST_CASE("data can be serialized in xml", "[data]")
{
    static const int array[] = { 42, 666 };
    auto dataView = DataView::fromStatic(array);

    auto data = saveAndLoadData<cereal::XMLOutputArchive, cereal::XMLInputArchive>(dataView);

    REQUIRE(data.view() == dataView);
}

TEST_CASE("data can be serialized in json", "[data]")
{
    static const int array[] = { 42, 666 };
    auto dataView = DataView::fromStatic(array);

    auto data = saveAndLoadData<cereal::JSONOutputArchive, cereal::JSONInputArchive>(dataView);

    REQUIRE(data.view() == dataView);
}