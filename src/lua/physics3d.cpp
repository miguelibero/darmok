#include "physics3d.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/physics3d.hpp>


namespace darmok::physics3d
{
   
    LuaPhysicsSystem::LuaPhysicsSystem(PhysicsSystem& system) noexcept
        : _system(system)
    {
        system.addListener(*this);
        system.addUpdater(*this);
    }

    LuaPhysicsSystem& LuaPhysicsSystem::registerUpdate(const sol::protected_function& func) noexcept
    {
        _updates.emplace_back(func);
        return *this;
    }

    bool LuaPhysicsSystem::unregisterUpdate(const sol::protected_function& func) noexcept
    {
        auto itr = std::find(_updates.begin(), _updates.end(), func);
        if (itr == _updates.end())
        {
            return false;
        }
        _updates.erase(itr);
        return true;
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addListener(const sol::table& listener) noexcept
    {
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
        return true;
    }

    void LuaPhysicsSystem::fixedUpdate(float fixedDeltaTime)
    {
        for (auto& update : _updates)
        {
            if (!update)
            {
                continue;
            }
            auto result = update(fixedDeltaTime);
            if (!result.valid())
            {
                recoveredLuaError("running fixed update", result);
            }
        }
    }

    template<typename Callback>
    static void callLuaCollisionListeners(const std::vector<sol::table>& listeners, PhysicsBody& body1, PhysicsBody& body2, const std::string& key, const std::string& desc, Callback callback)
    {
        LuaPhysicsBody luaBody1(body1);
        LuaPhysicsBody luaBody2(body2);
        for (auto& listener : listeners)
        {
            sol::protected_function func = listener[key];
            if (!func)
            {
                continue;
            }
            auto result = callback(func, luaBody1, luaBody2);
            if (!result.valid())
            {
                recoveredLuaError(desc, result);
            }
        }
    }

    void LuaPhysicsSystem::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        callLuaCollisionListeners(_listeners, body1, body2, "on_collision_enter", "running physics collision enter",
            [&collision](auto& func, auto& luaBody1, auto& luaBody2)
            {
                return func(luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsSystem::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        callLuaCollisionListeners(_listeners, body1, body2, "on_collision_stay", "running physics collision stay",
            [&collision](auto& func, auto& luaBody1, auto& luaBody2)
            {
                return func(luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsSystem::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        callLuaCollisionListeners(_listeners, body1, body2, "on_collision_exit", "running physics collision exit",
            [](auto& func, auto& luaBody1, auto& luaBody2)
            {
                return func(luaBody1, luaBody2);
            });
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

    LuaPhysicsBody::LuaPhysicsBody(PhysicsBody& body) noexcept
        : _body(body)
    {
    }

    const LuaPhysicsBody::Shape& LuaPhysicsBody::getShape() const noexcept
    {
        return _body->getShape();
    }

    LuaPhysicsBody::MotionType LuaPhysicsBody::getMotionType() const noexcept
    {
        return _body->getMotionType();
    }

    LuaPhysicsBody& LuaPhysicsBody::setPosition(const VarLuaTable<glm::vec3>& pos) noexcept
    {
        _body->setPosition(LuaGlm::tableGet(pos));
        return *this;
    }

    glm::vec3 LuaPhysicsBody::getPosition() const noexcept
    {
        return _body->getPosition();
    }

    LuaPhysicsBody& LuaPhysicsBody::setRotation(const VarLuaTable<glm::quat>& rot) noexcept
    {
        _body->setRotation(LuaGlm::tableGet(rot));
        return *this;
    }

    glm::quat LuaPhysicsBody::getRotation() const noexcept
    {
        return _body->getRotation();
    }

    LuaPhysicsBody& LuaPhysicsBody::setLinearVelocity(const VarLuaTable<glm::vec3>& velocity) noexcept
    {
        _body->setLinearVelocity(LuaGlm::tableGet(velocity));
        return *this;
    }

    glm::vec3 LuaPhysicsBody::getLinearVelocity() const noexcept
    {
        return _body->getLinearVelocity();
    }

    LuaPhysicsBody& LuaPhysicsBody::addTorque(const VarLuaTable<glm::vec3>& torque) noexcept
    {
        _body->addTorque(LuaGlm::tableGet(torque));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::addForce(const VarLuaTable<glm::vec3>& force) noexcept
    {
        _body->addForce(LuaGlm::tableGet(force));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::addImpulse(const VarLuaTable<glm::vec3>& impulse) noexcept
    {
        _body->addImpulse(LuaGlm::tableGet(impulse));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::move1(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot) noexcept
    {
        _body->move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::move2(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot, float deltaTime) noexcept
    {
        _body->move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot), deltaTime);
        return *this;
    }


    LuaPhysicsBody& LuaPhysicsBody::movePosition1(const VarLuaTable<glm::vec3>& pos) noexcept
    {
        _body->movePosition(LuaGlm::tableGet(pos));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::movePosition2(const VarLuaTable<glm::vec3>& pos, float deltaTime) noexcept
    {
        _body->movePosition(LuaGlm::tableGet(pos), deltaTime);
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::addListener(const sol::table& listener) noexcept
    {
        _listeners.emplace_back(listener);
        return *this;
    }

    bool LuaPhysicsBody::removeListener(const sol::table& listener) noexcept
    {
        auto itr = std::find(_listeners.begin(), _listeners.end(), listener);
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr);
        return true;
    }

    void LuaPhysicsBody::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        callLuaCollisionListeners(_listeners, body1, body2, "on_collision_enter", "running physics collision enter",
            [&collision](auto& func, auto& luaBody1, auto& luaBody2)
            {
                return func(luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsBody::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        callLuaCollisionListeners(_listeners, body1, body2, "on_collision_stay", "running physics collision stay",
            [&collision](auto& func, auto& luaBody1, auto& luaBody2)
            {
                return func(luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsBody::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        callLuaCollisionListeners(_listeners, body1, body2, "on_collision_exit", "running physics collision exit",
            [](auto& func, auto& luaBody1, auto& luaBody2)
            {
                return func(luaBody1, luaBody2);
            });
    }

    LuaPhysicsBody LuaPhysicsBody::addEntityComponent1(LuaEntity& entity, const Shape& shape) noexcept
    {
        return entity.addComponent<PhysicsBody>(shape);
    }

    LuaPhysicsBody LuaPhysicsBody::addEntityComponent2(LuaEntity& entity, const Shape& shape, MotionType motion) noexcept
    {
        return entity.addComponent<PhysicsBody>(shape, motion);
    }

    LuaPhysicsBody LuaPhysicsBody::addEntityComponent3(LuaEntity& entity, const Config& config) noexcept
    {
        return entity.addComponent<PhysicsBody>(config);
    }

    std::optional<LuaPhysicsBody> LuaPhysicsBody::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<PhysicsBody, LuaPhysicsBody>();
    }

    std::optional<LuaEntity> LuaPhysicsBody::getEntity(LuaScene& scene) noexcept
    {
        return scene.getEntity(_body.value());
    }
    
    void LuaPhysicsBody::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaPhysicsBody>("PhysicsBody3d", sol::no_constructor,
            "type_id", &entt::type_hash<PhysicsBody>::value,
            "add_entity_component", sol::overload(
                &LuaPhysicsBody::addEntityComponent1,
                &LuaPhysicsBody::addEntityComponent2,
                &LuaPhysicsBody::addEntityComponent3
            ),
            "get_entity_component", &LuaPhysicsBody::getEntityComponent,
            "get_entity", &LuaPhysicsBody::getEntity,
            "add_listener", &LuaPhysicsBody::addListener,
            "remove_listener", &LuaPhysicsBody::removeListener,
            "shape", sol::property(&LuaPhysicsBody::getShape),
            "motion_type", sol::property(&LuaPhysicsBody::getMotionType),
            "position", sol::property(&LuaPhysicsBody::getPosition, &LuaPhysicsBody::setPosition),
            "rotation", sol::property(&LuaPhysicsBody::getRotation, &LuaPhysicsBody::setRotation),
            "linear_velocity", sol::property(&LuaPhysicsBody::getLinearVelocity, &LuaPhysicsBody::setLinearVelocity)
        );
    }
}