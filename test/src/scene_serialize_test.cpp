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
    
    void saveAndLoadScene(Scene& scene)
    {
        Data data;

        TestComponent::bindMeta();
        TestRefComponent::bindMeta();
        ReflectionUtils::bind();

        std::streamoff pos = 0;
        {
            DataOutputStream stream(data);
            cereal::BinaryOutputArchive archive(stream);
            archive(scene);
            pos = stream.tellp();
        }

        scene.destroyEntitiesImmediate();

        {
            DataInputStream stream(data.view(0, pos));
            cereal::BinaryInputArchive archive(stream);
            archive(scene);
        }
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
    std::stringstream ss;
    {
        cereal::JSONOutputArchive archive(ss);
        archive(scene);
    }

    auto str = ss.str();
    ss.flush();
    ss.seekg(0, std::ios::beg);

    auto json = nlohmann::json::parse(ss).front();
    REQUIRE(json.is_object());
    REQUIRE(json.size() == 11);
}

TEST_CASE("scene can be loaded", "[scene-serialize]")
{
    Scene scene;
    auto entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 42);
    entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 666);

    saveAndLoadScene(scene);

    auto view = scene.getEntities();
    std::vector<Entity> entities(view.begin(), view.end());

    REQUIRE(entities.size() == 2);
    REQUIRE(scene.getComponent<TestComponent>(entity)->value == 666);
}

TEST_CASE("component references are serialized", "[scene-serialize]")
{
    Scene scene;
    auto entity = scene.createEntity();
    auto& comp = scene.addComponent<TestComponent>(entity, 42);
    scene.addComponent<TestRefComponent>(entity, comp);

    saveAndLoadScene(scene);

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

    saveAndLoadScene(scene);

    auto newChild2 = scene.getComponent<Transform>(entity);
    REQUIRE(newChild2->getPosition() == glm::vec3(42, 0, 666));

    auto newParent = newChild2->getParent();
    REQUIRE(newParent->getChildren().size() == 2);
}