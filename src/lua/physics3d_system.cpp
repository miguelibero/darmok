#include "physics3d.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/physics3d.hpp>

namespace darmok::physics3d
{
    LuaCollisionListener::LuaCollisionListener(const sol::table& table) noexcept
        : _table(table)
    {
    }

    entt::id_type LuaCollisionListener::getType() const noexcept
    {
        return entt::type_hash<LuaCollisionListener>::value();
    }

    bool LuaCollisionListener::operator==(const LuaCollisionListener& other) const noexcept
    {
        return _table == other._table;
    }

    bool LuaCollisionListener::operator!=(const LuaCollisionListener& other) const noexcept
    {
        return !operator==(other);
    }

    const LuaTableDelegateDefinition LuaCollisionListener::_enterDef("on_collision_enter", "running physics collision enter");
    const LuaTableDelegateDefinition LuaCollisionListener::_stayDef("on_collision_stay", "running physics collision stay");
    const LuaTableDelegateDefinition LuaCollisionListener::_exitDef("on_collision_exit", "running physics collision exit");

    void LuaCollisionListener::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        _enterDef(_table, body1, body2, collision);
    }

    void LuaCollisionListener::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        _stayDef(_table, body1, body2, collision);
    }

    void LuaCollisionListener::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        _exitDef(_table, body1, body2);
    }

    LuaPhysicsUpdater::LuaPhysicsUpdater(const sol::object& obj) noexcept
        : _delegate(obj, "fixed_update")
    {
    }

    bool LuaPhysicsUpdater::operator==(const LuaPhysicsUpdater& other) const noexcept
    {
        return _delegate == other._delegate;
    }
    bool LuaPhysicsUpdater::operator!=(const LuaPhysicsUpdater& other) const noexcept
    {
        return !operator==(other);
    }

    entt::id_type LuaPhysicsUpdater::getType() const noexcept
    {
        return entt::type_hash<LuaPhysicsUpdater>::value();
    }

    void LuaPhysicsUpdater::fixedUpdate(float fixedDeltaTime)
    {
        _delegate(fixedDeltaTime);
    }

    PhysicsSystem& LuaPhysicsSystem::addUpdater(PhysicsSystem& system, const sol::object& obj) noexcept
    {
        return system.addUpdater(std::make_unique<LuaPhysicsUpdater>(obj));
    }

    bool LuaPhysicsSystem::removeUpdater(PhysicsSystem& system, const sol::object& obj) noexcept
    {
        return system.removeUpdater(LuaPhysicsUpdater(obj));
    }

    PhysicsSystem& LuaPhysicsSystem::addListener(PhysicsSystem& system, const sol::table& table) noexcept
    {
        return system.addListener(std::make_unique<LuaCollisionListener>(table));
    }

    bool LuaPhysicsSystem::removeListener(PhysicsSystem& system, const sol::table& table) noexcept
    {
        return system.removeListener(LuaCollisionListener(table));
    }

    OptionalRef<Transform>::std_t LuaPhysicsSystem::getRootTransform(PhysicsSystem& system) noexcept
    {
        return system.getRootTransform();
    }

    void LuaPhysicsSystem::setRootTransform(PhysicsSystem& system, OptionalRef<Transform>::std_t root) noexcept
    {
        system.setRootTransform(root);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast1(const PhysicsSystem& system, const Ray& ray) noexcept
    {
        return system.raycast(ray);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast2(const PhysicsSystem& system, const Ray& ray, float maxDistance) noexcept
    {
        return system.raycast(ray, maxDistance);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast3(const PhysicsSystem& system, const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return system.raycast(ray, maxDistance, layerMask);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll1(const PhysicsSystem& system, const Ray& ray) noexcept
    {
        return system.raycastAll(ray);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll2(const PhysicsSystem& system, const Ray& ray, float maxDistance) noexcept
    {
        return system.raycastAll(ray, maxDistance);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll3(const PhysicsSystem& system, const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return system.raycastAll(ray, maxDistance, layerMask);
    }

    PhysicsSystem& LuaPhysicsSystem::addSceneComponent1(LuaScene& scene) noexcept
    {
        return scene.getReal()->addSceneComponent<PhysicsSystem>();
    }

    PhysicsSystem& LuaPhysicsSystem::addSceneComponent2(LuaScene& scene, const Config& config) noexcept
    {
        return scene.getReal()->addSceneComponent<PhysicsSystem>(config);
    }

    PhysicsSystem& LuaPhysicsSystem::addSceneComponent3(LuaScene& scene, const Config& config, bx::AllocatorI& alloc) noexcept
    {
        return scene.getReal()->addSceneComponent<PhysicsSystem>(config, alloc);
    }

    OptionalRef<PhysicsSystem>::std_t LuaPhysicsSystem::getSceneComponent(LuaScene& scene) noexcept
    {
        return scene.getReal()->getSceneComponent<PhysicsSystem>();
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

        lua.new_usertype<PhysicsSystem>("Physics3dSystem", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<PhysicsSystem>::value),
            "add_updater", &LuaPhysicsSystem::addUpdater,
            "remove_updater", &LuaPhysicsSystem::removeUpdater,
            "add_listener", &LuaPhysicsSystem::addListener,
            "remove_listener", &LuaPhysicsSystem::removeListener,
            "root_transform", sol::property(&LuaPhysicsSystem::getRootTransform, &LuaPhysicsSystem::setRootTransform),
            "gravity", sol::property(&PhysicsSystem::getGravity),
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