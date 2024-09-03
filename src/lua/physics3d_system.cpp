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

    LuaPhysicsSystem& LuaPhysicsSystem::addUpdater1(const sol::protected_function& func) noexcept
    {
        if (func)
        {
            _updaterFunctions.emplace_back(func);
        }
        return *this;
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addUpdater2(const sol::table& table) noexcept
    {
        _updaterTables.emplace_back(table);
        return *this;
    }

    bool LuaPhysicsSystem::removeUpdater1(const sol::protected_function& func) noexcept
    {
        auto itr = std::find(_updaterFunctions.begin(), _updaterFunctions.end(), func);
        if (itr == _updaterFunctions.end())
        {
            return false;
        }
        _updaterFunctions.erase(itr);
        return true;
    }

    bool LuaPhysicsSystem::removeUpdater2(const sol::table& table) noexcept
    {
        auto itr = std::find(_updaterTables.begin(), _updaterTables.end(), table);
        if (itr == _updaterTables.end())
        {
            return false;
        }
        _updaterTables.erase(itr);
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

    void LuaPhysicsSystem::fixedUpdate(float fixedDeltaTime)
    {
        for (auto& func : _updaterFunctions)
        {
            auto result = func(fixedDeltaTime);
            LuaUtils::checkResult("running fixed function updater", result);
        }
        LuaUtils::callTableDelegates(_updaterTables, "update", "running fixed table updater",
            [fixedDeltaTime](auto& func, auto& self) {
                return func(self, fixedDeltaTime);
            });
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

    void LuaPhysicsSystem::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableDelegates(_listeners, "on_collision_enter", "running physics collision enter",
            [&collision, &luaBody1, &luaBody2](auto& func, auto& self)
            {
                return func(self, luaBody1, luaBody2, collision);
        });
    }

    void LuaPhysicsSystem::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableDelegates(_listeners, "on_collision_stay", "running physics collision stay",
            [&collision, &luaBody1, &luaBody2](auto& func, auto& self)
            {
                return func(self, luaBody1, luaBody2, collision);
        });
    }

    void LuaPhysicsSystem::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableDelegates(_listeners, "on_collision_exit", "running physics collision exit",
            [&luaBody1, &luaBody2](auto& func, auto& self)
            {
                return func(self, luaBody1, luaBody2);
        });
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
            "add_updater", sol::overload(&LuaPhysicsSystem::addUpdater1, &LuaPhysicsSystem::addUpdater2),
            "remove_updater", sol::overload(&LuaPhysicsSystem::removeUpdater1, &LuaPhysicsSystem::removeUpdater2),
            "add_listener", &LuaPhysicsSystem::addListener,
            "remove_listener", &LuaPhysicsSystem::removeListener,
            "root_transform", sol::property(&LuaPhysicsSystem::getRootTransform, &LuaPhysicsSystem::setRootTransform),
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