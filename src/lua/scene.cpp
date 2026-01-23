#include "lua/scene.hpp"
#include "lua/transform.hpp"
#include "lua/camera.hpp"
#include "lua/light.hpp"
#include "lua/render_scene.hpp"
#include "lua/scene_filter.hpp"
#include "lua/scene_serialize.hpp"
#include "lua/freelook.hpp"
#include <darmok/scene.hpp>
#include <darmok/app.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/string.hpp>
#include <darmok/transform.hpp>

#ifdef DARMOK_OZZ
#include "skeleton.hpp"
#endif

#ifdef DARMOK_JOLT
#include "lua/physics3d.hpp"
#include "lua/physics3d_character.hpp"
#endif

namespace darmok
{
	LuaEntityComponent::LuaEntityComponent(const sol::table& table) noexcept
		: _table{ table }
	{
	}

	entt::id_type LuaEntityComponent::getType() const noexcept
	{
		return LuaUtils::getTypeId(_table).value();
	}

	sol::object LuaEntityComponent::getReal() const noexcept
	{
		return _table;
	}

	LuaEntity::LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene)
		: _entity{ entity }
		, _scene{ scene }
	{
		if (_entity == entt::null)
		{
			throw sol::error{ "cannot create null entity" };
		}
	}

	std::string LuaEntity::toString() const noexcept
	{
		std::string str{ "Entity(" + std::to_string(_entity) };
		if (!isValid())
		{
			str += " invalid";
		}
		return str + ")";
	}

	bool LuaEntity::isValid() const noexcept
	{
		if(_entity == entt::null)
		{
			return false;
		}
		if (auto scene = _scene.lock())
		{
			return scene->isValidEntity(_entity);
		}
		return false;
	}

	std::shared_ptr<Scene> LuaEntity::getScene() const
	{
		if (auto scene = _scene.lock())
		{
			return scene;
		}
		throw sol::error{ "scene expired" };
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

	const Entity& LuaEntity::getReal() const noexcept
	{
		return _entity;
	}

	bool LuaEntity::removeComponent(const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto scene = _scene.lock())
			{
				return scene->removeComponent(_entity, typeId.value());
			}
		}
		return false;
	}

	bool LuaEntity::hasComponent(const sol::object& type) const noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto scene = _scene.lock())
			{
				return scene->hasComponent(_entity, typeId.value());
			}
		}
		return false;
	}

	void LuaEntity::addLuaComponent(const sol::table& comp)
	{
		auto typeId = LuaUtils::getTypeId(comp).value();
		getScene()->addCustomComponent<LuaEntityComponent>(_entity, typeId, comp);
	}

	sol::object LuaEntity::getLuaComponent(const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto scene = _scene.lock())
			{
				auto comp = scene->getCustomComponent<LuaEntityComponent>(_entity, typeId.value());
				if (comp)
				{
					return comp->getReal();
				}
			}
		}
		return sol::nil;
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
			return LuaUtils::checkResult<bool>(result, "for each entity child");
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
			return LuaUtils::checkResult<bool>(result, "for each entity parent");
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

	bool LuaScene::removeSceneComponent(Scene& scene, const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return LuaUtils::unwrapExpected(scene.removeSceneComponent(typeId.value()));
		}
		return false;
	}

	bool LuaScene::hasSceneComponent(const Scene& scene, const sol::object& type)
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			return scene.hasSceneComponent(typeId.value());
		}
		return false;
	}

	void LuaScene::addLuaSceneComponent(const std::shared_ptr<Scene>& scene, const sol::table& table)
	{
		auto comp = std::make_unique<LuaSceneComponent>(table, scene);
		LuaUtils::unwrapExpected(scene->addSceneComponent(std::move(comp)), "adding scene component");
	}

	sol::object LuaScene::getLuaSceneComponent(Scene& scene, const sol::object& type) noexcept
	{
		if (auto typeId = LuaUtils::getTypeId(type))
		{
			if (auto comp = scene.getSceneComponent(typeId.value()))
			{
				auto& luaComp = static_cast<LuaSceneComponent&>(*comp);
				return luaComp.getReal();
			}
		}
		return sol::nil;
	}

	LuaEntity LuaScene::createEntity1(const std::shared_ptr<Scene>& scene) noexcept
	{
		return { scene->createEntity(), scene };
	}

	LuaEntity LuaScene::createEntity2(const std::shared_ptr<Scene>& scene, LuaEntity& parent) noexcept
	{
		auto entity = scene->createEntity();
		auto& parentTrans = scene->getOrAddComponent<Transform>(parent.getReal());
		scene->addComponent<Transform>(entity).setParent(parentTrans);
		return { entity, scene };
	}

	LuaEntity LuaScene::createEntity3(const std::shared_ptr<Scene>& scene, Transform& parent) noexcept
	{
		auto entity = scene->createEntity();
		scene->addComponent<Transform>(entity).setParent(parent);
		return { entity, scene };
	}

	LuaEntity LuaScene::createEntity4(const std::shared_ptr<Scene>& scene, LuaEntity& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = scene->createEntity();
		auto& parentTrans = scene->getOrAddComponent<Transform>(parent.getReal());
		auto& trans = scene->addComponent<Transform>(entity, LuaGlm::tableGet(position));
		trans.setParent(parentTrans);
		return { entity, scene };
	}

	LuaEntity LuaScene::createEntity5(const std::shared_ptr<Scene>& scene, Transform& parent, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = scene->createEntity();
		auto& trans = scene->addComponent<Transform>(entity, LuaGlm::tableGet(position));
		trans.setParent(parent);
		return { entity, scene };
	}

	LuaEntity LuaScene::createEntity6(const std::shared_ptr<Scene>& scene, const VarLuaTable<glm::vec3>& position) noexcept
	{
		auto entity = scene->createEntity();
		scene->addComponent<Transform>(entity, LuaGlm::tableGet(position));
		return { entity, scene };
	}

	void LuaScene::destroyEntity(Scene& scene, const LuaEntity& entity) noexcept
	{
		scene.destroyEntity(entity.getReal());
	}

	std::optional<LuaEntity> LuaScene::getEntity(const std::shared_ptr<Scene>& scene, const sol::object& comp) noexcept
	{
		// TODO: check why this does not work
		if (comp.get_type() != sol::type::userdata)
		{
			return std::nullopt;
		}
		auto typeId = LuaUtils::getTypeId(comp);
		if (!typeId)
		{
			return std::nullopt;
		}
		auto entity = scene->getEntity(typeId.value(), comp.as<void*>());
		if (entity == entt::null)
		{
			return std::nullopt;
		}
		return LuaEntity{ entity, scene };
	}

	std::optional<Viewport> LuaScene::getViewport(const Scene& scene) noexcept
	{
		return scene.getViewport();
	}

	void LuaScene::setViewport(Scene& scene, std::optional<VarViewport> vp) noexcept
	{
		if (vp)
		{
			scene.setViewport(LuaViewport::tableGet(vp));
		}
		else
		{
			scene.setViewport(sol::nullopt);
		}
	}

	void LuaScene::setUpdateFilter(Scene& scene, const sol::object& filter) noexcept
	{
		scene.setUpdateFilter(LuaEntityFilter::create(filter));
	}

	bool LuaScene::forEachEntity(const std::shared_ptr<Scene>& scene, const sol::protected_function& callback)
	{
		return scene->forEachEntity([callback, scene](auto entity) -> bool {
			auto result = callback(LuaEntity(entity, scene));
			return LuaUtils::checkResult<bool>(result, "for each entity");
		});
	}

	void LuaScene::bind(sol::state_view& lua) noexcept
	{
		LuaEntityFilter::bind(lua);
		LuaSceneAppComponent::bind(lua);
		LuaSceneLoader::bind(lua);
		LuaSceneDefinition::bind(lua);
		LuaTransform::bind(lua);
		LuaCamera::bind(lua);
		LuaEntity::bind(lua);
		LuaRenderable::bind(lua);
		LuaFreelookController::bind(lua);

#ifdef DARMOK_OZZ
		LuaSkeletalAnimator::bind(lua);
		LuaRenderableSkeleton::bind(lua);
		LuaSkinnable::bind(lua);
		LuaSkeletalAnimationSceneComponent::bind(lua);
#endif

#ifdef DARMOK_JOLT
		physics3d::LuaPhysicsSystem::bind(lua);
		physics3d::LuaPhysicsBody::bind(lua);
		physics3d::LuaCharacterController::bind(lua);
#endif

		lua.new_usertype<Scene>("Scene", sol::no_constructor,
			"create_entity", sol::overload(
				&LuaScene::createEntity1, &LuaScene::createEntity2,
				&LuaScene::createEntity3, &LuaScene::createEntity4,
				&LuaScene::createEntity5, &LuaScene::createEntity6),
			"destroy_entity", &LuaScene::destroyEntity,
			"get_entity", sol::resolve<std::optional<LuaEntity>(const std::shared_ptr<Scene>&, const sol::object&)>(&LuaScene::getEntity),
			sol::meta_function::to_string, &Scene::toString,
			"has_component", &LuaScene::hasSceneComponent,
			"remove_component", &LuaScene::removeSceneComponent,
			"add_lua_component", &LuaScene::addLuaSceneComponent,
			"get_lua_component", &LuaScene::getLuaSceneComponent,
			"for_each_entity", &LuaScene::forEachEntity,
			"viewport", sol::property(&LuaScene::getViewport, &LuaScene::setViewport),
			"current_viewport", sol::property(&Scene::getCurrentViewport),
			"render_chain", sol::property(sol::resolve<RenderChain&()>(&Scene::getRenderChain)),
			"name", sol::property(&Scene::getName, &Scene::setName),
			"paused", sol::property(&Scene::isPaused, &Scene::setPaused),
			"update_filter", sol::property(&Scene::getUpdateFilter, &LuaScene::setUpdateFilter)
		);
	}

	LuaSceneComponent::LuaSceneComponent(const sol::table& table, const std::weak_ptr<Scene>& scene) noexcept
		: _table(table)
		, _scene(scene)
	{
	}

	sol::object LuaSceneComponent::getReal() const noexcept
	{
		return _table;
	}

	const LuaTableDelegateDefinition LuaSceneComponent::_initDef{ "init", "scene component init" };
	const LuaTableDelegateDefinition LuaSceneComponent::_shutdownDef{ "shutdown", "scene component shutdown" };
	const LuaTableDelegateDefinition LuaSceneComponent::_renderResetDef{ "render_reset", "scene component render reset" };
	const LuaTableDelegateDefinition LuaSceneComponent::_updateDef{ "update", "scene component update" };

	expected<void, std::string> LuaSceneComponent::init(Scene& scene, App& app) noexcept
	{
		if (auto scene = _scene.lock())
		{
			return _initDef.tryRun(_table, scene);
		}
		else
		{
			return unexpected<std::string>{ "scene expired in lua scene component init" };
		}
	}

	expected<void, std::string> LuaSceneComponent::shutdown() noexcept
	{
		return _shutdownDef.tryRun(_table);
	}

	expected<bgfx::ViewId, std::string> LuaSceneComponent::renderReset(bgfx::ViewId viewId) noexcept
	{
		if (!_renderResetDef.valid(_table))
		{
			return viewId;
		}
		return _renderResetDef.tryGet<bgfx::ViewId>(_table, viewId);
	}

	expected<void, std::string> LuaSceneComponent::update(float deltaTime) noexcept
	{
		return _updateDef.tryRun(_table, deltaTime);
	}

	std::reference_wrapper<SceneAppComponent> LuaSceneAppComponent::addAppComponent1(App& app)
	{
		return LuaUtils::unwrapExpected(app.addComponent<SceneAppComponent>());
	}

	std::reference_wrapper<SceneAppComponent> LuaSceneAppComponent::addAppComponent2(App& app, const std::shared_ptr<Scene>& scene)
	{
		return LuaUtils::unwrapExpected(app.addComponent<SceneAppComponent>(scene));
	}

	OptionalRef<SceneAppComponent>::std_t LuaSceneAppComponent::getAppComponent(App& app) noexcept
	{
		return app.getComponent<SceneAppComponent>();
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
			"scene", sol::property(&SceneAppComponent::getScene, &SceneAppComponent::setScene),
			"paused", sol::property(&SceneAppComponent::isPaused, &SceneAppComponent::setPaused)
		);
	}
}