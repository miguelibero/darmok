#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/physics3d.hpp>
#include <optional>
#include <vector>
#include "glm.hpp"

namespace darmok
{
    class LuaEntity;
    class LuaScene;
    struct Ray;
}

namespace darmok::physics3d
{
    class PhysicsSystem;
    class RigidBody;
    struct RaycastHit;

    class LuaPhysicsUpdater : public IPhysicsUpdater
    {
    public:
        LuaPhysicsUpdater(const sol::protected_function& func) noexcept;
        void fixedUpdate(float fixedDeltaTime) override;
        bool operator==(const sol::protected_function& func) noexcept;
    private:
        sol::protected_function _func;
    };

    class LuaCollisionListener : public ICollisionListener
    {
    public:
        LuaCollisionListener(const sol::object& listener) noexcept;
        bool operator==(const sol::object& listener) noexcept;
        void onCollisionEnter(RigidBody& rigidBody1, RigidBody& rigidBody2, const Collision& collision) override;
        void onCollisionStay(RigidBody& rigidBody1, RigidBody& rigidBody2, const Collision& collision) override;
        void onCollisionExit(RigidBody& rigidBody1, RigidBody& rigidBody2) override;
    private:
        sol::object _listener;
    };

    class LuaPhysicsSystem final
    {
    public:
        using Config = PhysicsSystemConfig;

        LuaPhysicsSystem(PhysicsSystem& system) noexcept;
        LuaPhysicsSystem& registerUpdate(const sol::protected_function& func) noexcept;
        bool unregisterUpdate(const sol::protected_function& func) noexcept;
        LuaPhysicsSystem& addListener(const sol::object& listener) noexcept;
        bool removeListener(const sol::object& listener) noexcept;

        std::optional<RaycastHit> raycast1(const Ray& ray) noexcept;
        std::optional<RaycastHit> raycast2(const Ray& ray, float maxDistance) noexcept;
        std::optional<RaycastHit> raycast3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;
        std::vector<RaycastHit> raycastAll1(const Ray& ray) noexcept;
        std::vector<RaycastHit> raycastAll2(const Ray& ray, float maxDistance) noexcept;
        std::vector<RaycastHit> raycastAll3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;

        static LuaPhysicsSystem addSceneComponent1(LuaScene& scene) noexcept;
        static LuaPhysicsSystem addSceneComponent2(LuaScene& scene, const Config& config) noexcept;
        static LuaPhysicsSystem addSceneComponent3(LuaScene& scene, const Config& config, bx::AllocatorI& alloc) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<PhysicsSystem> _system;
        std::vector<LuaPhysicsUpdater> _updaters;
        std::vector<LuaCollisionListener> _listeners;
    };

    class RigidBody;

    class LuaRigidBody final
    {
    public:
        using MotionType = RigidBodyMotionType;
        using Shape = PhysicsShape;
        using Config = RigidBodyConfig;

        LuaRigidBody(RigidBody& rigidBody) noexcept;

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;

        LuaRigidBody& setPosition(const VarLuaTable<glm::vec3>& pos) noexcept;
        glm::vec3 getPosition() const noexcept;
        LuaRigidBody& setRotation(const VarLuaTable<glm::quat>& rot) noexcept;
        glm::quat getRotation() const noexcept;
        LuaRigidBody& setLinearVelocity(const VarLuaTable<glm::vec3>& velocity) noexcept;
        glm::vec3 getLinearVelocity() const noexcept;

        LuaRigidBody& addTorque(const VarLuaTable<glm::vec3>& torque) noexcept;
        LuaRigidBody& addForce(const VarLuaTable<glm::vec3>& force) noexcept;
        LuaRigidBody& addImpulse(const VarLuaTable<glm::vec3>& impulse) noexcept;
        LuaRigidBody& move1(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot) noexcept;
        LuaRigidBody& move2(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot, float deltaTime) noexcept;
        LuaRigidBody& movePosition1(const VarLuaTable<glm::vec3>& pos) noexcept;
        LuaRigidBody& movePosition2(const VarLuaTable<glm::vec3>& pos, float deltaTime) noexcept;

        LuaRigidBody& addListener(const sol::object& listener) noexcept;
        bool removeListener(const sol::object& listener) noexcept;
        
        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<RigidBody> _rigidBody;
        std::vector<LuaCollisionListener> _listeners;

        static LuaRigidBody addEntityComponent1(LuaEntity& entity, const Shape& shape) noexcept;
        static LuaRigidBody addEntityComponent2(LuaEntity& entity, const Shape& shape, MotionType motion) noexcept;
        static LuaRigidBody addEntityComponent3(LuaEntity& entity, const Config& config) noexcept;
        static std::optional<LuaRigidBody> getEntityComponent(LuaEntity& entity) noexcept;
        std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
    };
}