#include <catch2/catch_test_macros.hpp>
#include <cereal/archives/binary.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/reflect_serialize.hpp>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <map>

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

        TestSerializeStruct() = default;

        TestSerializeStruct(const TestSerializeStruct& other)
            : value(other.value)
        {
        }

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(value));
        }

        static void bindMeta()
        {
            auto factory = entt::meta<TestSerializeStruct>().type("TestSerializeStruct"_hs);
            ReflectionSerializeUtils::metaSerialize<TestSerializeStruct>();
        }
    };

    template<typename Archive, typename T>
    void checkSerialization()
    {
        auto i1 = cereal::traits::detail::count_specializations<T, Archive>::value;
        auto i2 = cereal::traits::has_member_save<T, Archive>::value; // this
        auto i3 = cereal::traits::has_non_member_save<T, Archive>::value;
        auto i4 = cereal::traits::has_member_serialize<T, Archive>::value;
        auto i5 = cereal::traits::has_non_member_serialize<T, Archive>::value; // this
        auto i6 = cereal::traits::has_member_save_minimal<T, Archive>::value;
        auto i7 = cereal::traits::has_non_member_save_minimal<T, Archive>::value;
        auto i = i1 + i2 + i3 + i4 + i5 + i6 + i7;
        REQUIRE(i == 1);
    }
}

/*
TEST_CASE("checking serialization methods for types", "[serialize]")
{
    using Archive = cereal::BinaryOutputArchive;
    checkSerialization<Archive, cereal::NameValuePair<entt::any>>();
    checkSerialization<Archive, cereal::MapItem<entt::any,entt::any>>();
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
    ReflectionUtils::bind();
    TestSerializeStruct::bindMeta();
    TestSerializeStruct v;
    v.value = 3.14F;

    auto any = entt::forward_as_meta(v);

    auto data = saveToData(any);
    v.value = 0;
    loadFromData(data, any);

    REQUIRE(v.value == 3.14F);
}

TEST_CASE("serialize unordered_set", "[serialize]")
{
    std::unordered_set<int> set{ 42, 666 };
    auto any = entt::forward_as_meta(set);

    auto data = saveToData(any);
    set.clear();
    loadFromData(data, any);

    REQUIRE(set.size() == 2);
    REQUIRE(*set.begin() == 42);
}
*/

TEST_CASE("serialize map", "[serialize]")
{
    std::map<std::string, std::string> map
    {
        {"key1", "test1" },
        {"key2", "test2" },
    };
    auto any = entt::forward_as_meta(map);

    std::stringstream ss;
    {
        cereal::JSONOutputArchive archive(ss);
        save(archive, any);
    }

    map.clear();

    auto str = ss.str();
    ss.flush();
    ss.seekg(0, std::ios::beg);
    auto json = nlohmann::json::parse(ss);
    REQUIRE(json.is_array());
    REQUIRE(json.size() == 2);
    REQUIRE(json[0]["key"] == "key1");
    REQUIRE(json[0]["value"] == "test1");
    REQUIRE(json[1]["key"] == "key2");
    REQUIRE(json[1]["value"] == "test2");

    {
        ss.seekg(0, std::ios::beg);
        cereal::JSONInputArchive archive(ss);
        load(archive, any);
    }

    REQUIRE(map.size() == 2);
    REQUIRE(map["key1"] == "test1");
    REQUIRE(map["key2"] == "test2");
}