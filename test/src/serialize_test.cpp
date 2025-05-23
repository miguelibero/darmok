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
        DataOutputStream stream(data);
        {
            cereal::BinaryOutputArchive archive(stream);
            save(archive, obj);
        }
        auto pos = stream.tellp();
        return data.view(0, pos);
    }

    template<typename T>
    void loadFromData(const Data& data, T& obj)
    {
        DataInputStream stream(data);
        cereal::BinaryInputArchive archive(stream);
        load(archive, obj);
    }

    template<typename Archive, typename T>
    std::string saveToString(const T& obj)
    {
        std::stringstream ss;
        {
            Archive archive(ss);
            save(archive, obj);
        }
        return ss.str();
    }

    template<typename Archive, typename T>
    void loadFromString(const std::string& str, T& obj)
    {
        std::stringstream ss(str);
        Archive archive(ss);
        load(archive, obj);
    }

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
    ReflectionSerializeUtils::bind();
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

TEST_CASE("serialize vector json", "[serialize]")
{
    std::vector<int> vec{ 666, 42 };
    auto any = entt::forward_as_meta(vec);
    auto str = saveToString<cereal::JSONOutputArchive>(any);
    vec.clear();
    loadFromString<cereal::JSONInputArchive>(str, any);

    REQUIRE(vec.size() == 2);
    REQUIRE(vec[0] == 666);
    REQUIRE(vec[1] == 42);

    str = saveToString<cereal::XMLOutputArchive>(any);
    vec.clear();
    loadFromString<cereal::XMLInputArchive>(str, any);

    REQUIRE(vec.size() == 2);
    REQUIRE(vec[0] == 666);
    REQUIRE(vec[1] == 42);
}

TEST_CASE("serialize map", "[serialize]")
{
    std::map<std::string, std::string> map
    {
        {"key1", "test1" },
        {"key2", "test2" },
    };
    auto any = entt::forward_as_meta(map);

    auto str = saveToString<cereal::JSONOutputArchive>(any);

    auto json = nlohmann::json::parse(str);
    REQUIRE(json.is_array());
    REQUIRE(json.size() == 2);
    REQUIRE(json[0]["key"] == "key1");
    REQUIRE(json[0]["value"] == "test1");
    REQUIRE(json[1]["key"] == "key2");
    REQUIRE(json[1]["value"] == "test2");

    map.clear();

    loadFromString<cereal::JSONInputArchive>(str, any);

    REQUIRE(map.size() == 2);
    REQUIRE(map["key1"] == "test1");
    REQUIRE(map["key2"] == "test2");

    
    str = saveToString<cereal::XMLOutputArchive>(any);

    map.clear();

    loadFromString<cereal::XMLInputArchive>(str, any);

    REQUIRE(map.size() == 2);
    REQUIRE(map["key1"] == "test1");
    REQUIRE(map["key2"] == "test2");
}