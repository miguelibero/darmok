#include "physics3d.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/physics3d.hpp>

namespace darmok::physics3d
{
    LuaPhysicsSystem::LuaPhysicsSystem(PhysicsSystem& system) noexcept
        : _system(system)
    {
        _system.addUpdater(*this);
    }

    void LuaPhysicsSystem::shutdown() noexcept
    {
        _system.removeListener(*this);
        _system.removeUpdater(*this);
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addUpdater(const sol::object& updater) noexcept
    {
        LuaDelegate dlg(updater, "updater");
        if (dlg)
        {
            _updaters.push_back(std::move(dlg));
        }
        return *this;
    }

    bool LuaPhysicsSystem::removeUpdater(const sol::object& updater) noexcept
    {
        auto itr = std::find(_updaters.begin(), _updaters.end(), updater);
        if (itr == _updaters.end())
        {
            return false;
        }
        _updaters.erase(itr);
        return true;
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addListener(const sol::table& listener) noexcept
    {
        if (_listeners.empty())
        {
            _system.addListener(*this);
        }
        _listeners.emplace_back(listener);
        return *this;
    }

    bool LuaPhysicsSystem::removeListener(const sol::table& listener) noexcept
    {
        auto itr = std::find(_listeners.begin(), _listeners.end(), listener);
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr);
        if (_listeners.empty())
        {
            _system.removeListener(*this);
        }
        return true;
    }

    PhysicsSystem& LuaPhysicsSystem::getReal() noexcept
    {
        return _system;
    }

    const PhysicsSystem& LuaPhysicsSystem::getReal() const noexcept
    {
        return _system;
    }

    OptionalRef<Transform>::std_t LuaPhysicsSystem::getRootTransform() const noexcept
    {
        return _system.getRootTransform();
    }

    void LuaPhysicsSystem::setRootTransform(OptionalRef<Transform>::std_t root) noexcept
    {
        _system.setRootTransform(root);
    }

    glm::vec3 LuaPhysicsSystem::getGravity() const
    {
        return _system.getGravity();
    }

    void LuaPhysicsSystem::fixedUpdate(float fixedDeltaTime)
    {
        for (auto& dlg : _updaters)
        {
            auto result = dlg(fixedDeltaTime);
            LuaUtils::checkResult("running fixed function updater", result);
        }
    }

    OptionalRef<LuaPhysicsBody> LuaPhysicsSystem::getLuaBody(PhysicsBody& body) const noexcept
    {
        auto optScene = _system.getScene();
        if (!optScene)
        {
            return nullptr;
        }
        auto& scene = optScene.value();
        auto entity = scene.getEntity(body);
        if (entity == entt::null)
        {
            return nullptr;
        }
        return scene.getComponent<LuaPhysicsBody>(entity);
    }

    const LuaTableDelegateDefinition LuaPhysicsSystem::_collisionEnterDef("on_collision_enter", "running physics collision enter");
    const LuaTableDelegateDefinition LuaPhysicsSystem::_collisionStayDef("on_collision_stay", "running physics collision stay");
    const LuaTableDelegateDefinition LuaPhysicsSystem::_collisionExitDef("on_collision_exit", "running physics collision exit");

    void LuaPhysicsSystem::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        _collisionEnterDef(_listeners, luaBody1, luaBody2, collision);
    }

    void LuaPhysicsSystem::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        _collisionStayDef(_listeners, luaBody1, luaBody2, collision);
    }

    void LuaPhysicsSystem::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        _collisionExitDef(_listeners, luaBody1, luaBody2);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast1(const Ray& ray) noexcept
    {
        return _system.raycast(ray);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast2(const Ray& ray, float maxDistance) noexcept
    {
        return _system.raycast(ray, maxDistance);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _system.raycast(ray, maxDistance, layerMask);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll1(const Ray& ray) noexcept
    {
        return _system.raycastAll(ray);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll2(const Ray& ray, float maxDistance) noexcept
    {
        return _system.raycastAll(ray, maxDistance);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _system.raycastAll(ray, maxDistance, layerMask);
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addSceneComponent1(LuaScene& scene) noexcept
    {
        return scene.addSceneComponent<LuaPhysicsSystem, PhysicsSystem>();
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addSceneComponent2(LuaScene& scene, const Config& config) noexcept
    {
        return scene.addSceneComponent<LuaPhysicsSystem, PhysicsSystem>(config);
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addSceneComponent3(LuaScene& scene, const Config& config, bx::AllocatorI& alloc) noexcept
    {
        return scene.addSceneComponent<LuaPhysicsSystem, PhysicsSystem>(config, alloc);
    }

    OptionalRef<LuaPhysicsSystem>::std_t LuaPhysicsSystem::getSceneComponent(LuaScene& scene) noexcept
    {
        return scene.getReal()->getSceneComponent<LuaPhysicsSystem>();
    }

    void LuaPhysicsSystem::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Config>("Physics3dSystemConfig",
            sol::constructors<Config()>(),
            "maxBodies", &Config::maxBodies,
            "numBodyMutexes", &Config::numBodyMutexes,
            "maxBodyPairs", &Config::maxBodyPairs,
            "maxContactConstraints", &Config::maxContactConstraints,
            "fixedDeltaTime", &Config::fixedDeltaTime,
            "collisionSteps", &Config::collisionSteps,
            "gravity", &Config::gravity
        );

        lua.new_usertype<LuaPhysicsSystem>("Physics3dSystem", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<LuaPhysicsSystem>::value),
            "add_updater", &LuaPhysicsSystem::addUpdater,
            "remove_updater", &LuaPhysicsSystem::removeUpdater,
            "add_listener", &LuaPhysicsSystem::addListener,
            "remove_listener", &LuaPhysicsSystem::removeListener,
            "root_transform", sol::property(&LuaPhysicsSystem::getRootTransform, &LuaPhysicsSystem::setRootTransform),
            "gravity", sol::property(&LuaPhysicsSystem::getGravity),
            "raycast", sol::overload(
                &LuaPhysicsSystem::raycast1,
                &LuaPhysicsSystem::raycast2,
                &LuaPhysicsSystem::raycast3
            ),
            "raycast_all", sol::overload(
                &LuaPhysicsSystem::raycastAll1,
                &LuaPhysicsSystem::raycastAll2,
                &LuaPhysicsSystem::raycastAll3
            ),
            "get_scene_component", &LuaPhysicsSystem::getSceneComponent,
            "add_scene_component", sol::overload(
                &LuaPhysicsSystem::addSceneComponent1,
                &LuaPhysicsSystem::addSceneComponent2 /*,
                &LuaPhysicsSystem::addSceneComponent3*/
            )
        );
    }
}