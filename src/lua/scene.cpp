#include "scene.hpp"
#include "app.hpp"
#include "transform.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "render.hpp"
#include "freelook.hpp"
#include "component.hpp"
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
	LuaEntity::LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
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
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return getRegistry().storage(typeId.value())->remove(_entity);
		}
		return removeLuaComponent(type);
	}

	bool LuaEntity::hasComponent(const sol::object& type) const
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			auto storage = getRegistry().storage(typeId.value());
			return storage->find(_entity) != storage->end();
		}
		return hasLuaComponent(type);
	}

	bool LuaEntity::tryGetLuaComponentContainer() const noexcept
	{
		if (_luaComponents)
		{
			return true;
		}
		if (auto scene = _scene.lock())
		{
			_luaComponents = scene->getComponent<LuaComponentContainer>(_entity);
		}
		return !_luaComponents.empty();
	}

	bool LuaEntity::hasLuaComponent(const sol::object& type) const noexcept
	{
		tryGetLuaComponentContainer();
		return _luaComponents && _luaComponents->contains(type);
	}

	void LuaEntity::addLuaComponent(const sol::table& comp)
	{
		if (!_luaComponents)
		{
			if (auto scene = _scene.lock())
			{
				_luaComponents = scene->getOrAddComponent<LuaComponentContainer>(_entity);
			}
			else
			{
				throw std::runtime_error("scene expired");
			}
		}
		_luaComponents->add(comp);
	}

	bool LuaEntity::removeLuaComponent(const sol::object& type) noexcept
	{
		tryGetLuaComponentContainer();
		return _luaComponents && _luaComponents->remove(type);
	}

	sol::object LuaEntity::getLuaComponent(const sol::object& type) noexcept
	{
		tryGetLuaComponentContainer();
		if (!_luaComponents)
		{
			return sol::nil;
		}
		return _luaComponents->get(type);
	}

	bool LuaEntity::checkForEachResult(const std::string& desc, const sol::protected_function_result& result)
	{
		LuaUtils::checkResult(desc, result);
		sol::object obj = result;
		auto type = obj.get_type();
		if (type == sol::type::boolean)
		{
			return obj.as<bool>();
		}
		return type != sol::type::nil;
	}

	bool LuaEntity::forEachChild(const sol::protected_function& callback)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return false;
		}
		return scene->forEachChild(_entity, [callback, scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntity(entity, scene), trans);
			return checkForEachResult("for each entity child", result);
		});
	}

	bool LuaEntity::forEachParent(const sol::protected_function& callback)
	{
		auto scene = _scene.lock();
		if (!scene)
		{
			return false;
		}
		return scene->forEachParent(_entity, [callback, scene](auto entity, auto& trans) -> bool {
			auto result = callback(LuaEntity(entity, scene), trans);
			return checkForEachResult("for each entity parent", result);
		});
	}

	void LuaEntity::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEntity>("Entity", sol::no_constructor,
			"scene", sol::property(&LuaEntity::getScene),
			"remove_component", &LuaEntity::removeComponent,
			"has_component", &LuaEntity::hasComponent,
			"add_lua_component", &LuaEntity::addLuaComponent,
			"get_lua_component", &LuaEntity::getLuaComponent,
			"for_each_child", &LuaEntity::forEachChild,
			"for_each_parent", &LuaEntity::forEachParent,
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

	bool LuaScene::removeSceneComponent(const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return _scene->removeSceneComponent(typeId.value());
		}
		return removeLuaSceneComponent(type);
	}

	bool LuaScene::hasSceneComponent(const sol::object& type) const
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return _scene->hasSceneComponent(typeId.value());
		}
		return hasLuaSceneComponent(type);
	}

	bool LuaScene::tryGetLuaComponentContainer() const noexcept
	{
		if (_luaComponents)
		{
			return true;
		}
		_luaComponents = _scene->getSceneComponent<LuaSceneComponentContainer>();
		return !_luaComponents.empty();
	}

	bool LuaScene::hasLuaSceneComponent(const sol::object& type) const noexcept
	{
		tryGetLuaComponentContainer();
		return _luaComponents && _luaComponents->contains(type);
	}

	void LuaScene::addLuaSceneComponent(const sol::table& comp)
	{
		if (!_luaComponents)
		{
			_luaComponents = _scene->addSceneComponent<LuaSceneComponentContainer>(*this);
		}
		_luaComponents->add(comp);
	}

	bool LuaScene::removeLuaSceneComponent(const sol::object& type) noexcept
	{
		tryGetLuaComponentContainer();
		return _luaComponents && _luaComponents->remove(type);
	}

	sol::object LuaScene::getLuaSceneComponent(const sol::object& type) noexcept
	{
		tryGetLuaComponentContainer();
		if (!_luaComponents)
		{
			return sol::nil;
		}
		return _luaComponents->get(type);
	}

	LuaEntity LuaScene::createEntity1() noexcept
	{
		return LuaEntity(getRegistry().create(), _scene);
	}

	LuaEntity LuaScene::createEntity2(LuaEntity& parent) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		registry.emplace<Transform>(entity)
			.setParent(registry.get_or_emplace<Transform>(parent.getReal()));
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity3(Transform& parent) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		registry.emplace<Transform>(entity)
			.setParent(parent);
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity4(LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		auto& parentTrans = registry.get_or_emplace<Transform>(parent.getReal());
		registry.emplace<Transform>(entity, parentTrans, LuaGlm::tableGet(position));
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity5(Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		registry.emplace<Transform>(entity, parent, LuaGlm::tableGet(position));
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity6(const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto& registry = getRegistry();
		auto entity = registry.create();
		registry.emplace<Transform>(entity, LuaGlm::tableGet(position));
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
			"create_entity", sol::overload(
				&LuaScene::createEntity1, &LuaScene::createEntity2,
				&LuaScene::createEntity3, &LuaScene::createEntity4,
				&LuaScene::createEntity5, &LuaScene::createEntity6),
			"destroy_entity", &LuaScene::destroyEntity,
			sol::meta_function::to_string, &LuaScene::toString,
			"has_component", &LuaScene::hasSceneComponent,
			"remove_component", &LuaScene::removeSceneComponent,
			"add_lua_component", &LuaScene::addLuaSceneComponent,
			"get_lua_component", &LuaScene::getLuaSceneComponent
		);
	}

	LuaSceneAppComponent::LuaSceneAppComponent(SceneAppComponent& comp) noexcept
		: _comp(comp)
	{
	}

	LuaSceneAppComponent& LuaSceneAppComponent::addAppComponent1(LuaApp& app) noexcept
	{
		return app.addComponent<LuaSceneAppComponent, SceneAppComponent>();
	}

	LuaSceneAppComponent& LuaSceneAppComponent::addAppComponent2(LuaApp& app, const LuaScene& scene) noexcept
	{
		return app.addComponent<LuaSceneAppComponent, SceneAppComponent>(scene.getReal());
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