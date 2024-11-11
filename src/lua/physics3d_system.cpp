#include "physics3d.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/physics3d.hpp>
#include <darmok/transform.hpp>

namespace darmok::physics3d
{
    LuaCollisionListener::LuaCollisionListener(const sol::table& table) noexcept
        : _table(table)
    {
    }

    sol::object LuaCollisionListener::getReal() const noexcept
    {
        return _table;
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

    LuaCollisionListenerFilter::LuaCollisionListenerFilter(const sol::table& table) noexcept
        : _table(table)
        , _type(entt::type_hash<LuaCollisionListener>::value())
    {
    }

    bool LuaCollisionListenerFilter::operator()(const ICollisionListener& listener) const noexcept
    {
        return listener.getCollisionListenerType() == _type && static_cast<const LuaCollisionListener&>(listener).getReal() == _table;
    }

    LuaPhysicsUpdater::LuaPhysicsUpdater(const sol::object& obj) noexcept
        : _delegate(obj, "fixed_update")
    {
    }

    const LuaDelegate& LuaPhysicsUpdater::getDelegate() const noexcept
    {
        return _delegate;
    }

    void LuaPhysicsUpdater::fixedUpdate(float fixedDeltaTime)
    {
        static const std::string desc = "running fixed updater";
        LuaUtils::checkResult(desc, _delegate(fixedDeltaTime));
    }

    LuaPhysicsUpdaterFilter::LuaPhysicsUpdaterFilter(const sol::object& obj) noexcept
        : _object(obj)
        , _type(entt::type_hash<LuaPhysicsUpdater>::value())
    {
    }

    bool LuaPhysicsUpdaterFilter::operator()(const IPhysicsUpdater& updater) const noexcept
    {
        return updater.getPhysicsUpdaterType() == _type && static_cast<const LuaPhysicsUpdater&>(updater).getDelegate() == _object;
    }

    PhysicsSystem& LuaPhysicsSystem::addUpdater(PhysicsSystem& system, const sol::object& obj) noexcept
    {
        auto updater = std::make_unique<LuaPhysicsUpdater>(obj);
        if (updater->getDelegate())
        {
            return system.addUpdater(std::move(updater));
        }
        return system;
    }

    bool LuaPhysicsSystem::removeUpdater(PhysicsSystem& system, const sol::object& obj) noexcept
    {
        return system.removeUpdaters(LuaPhysicsUpdaterFilter(obj)) > 0;
    }

    PhysicsSystem& LuaPhysicsSystem::addListener(PhysicsSystem& system, const sol::table& table) noexcept
    {
        return system.addListener(std::make_unique<LuaCollisionListener>(table));
    }

    bool LuaPhysicsSystem::removeListener(PhysicsSystem& system, const sol::table& table) noexcept
    {
        return system.removeListeners(LuaCollisionListenerFilter(table)) > 0;
    }

    OptionalRef<Transform>::std_t LuaPhysicsSystem::getRootTransform(PhysicsSystem& system) noexcept
    {
        return system.getRootTransform();
    }

    void LuaPhysicsSystem::setRootTransform(PhysicsSystem& system, OptionalRef<Transform>::std_t root) noexcept
    {
        system.setRootTransform(root);
    }

    bool LuaPhysicsSystem::isValidEntity(PhysicsSystem& system, LuaEntity& entity) noexcept
    {
        return system.isValidEntity(entity.getReal());
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast1(const PhysicsSystem& system, const Ray& ray) noexcept
    {
        return system.raycast(ray);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast2(const PhysicsSystem& system, const Ray& ray, LayerMask layers) noexcept
    {
        return system.raycast(ray, layers);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll1(const PhysicsSystem& system, const Ray& ray) noexcept
    {
        return system.raycastAll(ray);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll2(const PhysicsSystem& system, const Ray& ray, LayerMask layers) noexcept
    {
        return system.raycastAll(ray, layers);
    }

    void LuaPhysicsSystem::activateBodies1(PhysicsSystem& system, const BoundingBox& bbox) noexcept
    {
        system.activateBodies(bbox);
    }

    void LuaPhysicsSystem::activateBodies2(PhysicsSystem& system, const BoundingBox& bbox, LayerMask layers) noexcept
    {
        system.activateBodies(bbox, layers);
    }

    PhysicsSystem& LuaPhysicsSystem::addSceneComponent1(Scene& scene) noexcept
    {
        return scene.addSceneComponent<PhysicsSystem>();
    }

    PhysicsSystem& LuaPhysicsSystem::addSceneComponent2(Scene& scene, const Config& config) noexcept
    {
        return scene.addSceneComponent<PhysicsSystem>(config);
    }

    PhysicsSystem& LuaPhysicsSystem::addSceneComponent3(Scene& scene, const Config& config, bx::AllocatorI& alloc) noexcept
    {
        return scene.addSceneComponent<PhysicsSystem>(config, alloc);
    }

    OptionalRef<PhysicsSystem>::std_t LuaPhysicsSystem::getSceneComponent(Scene& scene) noexcept
    {
        return scene.getSceneComponent<PhysicsSystem>();
    }

    void LuaPhysicsSystem::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Config>("Physics3dSystemConfig",
            sol::constructors<Config()>(),
            "max_bodies", &Config::maxBodies,
            "num_body_mutexes", &Config::numBodyMutexes,
            "max_body_pairs", &Config::maxBodyPairs,
            "max_contact_constraints", &Config::maxContactConstraints,
            "fixed_delta_time", &Config::fixedDeltaTime,
            "collision_steps", &Config::collisionSteps,
            "gravity", &Config::gravity
        );

        lua.new_usertype<Collision>("Physics3DCollision", sol::no_constructor,
            "normal", &Collision::normal,
            "contacts", &Collision::contacts,
            sol::meta_function::to_string, &Collision::toString
        );

        lua.new_usertype<RaycastHit>("Physics3DRaycastHit", sol::no_constructor,
            "body", &RaycastHit::body,
            "distance", &RaycastHit::distance,
            "factor", &RaycastHit::factor,
            "point", &RaycastHit::point,
            sol::meta_function::to_string, &RaycastHit::toString
        );

        lua.new_usertype<PhysicsSystem>("Physics3dSystem", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<PhysicsSystem>::value),
            "paused", sol::property(&PhysicsSystem::isPaused, &PhysicsSystem::setPaused),
            "is_valid_entity", &LuaPhysicsSystem::isValidEntity,
            "add_updater", &LuaPhysicsSystem::addUpdater,
            "remove_updater", &LuaPhysicsSystem::removeUpdater,
            "add_listener", &LuaPhysicsSystem::addListener,
            "remove_listener", &LuaPhysicsSystem::removeListener,
            "root_transform", sol::property(&LuaPhysicsSystem::getRootTransform, &LuaPhysicsSystem::setRootTransform),
            "gravity", sol::property(&PhysicsSystem::getGravity),
            "raycast", sol::overload(
                &LuaPhysicsSystem::raycast1,
                &LuaPhysicsSystem::raycast2
            ),
            "raycast_all", sol::overload(
                &LuaPhysicsSystem::raycastAll1,
                &LuaPhysicsSystem::raycastAll2
            ),
            "activate_bodies", sol::overload(
                &LuaPhysicsSystem::activateBodies1,
                &LuaPhysicsSystem::activateBodies2
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