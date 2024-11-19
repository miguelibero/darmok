#include <catch2/catch_test_macros.hpp>
#include <darmok/serialize.hpp>
#include <cereal/archives/binary.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/reflect.hpp>

using namespace darmok;
using namespace entt::literals;

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

    struct TestStruct final
    {
        int value;
        std::string str;

        static void bindMeta()
        {
            entt::meta<TestStruct>().type("TestStruct"_hs)
                .data<&TestStruct::value, entt::as_ref_t>("value"_hs)
                .data<&TestStruct::str, entt::as_ref_t>("str"_hs);
        }
    };

    struct TestSerializeStruct final
    {
        float value;

        void serialize(const entt::meta_any& archive) const
        {
            archive.invoke("process"_hs, CEREAL_NVP(value));
        }

        static void bindMeta()
        {
            entt::meta<TestSerializeStruct>().type("TestSerializeStruct"_hs)
                .func<&TestSerializeStruct::serialize>("serialize"_hs);
        }
    };
}

TEST_CASE( "any can be serialized", "[serialize]" )
{
	int i = 42;
    auto any = entt::forward_as_meta(i);

    auto data = saveToData(any);
    i = 0;
    loadFromData(data, any);

    REQUIRE(i == 42);
}

TEST_CASE("struct any can be serialized", "[serialize]")
{
    TestStruct::bindMeta();
    TestStruct v{ 42, "lala" };
    auto any = entt::forward_as_meta(v);

    auto data = saveToData(any);
    v.value = 0;
    v.str = "";
    loadFromData(data, any);

    REQUIRE(v.value == 42);
    REQUIRE(v.str == "lala");
}

TEST_CASE("custom serialize function is called", "[serialize]")
{
    /*
    ReflectionUtils::bind();
    TestSerializeStruct::bindMeta();
    TestSerializeStruct v{ 3.14F };

    auto any = entt::forward_as_meta(v);

    auto data = saveToData(any);
    v.value = 0;
    loadFromData(data, any);

    REQUIRE(v.value == 3.14F);
    */
}