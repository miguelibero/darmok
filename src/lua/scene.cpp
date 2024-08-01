#include "scene.hpp"
#include "app.hpp"
#include "transform.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "render.hpp"
#include "freelook.hpp"
#include <darmok/scene.hpp>

#ifdef DARMOK_OZZ
#include "skeleton.hpp"
#endif

#ifdef DARMOK_JOLT
#include "physics3d.hpp"
#include "character.hpp"
#endif

namespace darmok
{
	bool LuaComponent::removeComponent(const sol::object& type) noexcept
	{
		auto itr = _components.find(getHash(type));
		if (itr == _components.end())
		{
			return false;
		}
		_components.erase(itr);
		return true;
	}

	bool LuaComponent::hasComponent(const sol::object& type) const noexcept
	{
		auto itr = _components.find(getHash(type));
		return itr != _components.end();
	}

	void LuaComponent::addComponent(const sol::table& comp)
	{
		auto metatable = comp[sol::metatable_key];
		auto hash = getHash(metatable);
		auto itr = _components.find(hash);
		if (itr != _components.end())
		{
			throw std::invalid_argument("component of type already exists");
		}
		_components.emplace(hash, comp);
	}

	sol::object LuaComponent::getComponent(const sol::object& type) noexcept
	{
		auto itr = _components.find(getHash(type));
		if (itr != _components.end())
		{
			return itr->second;
		}
		return sol::nil;
	}

	size_t LuaComponent::getHash(const sol::object& obj) noexcept
	{
		return sol::reference_hash()(obj);
	}

	LuaEntity::LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
		if (auto sharedSceme = scene.lock())
		{
			_lua = sharedSceme->getComponent<LuaComponent>(entity);
		}
	}

	std::string LuaEntity::toString() const noexcept
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
		if (type.get_type() == sol::type::number)
		{
			auto typeId = type.template as<entt::id_type>();
			return getRegistry().storage(typeId)->remove(_entity);
		}
		if (type.get_type() == sol::type::table)
		{
			return removeLuaComponent(type);
		}
		return false;
	}

	bool LuaEntity::hasComponent(const sol::object& type) const
	{
		if (type.get_type() == sol::type::number)
		{
			auto typeId = type.template as<entt::id_type>();
			auto storage = getRegistry().storage(typeId);
			return storage->find(_entity) != storage->end();
		}
		if (type.get_type() == sol::type::table)
		{
			return hasLuaComponent(type);
		}
		return false;
	}

	bool LuaEntity::hasLuaComponent(const sol::object& type) const noexcept
	{
		return _lua && _lua->hasComponent(type);
	}

	void LuaEntity::addLuaComponent(const sol::table& comp)
	{
		if (!_lua)
		{
			if (auto scene = _scene.lock())
			{
				_lua = scene->addComponent<LuaComponent>(_entity);
			}
			else
			{
				throw std::runtime_error("scene expired");
			}
		}
		_lua->addComponent(comp);
	}

	bool LuaEntity::removeLuaComponent(const sol::object& type) noexcept
	{
		return _lua && _lua->removeComponent(type);
	}

	sol::object LuaEntity::getLuaComponent(const sol::object& type) noexcept
	{
		if (!_lua)
		{
			return sol::nil;
		}
		return _lua->getComponent(type);
	}

	void LuaEntity::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEntity>("Entity", sol::no_constructor,
			"scene", sol::property(&LuaEntity::getScene),
			"remove_component", &LuaEntity::removeComponent,
			"has_component", &LuaEntity::hasComponent,
			"add_lua_component", &LuaEntity::addLuaComponent,
			"get_lua_component", &LuaEntity::getLuaComponent,
			sol::meta_function::to_string, &LuaEntity::toString
		);
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

	std::string LuaScene::toString() const noexcept
	{
		return "Scene()";
	}

	EntityRegistry& LuaScene::getRegistry() noexcept
	{
		return _scene->getRegistry();
	}

	LuaEntity LuaScene::createEntity1() noexcept
	{
		return LuaEntity(getRegistry().create(), _scene);
	}

	LuaEntity LuaScene::createEntity2(const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		registry.emplace<Transform>(entity, LuaGlm::tableGet(position));
		return LuaEntity(entity, _scene);
	}

	static OptionalRef<Transform> getVarParentTransform(EntityRegistry& registry, LuaScene::VarParent parent) noexcept
	{
		auto trans = std::get_if<Transform>(&parent);
		if (trans != nullptr)
		{
			return trans;
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
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity4(const VarParent& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		auto parentTrans = getVarParentTransform(registry, parent);
		registry.emplace<Transform>(entity, parentTrans, LuaGlm::tableGet(position));
		return LuaEntity(entity, _scene);
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
		LuaSceneAppComponent::bind(lua);
		LuaTransform::bind(lua);
		LuaCamera::bind(lua);
		LuaEntity::bind(lua);
		LuaAmbientLight::bind(lua);
		LuaPointLight::bind(lua);
		LuaRenderable::bind(lua);
		LuaFreelookController::bind(lua);

#ifdef DARMOK_OZZ
		LuaSkeletalAnimator::bind(lua);
		LuaRenderableSkeleton::bind(lua);
		LuaSkeletalAnimationSceneComponent::bind(lua);
#endif

#ifdef DARMOK_JOLT
		physics3d::LuaPhysicsSystem::bind(lua);
		physics3d::LuaPhysicsBody::bind(lua);
		physics3d::LuaCharacterController::bind(lua);
#endif

		lua.new_usertype<LuaScene>("Scene",
			sol::constructors<LuaScene(LuaApp&)>(),
			"create_entity",	sol::overload(
				&LuaScene::createEntity1, &LuaScene::createEntity2,
				&LuaScene::createEntity3, &LuaScene::createEntity4),
			"destroy_entity",	&LuaScene::destroyEntity,
			sol::meta_function::to_string, &LuaScene::toString
		);
	}

	LuaSceneAppComponent::LuaSceneAppComponent(SceneAppComponent& comp) noexcept
		: _comp(comp)
	{
	}

	LuaSceneAppComponent LuaSceneAppComponent::addAppComponent1(LuaApp& app) noexcept
	{
		return LuaSceneAppComponent(app.getReal().addComponent<SceneAppComponent>());
	}

	LuaSceneAppComponent LuaSceneAppComponent::addAppComponent2(LuaApp& app, const LuaScene& scene) noexcept
	{
		return LuaSceneAppComponent(app.getReal().addComponent<SceneAppComponent>(scene.getReal()));
	}

	LuaScene LuaSceneAppComponent::getScene() const noexcept
	{
		return LuaScene(_comp.get().getScene());
	}

	LuaSceneAppComponent& LuaSceneAppComponent::setScene(const LuaScene& scene) noexcept
	{
		_comp.get().setScene(scene.getReal());
		return *this;
	}

	void LuaSceneAppComponent::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaSceneAppComponent>("SceneAppComponent",
			sol::no_constructor,
			"add_app_component", sol::overload(
				&LuaSceneAppComponent::addAppComponent1,
				&LuaSceneAppComponent::addAppComponent2
			),
			"scene", sol::property(&LuaSceneAppComponent::getScene, &LuaSceneAppComponent::setScene)
		);
	}
}