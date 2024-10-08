#include "scene.hpp"
#include "app.hpp"
#include "transform.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "render_scene.hpp"
#include "freelook.hpp"
#include <darmok/scene.hpp>
#include <darmok/render_chain.hpp>

#ifdef DARMOK_OZZ
#include "skeleton.hpp"
#endif

#ifdef DARMOK_JOLT
#include "physics3d.hpp"
#include "character.hpp"
#endif

namespace darmok
{
	LuaEntityComponent::LuaEntityComponent(const sol::table& table) noexcept
		: _table(table)
	{
	}

	entt::id_type LuaEntityComponent::getType() const noexcept
	{
		return LuaUtils::getTypeId(_table).value();
	}

	const sol::table& LuaEntityComponent::getReal() const noexcept
	{
		return _table;
	}

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
			return scene->isValidEntity(_entity);
		}
		return false;
	}

	LuaScene LuaEntity::getScene() const
	{
		if (auto scene = _scene.lock())
		{
			return scene;
		}
		throw std::runtime_error("scene expired");
	}

	std::weak_ptr<Scene> LuaEntity::getWeakScene() const noexcept
	{
		return _scene;
	}

	bool LuaEntity::operator==(const LuaEntity& other) const noexcept
	{
		if (_entity != other._entity)
		{
			return false;
		}
		if (auto scene = _scene.lock())
		{
			if (auto otherScene = other._scene.lock())
			{
				return scene == otherScene;
			}
		}
		return false;
	}

	bool LuaEntity::operator!=(const LuaEntity& other) const noexcept
	{
		return !operator==(other);
	}

	Scene& LuaEntity::getRealScene()
	{
		if (auto scene = _scene.lock())
		{
			return *scene;
		}
		throw std::runtime_error("scene expired");
	}

	const Scene& LuaEntity::getRealScene() const
	{
		if (auto scene = _scene.lock())
		{
			return *scene;
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
			return getRealScene().removeComponent(_entity, typeId.value());
		}
		return false;
	}

	bool LuaEntity::hasComponent(const sol::object& type) const
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return getRealScene().hasComponent(_entity, typeId.value());
		}
		return false;
	}

	void LuaEntity::addLuaComponent(const sol::table& comp)
	{
		auto typeId = LuaUtils::getTypeId(comp).value();
		getRealScene().addCustomComponent<LuaEntityComponent>(_entity, typeId, comp);
	}

	sol::object LuaEntity::getLuaComponent(const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			auto comp = getRealScene().getCustomComponent<LuaEntityComponent>(_entity, typeId.value());
			if (comp)
			{
				return comp->getReal();
			}
		}
		return sol::nil;
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
		return _scene->toString();
	}

	bool LuaScene::removeSceneComponent(const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return _scene->removeSceneComponent(typeId.value());
		}
		return false;
	}

	bool LuaScene::hasSceneComponent(const sol::object& type) const
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return _scene->hasSceneComponent(typeId.value());
		}
		return false;
	}

	void LuaScene::addLuaSceneComponent(const sol::table& table)
	{
		auto comp = std::make_unique<LuaSceneComponent>(table, _scene);
		auto typeId = comp->getType();
		_scene->addSceneComponent(typeId, std::move(comp));
	}

	sol::object LuaScene::getLuaSceneComponent(const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto comp = _scene->getSceneComponent(typeId.value()))
			{
				auto& luaComp = static_cast<LuaSceneComponent&>(*comp);
				return luaComp.getReal();
			}
		}
		return sol::nil;
	}

	LuaEntity LuaScene::createEntity1() noexcept
	{
		return LuaEntity(_scene->createEntity(), _scene);
	}

	LuaEntity LuaScene::createEntity2(LuaEntity& parent) noexcept
	{
		auto entity = _scene->createEntity();
		auto& parentTrans = _scene->getOrAddComponent<Transform>(parent.getReal());
		_scene->addComponent<Transform>(entity).setParent(parentTrans);
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity3(Transform& parent) noexcept
	{
		auto entity = _scene->createEntity();
		_scene->addComponent<Transform>(entity).setParent(parent);
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity4(LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = _scene->createEntity();
		auto& parentTrans = _scene->getOrAddComponent<Transform>(parent.getReal());
		_scene->addComponent<Transform>(entity, parentTrans, LuaGlm::tableGet(position));
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity5(Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = _scene->createEntity();
		_scene->addComponent<Transform>(entity, parent, LuaGlm::tableGet(position));
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaScene::createEntity6(const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = _scene->createEntity();
		_scene->addComponent<Transform>(entity, LuaGlm::tableGet(position));
		return LuaEntity(entity, _scene);
	}

	void LuaScene::destroyEntity(const LuaEntity& entity) noexcept
	{
		_scene->destroyEntity(entity.getReal());
	}

	RenderChain& LuaScene::getRenderChain() noexcept
	{
		return _scene->getRenderChain();
	}

	void LuaScene::setName(const std::string& name) noexcept
	{
		_scene->setName(name);
	}

	const std::string& LuaScene::getName() const noexcept
	{
		return _scene->getName();
	}

	const std::shared_ptr<Scene>& LuaScene::getReal() const noexcept
	{
		return _scene;
	}

	std::shared_ptr<Scene>& LuaScene::getReal() noexcept
	{
		return _scene;
	}

	std::optional<Viewport> LuaScene::getViewport() const noexcept
	{
		return _scene->getViewport();
	}

	void LuaScene::setViewport(std::optional<VarViewport> vp) noexcept
	{
		if (vp)
		{
			_scene->setViewport(LuaViewport::tableGet(vp));
		}
		else
		{
			_scene->setViewport(sol::nullopt);
		}
	}

	Viewport LuaScene::getCurrentViewport() noexcept
	{
		return _scene->getCurrentViewport();
	}

	bool LuaScene::forEachEntity(const sol::protected_function& callback)
	{
		return _scene->forEachEntity([callback, this](auto entity) -> bool {
			auto result = callback(LuaEntity(entity, _scene));
			return LuaEntity::checkForEachResult("for each entity", result);
		});
	}

	void LuaScene::bind(sol::state_view& lua) noexcept
	{
		LuaSceneAppComponent::bind(lua);
		LuaTransform::bind(lua);
		LuaCamera::bind(lua);
		LuaEntity::bind(lua);
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
			"get_lua_component", &LuaScene::getLuaSceneComponent,
			"for_each_entity", &LuaScene::forEachEntity,
			"viewport", sol::property(&LuaScene::getViewport, &LuaScene::setViewport),
			"current_viewport", sol::property(&LuaScene::getCurrentViewport),
			"render_chain", sol::property(&LuaScene::getRenderChain),
			"name", sol::property(&LuaScene::getName, &LuaScene::setName)
		);
	}

	LuaSceneComponent::LuaSceneComponent(const sol::table& table, const std::weak_ptr<Scene>& scene) noexcept
		: _table(table)
		, _scene(scene)
	{
	}

	entt::id_type LuaSceneComponent::getType() const noexcept
	{
		return LuaUtils::getTypeId(_table).value();
	}

	const sol::table& LuaSceneComponent::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaSceneComponent::_initDef("init", "scene component init");
	const LuaTableDelegateDefinition LuaSceneComponent::_shutdownDef("shutdown", "scene component shutdown");
	const LuaTableDelegateDefinition LuaSceneComponent::_renderResetDef("render_reset", "scene component render reset");
	const LuaTableDelegateDefinition LuaSceneComponent::_updateDef("update", "scene component update");

	void LuaSceneComponent::init(Scene& scene, App& app)
	{
		if (auto scene = _scene.lock())
		{
			LuaScene luaScene(scene);
			_initDef(_table, luaScene);
		}
	}

	void LuaSceneComponent::shutdown()
	{
		_shutdownDef(_table);
	}

	void LuaSceneComponent::renderReset()
	{
		_renderResetDef(_table);
	}

	void LuaSceneComponent::update(float deltaTime)
	{
		_updateDef(_table, deltaTime);
	}

	SceneAppComponent& LuaSceneAppComponent::addAppComponent1(LuaApp& app) noexcept
	{
		return app.getReal().addComponent<SceneAppComponent>();
	}

	SceneAppComponent& LuaSceneAppComponent::addAppComponent2(LuaApp& app, const LuaScene& scene) noexcept
	{
		return app.getReal().addComponent<SceneAppComponent>(scene.getReal());
	}

	OptionalRef<SceneAppComponent>::std_t LuaSceneAppComponent::getAppComponent(LuaApp& app) noexcept
	{
		return app.getReal().getComponent<SceneAppComponent>();
	}

	std::optional<LuaScene> LuaSceneAppComponent::getScene1(const SceneAppComponent& comp) noexcept
	{
		return getScene2(comp, 0);
	}

	std::optional<LuaScene> LuaSceneAppComponent::getScene2(const SceneAppComponent& comp, size_t i) noexcept
	{
		if (auto scene = comp.getScene(i))
		{
			return LuaScene(scene);
		}
		return std::nullopt;
	}

	SceneAppComponent& LuaSceneAppComponent::setScene1(SceneAppComponent& comp, const LuaScene& scene) noexcept
	{
		return setScene2(comp, scene, 0);
	}

	SceneAppComponent& LuaSceneAppComponent::setScene2(SceneAppComponent& comp, const LuaScene& scene, size_t i) noexcept
	{
		comp.setScene(scene.getReal(), i);
		return comp;
	}

	LuaScene LuaSceneAppComponent::addScene1(SceneAppComponent& comp) noexcept
	{
		return LuaScene(comp.addScene());
	}

	SceneAppComponent& LuaSceneAppComponent::addScene2(SceneAppComponent& comp, LuaScene& scene) noexcept
	{
		comp.addScene(scene.getReal());
		return comp;
	}

	void LuaSceneAppComponent::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<SceneAppComponent>("SceneAppComponent",
			sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<SceneAppComponent>::value),
			"add_app_component", sol::overload(
				&LuaSceneAppComponent::addAppComponent1,
				&LuaSceneAppComponent::addAppComponent2
			),
			"get_app_component", &LuaSceneAppComponent::getAppComponent,
			"scene", sol::property(&LuaSceneAppComponent::getScene1, &LuaSceneAppComponent::setScene1),
			"get_scene", sol::overload(&LuaSceneAppComponent::getScene1, &LuaSceneAppComponent::getScene2),
			"set_scene", sol::overload(&LuaSceneAppComponent::setScene1, &LuaSceneAppComponent::setScene2),
			"add_scene", sol::overload(&LuaSceneAppComponent::addScene1, &LuaSceneAppComponent::addScene2)
		);
	}
}