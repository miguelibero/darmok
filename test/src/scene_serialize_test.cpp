#include <catch2/catch_test_macros.hpp>
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/transform.hpp>
#include <nlohmann/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>

using namespace darmok;
using namespace entt::literals;

namespace
{
    struct TestComponent
    {
        int value;

        static void bindMeta() noexcept
        {
            ReflectionUtils::metaEntityComponent<TestComponent>("TestComponent")
                .ctor()
                .data<&TestComponent::value, entt::as_ref_t>("value"_hs);
        }
    };

    struct TestRefComponent
    {
        OptionalRef<TestComponent> comp;

        static void bindMeta() noexcept
        {
            ReflectionUtils::metaEntityComponent<TestRefComponent>("TestRefComponent")
                .ctor()
                .data<&TestRefComponent::comp, entt::as_ref_t>("comp"_hs);
        }
    };

    struct TestStruct
    {
        int value;
        std::string str;
    };

    struct TestStructComponent
    {
        TestStruct value;

        static void bindMeta() noexcept
        {
            entt::meta<TestStruct>()
                .ctor()
                .data<&TestStruct::value, entt::as_ref_t>("value"_hs)
                .data<&TestStruct::str, entt::as_ref_t>("str"_hs);
            ReflectionUtils::metaEntityComponent<TestStructComponent>("TestStructComponent")
                .ctor()
                .data<&TestStructComponent::value, entt::as_ref_t>("value"_hs);
        }
    };

    void bindMeta()
    {
        TestComponent::bindMeta();
        TestRefComponent::bindMeta();
        TestStructComponent::bindMeta();
        ReflectionUtils::bind();
    }

    Data saveToData(const Scene& scene)
    {
        Data data;
        DataOutputStream stream(data);
        {
            cereal::BinaryOutputArchive archive(stream);
            save(archive, scene);
        }
        auto pos = stream.tellp();
        return data.view(0, pos);
    }

    void loadFromData(const Data& data, Scene& scene)
    {
        DataInputStream stream(data);
        cereal::BinaryInputArchive archive(stream);
        load(archive, scene);
    }

    template<typename Archive>
    std::string saveToString(const Scene& scene)
    {
        std::stringstream ss;
        {
            Archive archive(ss);
            save(archive, scene);
        }
        return ss.str();
    }

    template<typename Archive>
    void loadFromString(const std::string& str, Scene& scene)
    {
        std::stringstream ss(str);
        Archive archive(ss);
        load(archive, scene);
    }
}

TEST_CASE( "scene can be serialized", "[scene-serialize]" )
{
    Scene scene;
    auto entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 42);
    entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 666);

    TestComponent::bindMeta();

    auto str = saveToString<cereal::JSONOutputArchive>(scene);
    auto json = nlohmann::json::parse(str);

    REQUIRE(json.is_object());
    REQUIRE(json.size() == 5);
    REQUIRE(json["entities"].size() == 2);
    REQUIRE(json["freeList"] == 2);
    REQUIRE(json["entityComponents"].size() == 1);
    REQUIRE(json["entityComponents"][0]["value"].size() == 2);
    REQUIRE(json["entityComponents"][0]["value"][0][0]["value"] == 42);
    REQUIRE(json["entityComponents"][0]["value"][1][0]["value"] == 666);
}

TEST_CASE("scene can be loaded", "[scene-serialize]")
{
    Scene scene;
    auto entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 42);
    entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 666);

    auto data = saveToData(scene);
    scene.destroyEntitiesImmediate();
    loadFromData(data, scene);

    auto view = scene.getEntities();
    std::vector<Entity> entities(view.begin(), view.end());

    REQUIRE(entities.size() == 2);
    REQUIRE(scene.getComponent<TestComponent>(entity)->value == 666);
}


TEST_CASE("component structs are serialized", "[scene-serialize]")
{
    Scene scene;
    auto entity = scene.createEntity();
    scene.addComponent<TestStructComponent>(entity, 42, "lala");
    entity = scene.createEntity();
    scene.addComponent<TestStructComponent>(entity, 666, "lolo");

    TestStructComponent::bindMeta();

    auto data = saveToData(scene);
    scene.destroyEntitiesImmediate();
    loadFromData(data, scene);

    auto view = scene.getEntities();
    std::vector<Entity> entities(view.begin(), view.end());

    REQUIRE(entities.size() == 2);
    auto comp = scene.getComponent<TestStructComponent>(entity);
    REQUIRE(comp->value.value == 666);
    REQUIRE(comp->value.str == "lolo");
}

TEST_CASE("component references are serialized", "[scene-serialize]")
{
    Scene scene;
    auto entity = scene.createEntity();
    auto& comp = scene.addComponent<TestComponent>(entity, 42);
    auto& refComp1 = scene.addComponent<TestRefComponent>(entity, comp);

    TestComponent::bindMeta();
    TestRefComponent::bindMeta();

    auto data = saveToData(scene);
    scene.destroyEntitiesImmediate();
    loadFromData(data, scene);

    auto refComp = scene.getComponent<TestRefComponent>(entity);
    REQUIRE(refComp->comp->value == 42);
}

TEST_CASE("transform hierarchy is serialized", "[scene-serialize]")
{
    Scene scene;

    auto entity = scene.createEntity();
    auto& parent = scene.addComponent<Transform>(entity);
    entity = scene.createEntity();
    auto& child1 = scene.addComponent<Transform>(entity);
    child1.setParent(parent);
    entity = scene.createEntity();
    auto& child2 = scene.addComponent<Transform>(entity);
    child2.setParent(parent);
    child2.setPosition(glm::vec3(42, 0, 666));

    Transform::bindMeta();

    auto data = saveToData(scene);
    scene.destroyEntitiesImmediate();
    loadFromData(data, scene);

    auto newChild2 = scene.getComponent<Transform>(entity);
    REQUIRE(newChild2->getPosition() == glm::vec3(42, 0, 666));

    auto newParent = newChild2->getParent();
    REQUIRE(newParent->getChildren().size() == 2);
}
