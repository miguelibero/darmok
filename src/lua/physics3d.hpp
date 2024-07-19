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
    class Transform;
    class Scene;
    struct Ray;
}

namespace darmok::physics3d
{
    class PhysicsSystem;
    class PhysicsBody;
    struct RaycastHit;

    class LuaPhysicsSystem final : public IPhysicsUpdater, public ICollisionListener
    {
    public:
        using Config = PhysicsSystemConfig;

        LuaPhysicsSystem(PhysicsSystem& system, const std::shared_ptr<Scene>& scene) noexcept;
        ~LuaPhysicsSystem() noexcept;
        LuaPhysicsSystem& registerUpdate(const sol::protected_function& func) noexcept;
        bool unregisterUpdate(const sol::protected_function& func) noexcept;
        LuaPhysicsSystem& addListener(const sol::table& listener) noexcept;
        bool removeListener(const sol::table& listener) noexcept;

        PhysicsSystem& getReal() noexcept;
        const PhysicsSystem& getReal() const noexcept;

        OptionalRef<Transform>::std_t getRootTransform() const noexcept;
        void setRootTransform(OptionalRef<Transform>::std_t root) noexcept;

        std::optional<RaycastHit> raycast1(const Ray& ray) noexcept;
        std::optional<RaycastHit> raycast2(const Ray& ray, float maxDistance) noexcept;
        std::optional<RaycastHit> raycast3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;
        std::vector<RaycastHit> raycastAll1(const Ray& ray) noexcept;
        std::vector<RaycastHit> raycastAll2(const Ray& ray, float maxDistance) noexcept;
        std::vector<RaycastHit> raycastAll3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;

        static LuaPhysicsSystem addSceneComponent1(LuaScene& scene) noexcept;
        static LuaPhysicsSystem addSceneComponent2(LuaScene& scene, const Config& config) noexcept;
        static LuaPhysicsSystem addSceneComponent3(LuaScene& scene, const Config& config, bx::AllocatorI& alloc) noexcept;

        void fixedUpdate(float fixedDeltaTime) override;
        void onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionExit(PhysicsBody& body1, PhysicsBody& body2) override;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<PhysicsSystem> _system;
        std::shared_ptr<Scene> _scene;
        std::vector<sol::protected_function> _updates;
        std::vector<sol::table> _listeners;
    };

    class PhysicsBody;
    struct PhysicsBodyConfig;
    struct CharacterConfig;

    class LuaPhysicsBody final : public ICollisionListener
    {
    public:
        using MotionType = PhysicsBodyMotionType;
        using Shape = PhysicsShape;
        using Config = PhysicsBodyConfig;

        LuaPhysicsBody(PhysicsBody& body, const std::shared_ptr<Scene>& scene) noexcept;
        ~LuaPhysicsBody() noexcept;

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;

        void setPosition(const VarLuaTable<glm::vec3>& pos) noexcept;
        glm::vec3 getPosition() const noexcept;
        void setRotation(const VarLuaTable<glm::quat>& rot) noexcept;
        glm::quat getRotation() const noexcept;
        void setLinearVelocity(const VarLuaTable<glm::vec3>& velocity) noexcept;
        glm::vec3 getLinearVelocity() const noexcept;

        LuaPhysicsBody& addTorque(const VarLuaTable<glm::vec3>& torque) noexcept;
        LuaPhysicsBody& addForce(const VarLuaTable<glm::vec3>& force) noexcept;
        LuaPhysicsBody& addImpulse(const VarLuaTable<glm::vec3>& impulse) noexcept;
        LuaPhysicsBody& move1(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot) noexcept;
        LuaPhysicsBody& move2(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot, float deltaTime) noexcept;
        LuaPhysicsBody& movePosition1(const VarLuaTable<glm::vec3>& pos) noexcept;
        LuaPhysicsBody& movePosition2(const VarLuaTable<glm::vec3>& pos, float deltaTime) noexcept;

        LuaPhysicsBody& addListener(const sol::table& listener) noexcept;
        bool removeListener(const sol::table& listener) noexcept;

        void onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override;
        void onCollisionExit(PhysicsBody& body1, PhysicsBody& body2) override;
        
        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<PhysicsBody> _body;
        std::shared_ptr<Scene> _scene;
        std::vector<sol::table> _listeners;

        static LuaPhysicsBody addEntityComponent1(LuaEntity& entity, const Shape& shape) noexcept;
        static LuaPhysicsBody addEntityComponent2(LuaEntity& entity, const Shape& shape, MotionType motion) noexcept;
        static LuaPhysicsBody addEntityComponent3(LuaEntity& entity, const Config& config) noexcept;
        static LuaPhysicsBody addEntityComponent4(LuaEntity& entity, const CharacterConfig& config) noexcept;
        static std::optional<LuaPhysicsBody> getEntityComponent(LuaEntity& entity) noexcept;
        std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
    };
}