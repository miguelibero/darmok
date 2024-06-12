#include "scene.hpp"
#include "app.hpp"
#include "transform.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "render.hpp"
#include "skeleton.hpp"
#include <darmok/scene.hpp>

namespace darmok
{
	LuaEntity::LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
	}

	std::string LuaEntity::to_string() const noexcept
	{
		std::string str = "Entity(" + std::to_string(_entity);
		if (!isValid())
		{
			str += " invalid";
		}
		return str + ")";
	}

	bool LuaEntity::isValid() const noexcept
	{
		if (auto scene = _scene.lock())
		{
			return scene->getRegistry().valid(_entity);
		}
		return false;
	}

	LuaScene LuaEntity::getScene() const
	{
		if (auto scene = _scene.lock())
		{
			return LuaScene(scene);
		}
		throw std::runtime_error("scene expired");
	}

	EntityRegistry& LuaEntity::getRegistry()
	{
		if (auto scene = _scene.lock())
		{
			return scene->getRegistry();
		}
		throw std::runtime_error("scene expired");
	}

	const EntityRegistry& LuaEntity::getRegistry() const
	{
		if (auto scene = _scene.lock())
		{
			return scene->getRegistry();
		}
		throw std::runtime_error("scene expired");
	}

	const Entity& LuaEntity::getReal() const noexcept
	{
		return _entity;
	}

	bool LuaEntity::removeComponent(const sol::object& type)
	{
		auto typeId = getComponentTypeId(type);
		if (!typeId)
		{
			return false;
		}
		return getRegistry().storage(typeId.value())->remove(_entity);
	}

	bool LuaEntity::hasComponent(const sol::object& type) const
	{
		auto typeId = getComponentTypeId(type);
		if (!typeId)
		{
			return false;
		}
		auto storage = getRegistry().storage(typeId.value());
		return storage->find(_entity) != storage->end();
	}

	std::optional<entt::id_type> LuaEntity::getComponentTypeId(const sol::object& obj) noexcept
	{
		switch (obj.get_type())
		{
		case sol::type::number:
			return obj.template as<entt::id_type>();
		case sol::type::table:
			sol::table tab = obj;
			auto f = tab["__type_id"].get<sol::function>();
			if (f.valid())
			{
				return f().get<entt::id_type>();
			}
			break;
		}
		return std::nullopt;
	}

	void LuaEntity::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEntity>("Entity", sol::no_constructor,
			"scene", sol::property(&LuaEntity::getScene),
			"remove_component", &LuaEntity::removeComponent,
			"has_component", &LuaEntity::hasComponent
		);

		lua.script(R"(
function Entity:add_component(type, ...)
	return type.add_entity_component(self, ...)
end
function Entity:get_component(type)
	return type.get_entity_component(self)
end
function Entity:get_or_add_component(type, ...)
	local comp = self:get_component(type)
	if comp ~= nil then
		return comp
	end
	return self:add_component(type, ...)
end
)");
	}

	LuaScene::LuaScene(const std::shared_ptr<Scene>& scene) noexcept
		: _scene(scene == nullptr ? std::make_shared<Scene>() : scene)
	{
	}

	LuaScene::LuaScene(LuaApp& app) noexcept
		: _scene(std::make_shared<Scene>())
	{
		_scene->init(app.getReal());
	}

	std::string LuaScene::to_string() const noexcept
	{
		return "Scene()";
	}

	EntityRegistry& LuaScene::getRegistry() noexcept
	{
		return _scene->getRegistry();
	}

	LuaEntity LuaScene::createEntity1() noexcept
	{
		return LuaEntity(getRegistry().create(), std::weak_ptr<Scene>(_scene));
	}

	LuaEntity LuaScene::createEntity2(const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		registry.emplace<Transform>(entity, LuaGlm::tableGet(position));
		return LuaEntity(entity, std::weak_ptr<Scene>(_scene));
	}

	static OptionalRef<Transform> getVarParentTransform(EntityRegistry& registry, LuaScene::VarParent parent) noexcept
	{
		auto luaTrans = std::get_if<LuaTransform>(&parent);
		if (luaTrans != nullptr)
		{
			return luaTrans->getReal();
		}
		auto parentEntity = std::get<Entity>(parent);
		if (parentEntity == entt::null)
		{
			return nullptr;
		}
		return registry.get_or_emplace<Transform>(parentEntity);
	}

	LuaEntity LuaScene::createEntity3(const VarParent& parent) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		auto parentTrans = getVarParentTransform(registry, parent);
		registry.emplace<Transform>(entity, parentTrans);
		return LuaEntity(entity, std::weak_ptr<Scene>(_scene));
	}

	LuaEntity LuaScene::createEntity4(const VarParent& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		auto parentTrans = getVarParentTransform(registry, parent);
		registry.emplace<Transform>(entity, parentTrans, LuaGlm::tableGet(position));
		return LuaEntity(entity, std::weak_ptr<Scene>(_scene));
	}

	bool LuaScene::destroyEntity(const LuaEntity& entity) noexcept
	{
		return getRegistry().destroy(entity.getReal()) != 0;
	}

	const std::shared_ptr<Scene>& LuaScene::getReal() const noexcept
	{
		return _scene;
	}

	std::shared_ptr<Scene>& LuaScene::getReal() noexcept
	{
		return _scene;
	}

	void LuaScene::bind(sol::state_view& lua) noexcept
	{
		LuaTransform::bind(lua);
		LuaCamera::bind(lua);
		LuaEntity::bind(lua);
		LuaAmbientLight::bind(lua);
		LuaPointLight::bind(lua);
		LuaRenderable::bind(lua);
		LuaSkeletalAnimator::bind(lua);
		LuaRenderableSkeleton::bind(lua);

		lua.new_usertype<LuaScene>("Scene",
			sol::constructors<LuaScene(LuaApp&)>(),
			"create_entity",	sol::overload(
				&LuaScene::createEntity1, &LuaScene::createEntity2,
				&LuaScene::createEntity3, &LuaScene::createEntity4),
			"destroy_entity",	&LuaScene::destroyEntity
		);
		lua.script(R"(
function Scene:get_entity(comp)
	return comp:get_entity(self)
end
)");
	}
}