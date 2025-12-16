#include <catch2/catch_test_macros.hpp>
#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/transform.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/asset_pack.hpp>
#include <nlohmann/json.hpp>

#include "protobuf/scene_serialize_test.pb.h"

namespace
{
    using namespace darmok;
    using namespace entt::literals;

    template<class T>
    using unexpected = tl::unexpected<T>;

    struct TestComponent
    {
        int value;

		using Definition = protobuf::TestComponent;

        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt) noexcept
        {
			value = def.value();
            return {};
        }
    };

    struct TestRefComponent
    {
        OptionalRef<TestComponent> comp;

        using Definition = protobuf::TestRefComponent;

        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt) noexcept
        {
            auto entity = ctxt.getEntity(def.comp());
            if (entity == entt::null)
            {
				return unexpected{ "Referenced entity not found" };
            }
            comp = ctxt.getScene().getComponent<TestComponent>(entity);
            return {};
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

        using Definition = protobuf::TestStructComponent;

        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt) noexcept
        {
            value.value = def.value().value();
			value.str = def.value().str();
            return {};
        }
    };

    Data saveToData(const Scene::Definition& scene)
    {
        Data data;
        DataOutputStream out{ data };
        auto result = protobuf::write(scene, out, protobuf::Format::Json);
        if (!result)
        {
			throw std::exception{ result.error().c_str() };
        }
        return data.view(0, out.tellp());
    }

    void loadFromData(const Data& data, Scene::Definition& scene)
    {
        DataInputStream in{ data };
        auto result = protobuf::read(scene, in, protobuf::Format::Json);
        if (!result)
        {
            throw std::exception{ result.error().c_str() };
        }
    }

    std::string saveToString(const Scene::Definition& scene)
    {
        std::ostringstream out;
        auto result = protobuf::write(scene, out, protobuf::Format::Json);
        if (!result)
        {
            throw std::exception{ result.error().c_str() };
        }
        return out.str();
    }

    void loadFromString(const std::string& str, Scene::Definition& scene)
    {
        std::istringstream in{ str };
        auto result = protobuf::read(scene, in, protobuf::Format::Json);
        if (!result)
        {
            throw std::exception{ result.error().c_str() };
        }
    }

    Scene::Definition createTestScene()
    {
        Scene::Definition sceneDef;
        SceneDefinitionWrapper scene{ sceneDef };

        auto entity = scene.createEntity();
        TestComponent::Definition comp;
        comp.set_value(42);
        scene.setComponent(entity, comp);
        entity = scene.createEntity();
        comp.set_value(666);
        scene.setComponent(entity, comp);

        return sceneDef; 
    }
}

TEST_CASE("scene can be serialized", "[scene-serialize]")
{
	auto sceneDef = createTestScene();
    auto str = saveToString(sceneDef);
    auto json = nlohmann::json::parse(str);

    REQUIRE(json.is_object());
    REQUIRE(json.size() == 1);
    auto& reg = json["registry"];
    REQUIRE(reg["entities"] == 2);
    auto typeId = std::to_string(protobuf::getTypeId<TestComponent::Definition>());
    auto& comps = reg["components"][typeId]["components"];
    REQUIRE(comps.size() == 2);
    REQUIRE(comps["1"]["value"] == 42);
    REQUIRE(comps["2"]["value"] == 666);
}

TEST_CASE("scene can be loaded", "[scene-serialize]")
{
    auto sceneDef = createTestScene();
    auto data = saveToData(sceneDef);
    sceneDef = {};
    loadFromData(data, sceneDef);

    Scene scene;
    SceneLoader sceneLoader;
    sceneLoader.registerComponent<TestComponent>();
    auto result = sceneLoader(sceneDef, scene);

    auto view = scene.getEntities();
    std::vector<Entity> entities{ view.begin(), view.end() };

    Entity entity = sceneLoader.getComponentLoadContext().getEntity(2);
    REQUIRE(result);

    REQUIRE(entities.size() == 2);
    REQUIRE(scene.getComponent<TestComponent>(entity)->value == 666);
}

TEST_CASE("component structs are serialized", "[scene-serialize]")
{
    Scene::Definition sceneDef;

    EntityId entityId;
    {
        SceneDefinitionWrapper sceneWrap{ sceneDef };
        entityId = sceneWrap.createEntity();
        TestStructComponent::Definition comp;
        comp.mutable_value()->set_value(42);
        comp.mutable_value()->set_str("lala");
        sceneWrap.setComponent(entityId, comp);
        entityId = sceneWrap.createEntity();
        comp.mutable_value()->set_value(666);
        comp.mutable_value()->set_str("lolo");
        sceneWrap.setComponent(entityId, comp);
    }

    Scene scene;
    SceneLoader sceneLoader;
    sceneLoader.registerComponent<TestStructComponent>();
    auto result = sceneLoader(sceneDef, scene);
    REQUIRE(result);

    auto view = scene.getEntities();
    std::vector<Entity> entities(view.begin(), view.end());

    REQUIRE(entities.size() == 2);
    auto entity = sceneLoader.getComponentLoadContext().getEntity(entityId);
    auto comp = scene.getComponent<TestStructComponent>(entity);
    REQUIRE(comp->value.value == 666);
    REQUIRE(comp->value.str == "lolo");
}

TEST_CASE("component references are serialized", "[scene-serialize]")
{
    Scene::Definition sceneDef;
    EntityId entityId;
    {
        SceneDefinitionWrapper sceneWrap{ sceneDef };
        entityId = sceneWrap.createEntity();
        TestComponent::Definition comp1;
        comp1.set_value(42);
        sceneWrap.setComponent(entityId, comp1);
        TestRefComponent::Definition comp2;
        comp2.set_comp(entityId);
        sceneWrap.setComponent(entityId, comp2);
    }

    Scene scene;
    SceneLoader sceneLoader;
    sceneLoader.registerComponent<TestComponent>();
    sceneLoader.registerComponent<TestRefComponent>();
    auto result = sceneLoader(sceneDef, scene);
    REQUIRE(result);

    auto entity = sceneLoader.getComponentLoadContext().getEntity(entityId);
    auto refComp = scene.getComponent<TestRefComponent>(entity);
    REQUIRE(refComp->comp->value == 42);
}

TEST_CASE("transform hierarchy is serialized", "[scene-serialize]")
{
    Scene::Definition sceneDef;
    EntityId parentEntityId;
    EntityId child2EntityId;
    uint32_t entityId;
    {
        SceneDefinitionWrapper sceneWrap{ sceneDef };
        parentEntityId = sceneWrap.createEntity();
        auto parent = Transform::createDefinition();
        sceneWrap.setComponent(parentEntityId, parent);
        auto child1EntityId = sceneWrap.createEntity();
        auto child1 = Transform::createDefinition();
        child1.set_parent(parentEntityId);
        sceneWrap.setComponent(child1EntityId, child1);
        child2EntityId = sceneWrap.createEntity();
        auto child2 = Transform::createDefinition();
        child2.set_parent(parentEntityId);
        *child2.mutable_position() = convert<protobuf::Vec3>(glm::vec3{ 42, 0, 666 });
        sceneWrap.setComponent(child2EntityId, child2);
    }

    Scene scene;
    SceneLoader sceneLoader;
    auto result = sceneLoader(sceneDef, scene);
    REQUIRE(result);

    auto child2Entity = sceneLoader.getComponentLoadContext().getEntity(child2EntityId);
    auto parentEntity = sceneLoader.getComponentLoadContext().getEntity(parentEntityId);
    auto child2 = scene.getComponent<Transform>(child2Entity);
    REQUIRE(child2->getPosition() == glm::vec3(42, 0, 666));

    auto parent = child2->getParent();
    REQUIRE(scene.getEntity(*parent) == parentEntity);
    REQUIRE(parent->getChildren().size() == 2);
}