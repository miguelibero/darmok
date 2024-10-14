#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/physics3d.hpp>
#include <optional>
#include <vector>
#include "glm.hpp"
#include "utils.hpp"

namespace darmok
{
    class LuaEntity;
    class LuaScene;
    class Transform;
    class Scene;
    struct Ray;
}

namespace darmok::physics3d
{
    class PhysicsSystem;
    class PhysicsBody;
    struct RaycastHit;

    class LuaCollisionListener final : public ICollisionListener
    {
    public:
        LuaCollisionListener(const sol::table& table) noexcept;

        bool operator==(const LuaCollisionListener& other) const noexcept;
        bool operator!=(const LuaCollisionListener& other) const noexcept;
        entt::id_type getType() const noexcept override;

        void onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionExit(PhysicsBody& body1, PhysicsBody& body2) override;

    private:
        sol::main_table _table;
        static const LuaTableDelegateDefinition _enterDef;
        static const LuaTableDelegateDefinition _stayDef;
        static const LuaTableDelegateDefinition _exitDef;
    };

    class LuaPhysicsUpdater final : public IPhysicsUpdater
    {
    public:
        LuaPhysicsUpdater(const sol::object& obj) noexcept;

        bool operator==(const LuaPhysicsUpdater& other) const noexcept;
        bool operator!=(const LuaPhysicsUpdater& other) const noexcept;
        entt::id_type getType() const noexcept override;

        void fixedUpdate(float fixedDeltaTime) override;
    private:
        LuaDelegate _delegate;
    };

    class LuaPhysicsSystem final
    {
    public:
        using Config = PhysicsSystemConfig;

        static void bind(sol::state_view& lua) noexcept;
    private:

        static PhysicsSystem& addUpdater(PhysicsSystem& system, const sol::object& obj) noexcept;
        static bool removeUpdater(PhysicsSystem& system, const sol::object& obj) noexcept;

        static PhysicsSystem& addListener(PhysicsSystem& system, const sol::table& table) noexcept;
        static bool removeListener(PhysicsSystem& system, const sol::table& table) noexcept;

        static OptionalRef<Transform>::std_t getRootTransform(PhysicsSystem& system) noexcept;
        static void setRootTransform(PhysicsSystem& system, OptionalRef<Transform>::std_t root) noexcept;

        static std::optional<RaycastHit> raycast1(const PhysicsSystem& system, const Ray& ray) noexcept;
        static std::optional<RaycastHit> raycast2(const PhysicsSystem& system, const Ray& ray, float maxDistance) noexcept;
        static std::optional<RaycastHit> raycast3(const PhysicsSystem& system, const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;
        static std::vector<RaycastHit> raycastAll1(const PhysicsSystem& system, const Ray& ray) noexcept;
        static std::vector<RaycastHit> raycastAll2(const PhysicsSystem& system, const Ray& ray, float maxDistance) noexcept;
        static std::vector<RaycastHit> raycastAll3(const PhysicsSystem& system, const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;

        static PhysicsSystem& addSceneComponent1(LuaScene& scene) noexcept;
        static PhysicsSystem& addSceneComponent2(LuaScene& scene, const Config& config) noexcept;
        static PhysicsSystem& addSceneComponent3(LuaScene& scene, const Config& config, bx::AllocatorI& alloc) noexcept;
        static OptionalRef<PhysicsSystem>::std_t getSceneComponent(LuaScene& scene) noexcept;
    };

    class PhysicsBody;
    struct PhysicsBodyConfig;
    struct CharacterConfig;

    class LuaPhysicsBody final
    {
    public:
        using MotionType = PhysicsBodyMotionType;
        using Shape = PhysicsShape;
        using Config = PhysicsBodyConfig;
        
        static void bind(sol::state_view& lua) noexcept;
    private:

        static void setPosition(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos);
        static void setRotation(PhysicsBody& body, const VarLuaTable<glm::quat>& rot);
        static void setLinearVelocity(PhysicsBody& body, const VarLuaTable<glm::vec3>& velocity);

        static PhysicsBody& addTorque(PhysicsBody& body, const VarLuaTable<glm::vec3>& torque) noexcept;
        static PhysicsBody& addForce(PhysicsBody& body, const VarLuaTable<glm::vec3>& force) noexcept;
        static PhysicsBody& addImpulse(PhysicsBody& body, const VarLuaTable<glm::vec3>& impulse) noexcept;
        static PhysicsBody& move1(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot) noexcept;
        static PhysicsBody& move2(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot, float deltaTime) noexcept;
        static PhysicsBody& movePosition1(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos) noexcept;
        static PhysicsBody& movePosition2(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos, float deltaTime) noexcept;

        static PhysicsBody& addListener(PhysicsBody& body, const sol::table& table) noexcept;
        static bool removeListener(PhysicsBody& body, const sol::table& table) noexcept;

        static PhysicsBody& addEntityComponent1(LuaEntity& entity, const Shape& shape) noexcept;
        static PhysicsBody& addEntityComponent2(LuaEntity& entity, const Shape& shape, MotionType motion) noexcept;
        static PhysicsBody& addEntityComponent3(LuaEntity& entity, const Shape& shape, MotionType motion, bool trigger) noexcept;
        static PhysicsBody& addEntityComponent4(LuaEntity& entity, const Config& config) noexcept;
        static PhysicsBody& addEntityComponent5(LuaEntity& entity, const CharacterConfig& config) noexcept;
        static OptionalRef<PhysicsBody>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        static std::optional<LuaEntity> getEntity(const PhysicsBody& body, LuaScene& scene) noexcept;
    };
}