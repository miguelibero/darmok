#include <catch2/catch_test_macros.hpp>
#include <darmok/serialize.hpp>
#include <cereal/archives/binary.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>

using namespace darmok;

namespace
{
    template<typename T>
    Data saveToData(const T& obj)
    {
        Data data;
        std::streamoff pos = 0;
        DataOutputStream stream(data);
        cereal::BinaryOutputArchive archive(stream);
        archive(obj);
        pos = stream.tellp();
        return data.view(0, pos);
    }

    template<typename T>
    void loadFromData(const Data& data, T& obj)
    {
        DataInputStream stream(data);
        cereal::BinaryInputArchive archive(stream);
        archive(obj);
    }
}

TEST_CASE( "any can be serialized", "[serialize]" )
{
	int i = 42;
    auto any = *entt::meta_any(&i);

    auto data = saveToData(any);
    i = 0;
    loadFromData(data, any);

    REQUIRE(i == 42);

}