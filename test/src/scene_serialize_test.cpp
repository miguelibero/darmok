#include <catch2/catch_test_macros.hpp>
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/transform.hpp>
#include <darmok/glm_serialize.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <nlohmann/json.hpp>

using namespace darmok;

namespace
{
    struct TestComponent
    {
        int value;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(value));
        }
    };

    struct TestRefComponent
    {
        OptionalRef<TestComponent> comp;

        template<class Archive>
        void serialize(Archive& archive)
        {
        }

        template<class Archive>
        void lateSerialize(Archive& archive)
        {
            auto scomp = SceneSerializeUtils::createComponentRef(comp);
            archive(cereal::make_nvp("comp", scomp));
        }
    };

    void saveAndLoadSnapshot(Scene& scene)
    {
        Data data;
        App app(nullptr);

        {
            DataOutputStream stream(data);
            SceneOutputArchive<cereal::BinaryOutputArchive> archive(app, scene, stream);
            archive
                .saveComponents<TestComponent>()
                .saveComponents<TestRefComponent>()
                .saveComponents<Transform>()
                .lateSaveComponents<TestRefComponent>()
                .lateSaveComponents<Transform>();
        }

        scene.destroyEntitiesImmediate();

        {
            DataInputStream stream(data);
            SceneInputArchive<cereal::BinaryInputArchive> archive(app, scene, stream);
            archive
                .loadComponents<TestComponent>()
                .loadComponents<TestRefComponent>()
                .loadComponents<Transform>()
                .lateLoadComponents<TestRefComponent>()
                .lateLoadComponents<Transform>();
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

    std::stringstream ss;
    {
        cereal::JSONOutputArchive archive(ss);

        scene.createSnapshot()
            .get<TestComponent>(archive);
    }

    ss.flush();
    ss.seekg(0, std::ios::beg);

    auto json = nlohmann::json::parse(ss);
    REQUIRE(json.is_object());
    REQUIRE(json.size() == 5);
}

TEST_CASE("scene can be loaded", "[scene-serialize]")
{
    Scene scene;
    auto entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 42);
    entity = scene.createEntity();
    scene.addComponent<TestComponent>(entity, 666);

    saveAndLoadSnapshot(scene);

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

    saveAndLoadSnapshot(scene);

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

    saveAndLoadSnapshot(scene);

    auto newChild2 = scene.getComponent<Transform>(entity);
    REQUIRE(newChild2->getPosition() == glm::vec3(42, 0, 666));

    auto newParent = newChild2->getParent();
    REQUIRE(newParent->getChildren().size() == 2);
}