#pragma once

#include "lua.hpp"
#include "glm.hpp"
#include "utils.hpp"

#include <darmok/optional_ref.hpp>
#include <darmok/physics3d.hpp>

#include <optional>
#include <vector>

namespace darmok
{
    class LuaEntity;
    class Scene;
    class Transform;
    struct Ray;
}

namespace darmok::physics3d
{
    class PhysicsSystem;
    class PhysicsBody;
    struct RaycastHit;

    class LuaCollisionListener final : public ITypeCollisionListener<LuaCollisionListener>
    {
    public:
        LuaCollisionListener(const sol::table& table) noexcept;
        sol::object getReal() const noexcept;

        void onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionExit(PhysicsBody& body1, PhysicsBody& body2) override;

    private:
        sol::main_table _table;
        static const LuaTableDelegateDefinition _enterDef;
        static const LuaTableDelegateDefinition _stayDef;
        static const LuaTableDelegateDefinition _exitDef;
    };

    class LuaCollisionListenerFilter final : public ICollisionListenerFilter
    {
    public:
        LuaCollisionListenerFilter(const sol::table& table) noexcept;
        bool operator()(const ICollisionListener& listener) const noexcept override;
    private:
        sol::table _table;
        entt::id_type _type;
    };

    class LuaPhysicsUpdater final : public ITypePhysicsUpdater<LuaPhysicsUpdater>
    {
    public:
        LuaPhysicsUpdater(const sol::object& obj) noexcept;
        const LuaDelegate& getDelegate() const noexcept;

        void fixedUpdate(float fixedDeltaTime) override;
    private:
        LuaDelegate _delegate;
    };

    class LuaPhysicsUpdaterFilter final : public IPhysicsUpdaterFilter
    {
    public:
        LuaPhysicsUpdaterFilter(const sol::object& obj) noexcept;
        bool operator()(const IPhysicsUpdater& updater) const noexcept override;
    private:
        sol::object _object;
        entt::id_type _type;
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

        static bool isValidEntity(PhysicsSystem& system, LuaEntity& entity) noexcept;

        static std::optional<RaycastHit> raycast1(const PhysicsSystem& system, const Ray& ray) noexcept;
        static std::optional<RaycastHit> raycast2(const PhysicsSystem& system, const Ray& ray, LayerMask layers) noexcept;
        static std::vector<RaycastHit> raycastAll1(const PhysicsSystem& system, const Ray& ray) noexcept;
        static std::vector<RaycastHit> raycastAll2(const PhysicsSystem& system, const Ray& ray, LayerMask layers) noexcept;
        
        static void activateBodies1(PhysicsSystem& system, const BoundingBox& bbox) noexcept;
        static void activateBodies2(PhysicsSystem& system, const BoundingBox& bbox, LayerMask layers) noexcept;

        static PhysicsSystem& addSceneComponent1(Scene& scene) noexcept;
        static PhysicsSystem& addSceneComponent2(Scene& scene, const Config& config) noexcept;
        static PhysicsSystem& addSceneComponent3(Scene& scene, const Config& config, bx::AllocatorI& alloc) noexcept;
        static OptionalRef<PhysicsSystem>::std_t getSceneComponent(Scene& scene) noexcept;
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

        static OptionalRef<PhysicsSystem>::std_t getSystem(PhysicsBody& body) noexcept;
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
        static std::optional<LuaEntity> getEntity(const PhysicsBody& body, const std::shared_ptr<Scene>& scene) noexcept;
    };
}