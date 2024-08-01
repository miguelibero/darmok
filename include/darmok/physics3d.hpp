#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/shape.hpp>
#include <darmok/physics3d_fwd.hpp>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <variant>
#include <vector>
#include <optional>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class Camera;
    class Program;
}

namespace darmok::physics3d
{
	class DARMOK_EXPORT BX_NO_VTABLE IPhysicsUpdater
	{
	public:
		virtual ~IPhysicsUpdater() = default;
		virtual void fixedUpdate(float fixedDeltaTime) = 0;
	};

    class PhysicsBody;

    struct DARMOK_EXPORT Collision final
    {
        glm::vec3 normal;
        std::vector<glm::vec3> contacts;

        std::string toString() const noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ICollisionListener
    {
    public:
        virtual ~ICollisionListener() = default;
        virtual void onCollisionEnter(PhysicsBody& physicsBody1, PhysicsBody& physicsBody2, const Collision& collision) {};
        virtual void onCollisionStay(PhysicsBody& physicsBody1, PhysicsBody& physicsBody2, const Collision& collision) {};
        virtual void onCollisionExit(PhysicsBody& physicsBody1, PhysicsBody& physicsBody2) {};
    };

    struct PhysicsSystemConfig final
    {
        uint16_t maxBodies = 1024;
        uint16_t numBodyMutexes = 0;
        uint16_t maxBodyPairs = 1024;
        uint16_t maxContactConstraints = 1024;
        float fixedDeltaTime = 1.F / 60.F;
        uint16_t collisionSteps = 1;
        glm::vec3 gravity = { 0, -9.81F, 0 };
        std::vector<std::string> layers = { "default" };
    };

    struct RaycastHit final
    {
        std::reference_wrapper<PhysicsBody> physicsBody;
        float distance;

        std::string toString() const noexcept;
    };

    class PhysicsSystemImpl;

    class DARMOK_EXPORT PhysicsSystem final : public ISceneComponent
    {
    public:
        using Config = PhysicsSystemConfig;
        PhysicsSystem(const Config& config = {}) noexcept;
        PhysicsSystem(const Config& config, bx::AllocatorI& alloc) noexcept;
        PhysicsSystem(bx::AllocatorI& alloc) noexcept;
        ~PhysicsSystem() noexcept;

        PhysicsSystemImpl& getImpl() noexcept;
        const PhysicsSystemImpl& getImpl() const noexcept;

        PhysicsSystem& setRootTransform(OptionalRef<Transform> root) noexcept;
        OptionalRef<Transform> getRootTransform() noexcept;

        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        
        void addUpdater(IPhysicsUpdater& updater) noexcept;
        bool removeUpdater(IPhysicsUpdater& updater) noexcept;
        void addListener(ICollisionListener& listener) noexcept;
        bool removeListener(ICollisionListener& listener) noexcept;

        std::optional<RaycastHit> raycast(const Ray& ray, float maxDistance = bx::kFloatInfinity, uint16_t layerMask = 255) noexcept;
        std::vector<RaycastHit> raycastAll(const Ray& ray, float maxDistance = bx::kFloatInfinity, uint16_t layerMask = 255) noexcept;
    private:
        std::unique_ptr<PhysicsSystemImpl> _impl;
    };

    struct PhysicsBodyConfig final
    {
        using Shape = PhysicsShape;
        using MotionType = PhysicsBodyMotionType;
        Shape shape;
        MotionType motion = MotionType::Dynamic;
        std::optional<float> mass = std::nullopt;
        float friction = 0.2;
        float gravityFactor = 1.0F;
        uint8_t layer = 0;
        bool trigger = false;
    };

    class PhysicsBodyImpl;
    struct CharacterConfig;

    class PhysicsBody final
    {
    public:
        using MotionType = PhysicsBodyMotionType;
        using Shape = PhysicsShape;
        using Config = PhysicsBodyConfig;

        PhysicsBody(const Shape& shape, MotionType motion = MotionType::Dynamic) noexcept;
        PhysicsBody(const Config& config) noexcept;
        PhysicsBody(const CharacterConfig& config) noexcept;
        ~PhysicsBody() noexcept;

        PhysicsBodyImpl& getImpl() noexcept;
        const PhysicsBodyImpl& getImpl() const noexcept;

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;

        PhysicsBody& setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();
        PhysicsBody& setRotation(const glm::quat& rot);
        glm::quat getRotation();
        PhysicsBody& setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity();

        PhysicsBody& addTorque(const glm::vec3& torque);
        PhysicsBody& addForce(const glm::vec3& force);
        PhysicsBody& addImpulse(const glm::vec3& impulse);
        PhysicsBody& move(const glm::vec3& pos, const glm::quat& rot, float deltaTime = 0.F);
        PhysicsBody& movePosition(const glm::vec3& pos, float deltaTime = 0.F);

        PhysicsBody& addListener(ICollisionListener& listener) noexcept;
        bool removeListener(ICollisionListener& listener) noexcept;

        std::string toString() const noexcept;

    private:
        std::unique_ptr<PhysicsBodyImpl> _impl;
    };
}