#include <catch2/catch_test_macros.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/adapters.hpp>
#include <nlohmann/json.hpp>

using namespace darmok;

namespace
{
    struct TestComponent
    {
        int i;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(i);
        }
    };

    struct TestRefComponent
    {
        OptionalRef<TestComponent> comp;

        template<class Archive>
        void save(Archive& archive) const
        {
            Entity entity = entt::null;
            if (comp)
            {
                auto& scene = cereal::get_user_data<Scene>(archive);
                entity = scene.getEntity(comp.value());
            }
            archive(entity);
        }

        template<class Archive>
        void load(Archive& archive)
        {
            Entity entity;
            archive(entity);
            if (entity != entt::null)
            {
                auto& scene = cereal::get_user_data<Scene>(archive);
                comp = scene.getComponent<TestComponent>(entity);
            }
        }
    };

    void saveAndLoadSnapshot(Scene& scene)
    {
        Data data;

        {
            DataOutputStream stream(data);
            cereal::UserDataAdapter<Scene, cereal::BinaryOutputArchive> archive(scene, stream);
            scene.createSnapshot()
                .get<TestComponent>(archive)
                .get<TestRefComponent>(archive)
                ;
        }

        scene.destroyEntitiesImmediate();

        {
            DataInputStream stream(data);
            cereal::UserDataAdapter<Scene, cereal::BinaryInputArchive> archive(scene, stream);
            scene.createSnapshotLoader()
                .get<TestComponent>(archive)
                .get<TestRefComponent>(archive)
                ;
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
    auto str = ss.str();
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
    REQUIRE(scene.getComponent<TestComponent>(entity)->i == 666);
}

TEST_CASE("component references are serialized", "[scene-serialize]")
{
    Scene scene;
    auto entity = scene.createEntity();
    auto& comp = scene.addComponent<TestComponent>(entity, 42);
    scene.addComponent<TestRefComponent>(entity, comp);

    saveAndLoadSnapshot(scene);

    REQUIRE(scene.getComponent<TestRefComponent>(entity)->comp->i == 42);
}