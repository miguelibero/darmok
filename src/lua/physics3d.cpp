#include "physics3d.hpp"
#include "scene.hpp"
#include <darmok/physics3d.hpp>


namespace darmok::physics3d
{
    LuaPhysicsUpdater::LuaPhysicsUpdater(const sol::protected_function& func) noexcept
        : _func(func)
    {
    }

    void LuaPhysicsUpdater::fixedUpdate(float fixedDeltaTime)
    {
        _func(fixedDeltaTime);
    }

    bool LuaPhysicsUpdater::operator==(const sol::protected_function& func) noexcept
    {
        return _func == func;
    }

    LuaCollisionListener::LuaCollisionListener(const sol::object& listener) noexcept
    {
    }

    bool LuaCollisionListener::operator==(const sol::object& listener) noexcept
    {
        return _listener == listener;
    }

    void LuaCollisionListener::onCollisionEnter(RigidBody& rigidBody1, RigidBody& rigidBody2, const Collision& collision)
    {
    }

    void LuaCollisionListener::onCollisionStay(RigidBody& rigidBody1, RigidBody& rigidBody2, const Collision& collision)
    {

    }

    void LuaCollisionListener::onCollisionExit(RigidBody& rigidBody1, RigidBody& rigidBody2)
    {

    }

    LuaPhysicsSystem::LuaPhysicsSystem(PhysicsSystem& system) noexcept
        : _system(system)
    {
    }

    LuaPhysicsSystem& LuaPhysicsSystem::registerUpdate(const sol::protected_function& func) noexcept
    {
        _updaters.emplace_back(func);
        return *this;
    }

    bool LuaPhysicsSystem::unregisterUpdate(const sol::protected_function& func) noexcept
    {
        auto itr = std::find(_updaters.begin(), _updaters.end(), func);
        if (itr == _updaters.end())
        {
            return false;
        }
        _updaters.erase(itr);
        return true;
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addListener(const sol::object& listener) noexcept
    {
        _listeners.emplace_back(listener);
        return *this;
    }

    bool LuaPhysicsSystem::removeListener(const sol::object& listener) noexcept
    {
        auto itr = std::find(_listeners.begin(), _listeners.end(), listener);
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr);
        return true;
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast1(const Ray& ray) noexcept
    {
        return _system->raycast(ray);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast2(const Ray& ray, float maxDistance) noexcept
    {
        return _system->raycast(ray, maxDistance);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _system->raycast(ray, maxDistance, layerMask);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll1(const Ray& ray) noexcept
    {
        return _system->raycastAll(ray);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll2(const Ray& ray, float maxDistance) noexcept
    {
        return _system->raycastAll(ray, maxDistance);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _system->raycastAll(ray, maxDistance, layerMask);
    }

    LuaPhysicsSystem LuaPhysicsSystem::addSceneComponent1(LuaScene& scene) noexcept
    {
        return LuaPhysicsSystem(scene.getReal()->addComponent<PhysicsSystem>());
    }

    LuaPhysicsSystem LuaPhysicsSystem::addSceneComponent2(LuaScene& scene, const Config& config) noexcept
    {
        return LuaPhysicsSystem(scene.getReal()->addComponent<PhysicsSystem>(config));
    }

    LuaPhysicsSystem LuaPhysicsSystem::addSceneComponent3(LuaScene& scene, const Config& config, bx::AllocatorI& alloc) noexcept
    {
        return LuaPhysicsSystem(scene.getReal()->addComponent<PhysicsSystem>(config, alloc));
    }

    void LuaPhysicsSystem::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaPhysicsSystem>("Physics3dSystem", sol::no_constructor,
            "register_update", &LuaPhysicsSystem::registerUpdate,
            "unregister_update", &LuaPhysicsSystem::unregisterUpdate,
            "add_listener", &LuaPhysicsSystem::addListener,
            "remove_listener", &LuaPhysicsSystem::removeListener,
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
            "add_scene_component", sol::overload(
                &LuaPhysicsSystem::addSceneComponent1,
                &LuaPhysicsSystem::addSceneComponent2 /*,
                &LuaPhysicsSystem::addSceneComponent3*/
            )
        );
    }

    LuaRigidBody::LuaRigidBody(RigidBody& rigidBody) noexcept
        : _rigidBody(rigidBody)
    {
    }

    const LuaRigidBody::Shape& LuaRigidBody::getShape() const noexcept
    {
        return _rigidBody->getShape();
    }

    LuaRigidBody::MotionType LuaRigidBody::getMotionType() const noexcept
    {
        return _rigidBody->getMotionType();
    }

    LuaRigidBody& LuaRigidBody::setPosition(const VarLuaTable<glm::vec3>& pos) noexcept
    {
        _rigidBody->setPosition(LuaGlm::tableGet(pos));
        return *this;
    }

    glm::vec3 LuaRigidBody::getPosition() const noexcept
    {
        return _rigidBody->getPosition();
    }

    LuaRigidBody& LuaRigidBody::setRotation(const VarLuaTable<glm::quat>& rot) noexcept
    {
        _rigidBody->setRotation(LuaGlm::tableGet(rot));
        return *this;
    }

    glm::quat LuaRigidBody::getRotation() const noexcept
    {
        return _rigidBody->getRotation();
    }

    LuaRigidBody& LuaRigidBody::setLinearVelocity(const VarLuaTable<glm::vec3>& velocity) noexcept
    {
        _rigidBody->setLinearVelocity(LuaGlm::tableGet(velocity));
        return *this;
    }

    glm::vec3 LuaRigidBody::getLinearVelocity() const noexcept
    {
        return _rigidBody->getLinearVelocity();
    }

    LuaRigidBody& LuaRigidBody::addTorque(const VarLuaTable<glm::vec3>& torque) noexcept
    {
        _rigidBody->addTorque(LuaGlm::tableGet(torque));
        return *this;
    }

    LuaRigidBody& LuaRigidBody::addForce(const VarLuaTable<glm::vec3>& force) noexcept
    {
        _rigidBody->addForce(LuaGlm::tableGet(force));
        return *this;
    }

    LuaRigidBody& LuaRigidBody::addImpulse(const VarLuaTable<glm::vec3>& impulse) noexcept
    {
        _rigidBody->addImpulse(LuaGlm::tableGet(impulse));
        return *this;
    }

    LuaRigidBody& LuaRigidBody::move1(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot) noexcept
    {
        _rigidBody->move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot));
        return *this;
    }

    LuaRigidBody& LuaRigidBody::move2(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot, float deltaTime) noexcept
    {
        _rigidBody->move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot), deltaTime);
        return *this;
    }


    LuaRigidBody& LuaRigidBody::movePosition1(const VarLuaTable<glm::vec3>& pos) noexcept
    {
        _rigidBody->movePosition(LuaGlm::tableGet(pos));
        return *this;
    }

    LuaRigidBody& LuaRigidBody::movePosition2(const VarLuaTable<glm::vec3>& pos, float deltaTime) noexcept
    {
        _rigidBody->movePosition(LuaGlm::tableGet(pos), deltaTime);
        return *this;
    }

    LuaRigidBody& LuaRigidBody::addListener(const sol::object& listener) noexcept
    {
        return *this;
    }

    bool LuaRigidBody::removeListener(const sol::object& listener) noexcept
    {
        auto itr = std::find(_listeners.begin(), _listeners.end(), listener);
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr);
        return true;
    }

    LuaRigidBody LuaRigidBody::addEntityComponent1(LuaEntity& entity, const Shape& shape) noexcept
    {
        return entity.addComponent<RigidBody>(shape);
    }

    LuaRigidBody LuaRigidBody::addEntityComponent2(LuaEntity& entity, const Shape& shape, MotionType motion) noexcept
    {
        return entity.addComponent<RigidBody>(shape, motion);
    }

    LuaRigidBody LuaRigidBody::addEntityComponent3(LuaEntity& entity, const Config& config) noexcept
    {
        return entity.addComponent<RigidBody>(config);
    }

    std::optional<LuaRigidBody> LuaRigidBody::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<RigidBody, LuaRigidBody>();
    }

    std::optional<LuaEntity> LuaRigidBody::getEntity(LuaScene& scene) noexcept
    {
        return scene.getEntity(_rigidBody.value());
    }
    
    void LuaRigidBody::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaRigidBody>("RigidBody3d", sol::no_constructor,
            "type_id", &entt::type_hash<RigidBody>::value,
            "add_entity_component", sol::overload(
                &LuaRigidBody::addEntityComponent1,
                &LuaRigidBody::addEntityComponent2,
                &LuaRigidBody::addEntityComponent3
            ),
            "get_entity_component", &LuaRigidBody::getEntityComponent,
            "get_entity", &LuaRigidBody::getEntity,
            "add_listener", &LuaRigidBody::addListener,
            "remove_listener", &LuaRigidBody::removeListener,
            "shape", sol::property(&LuaRigidBody::getShape),
            "motion_type", sol::property(&LuaRigidBody::getMotionType),
            "position", sol::property(&LuaRigidBody::getPosition, &LuaRigidBody::setPosition),
            "rotation", sol::property(&LuaRigidBody::getRotation, &LuaRigidBody::setRotation),
            "linear_velocity", sol::property(&LuaRigidBody::getLinearVelocity, &LuaRigidBody::setLinearVelocity)
        );
    }
}