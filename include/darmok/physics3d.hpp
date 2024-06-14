#pragma once

#include <darmok/scene.hpp>
#include <darmok/shape.hpp>
#include <darmok/physics3d_fwd.hpp>
#include <memory>
#include <bx/bx.h>
#include <variant>

namespace bx
{
    struct AllocatorI;
}

namespace darmok::physics3d
{
	class DARMOK_EXPORT BX_NO_VTABLE IPhysicsUpdater
	{
	public:
		virtual ~IPhysicsUpdater() = default;
		virtual void fixedUpdate(float fixedDeltaTime) = 0;
	};

    class RigidBody;

    struct Collision final
    {
        glm::vec3 normal;
        std::vector<glm::vec3> contacts;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ICollisionListener
    {
    public:
        virtual ~ICollisionListener() = default;
        virtual void onCollisionEnter(RigidBody& rigidBody1, RigidBody& rigidBody2, const Collision& collision) {};
        virtual void onCollisionStay(RigidBody& rigidBody1, RigidBody& rigidBody2, const Collision& collision) {};
        virtual void onCollisionExit(RigidBody& rigidBody1, RigidBody& rigidBody2) {};
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
    };

    class PhysicsSystemImpl;

    class DARMOK_EXPORT PhysicsSystem final : public ISceneLogicUpdater
    {
    public:
        using Config = PhysicsSystemConfig;
        PhysicsSystem(bx::AllocatorI& alloc, const Config& config = {}) noexcept;
        ~PhysicsSystem() noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        void addUpdater(IPhysicsUpdater& updater) noexcept;
        bool removeUpdater(IPhysicsUpdater& updater) noexcept;
        void addListener(ICollisionListener& listener) noexcept;
        bool removeListener(ICollisionListener& listener) noexcept;
    private:
        std::unique_ptr<PhysicsSystemImpl> _impl;
    };

    using PhysicsShape = std::variant<Cuboid, Sphere, Capsule>;

    struct RigidBodyConfig final
    {
        using Shape = PhysicsShape;
        using MotionType = RigidBodyMotionType;
        Shape shape;
        MotionType motion = MotionType::Dynamic;
        std::optional<float> mass = std::nullopt;
        float friction = 0.2;
        float gravityFactor = 1.0F;
    };

    class RigidBodyImpl;
    struct CharacterConfig;

    class RigidBody final
    {
    public:
        using MotionType = RigidBodyMotionType;
        using Shape = PhysicsShape;
        using Config = RigidBodyConfig;

        RigidBody(const Shape& shape, MotionType motion = MotionType::Dynamic) noexcept;
        RigidBody(const Config& config) noexcept;
        RigidBody(const CharacterConfig& config) noexcept;
        ~RigidBody() noexcept;

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;
        RigidBodyImpl& getImpl() noexcept;
        const RigidBodyImpl& getImpl() const noexcept;

        RigidBody& setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();
        RigidBody& setRotation(const glm::quat& rot);
        glm::quat getRotation();
        RigidBody& setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity();

        RigidBody& addTorque(const glm::vec3& torque);
        RigidBody& addForce(const glm::vec3& force);
        RigidBody& addImpulse(const glm::vec3& impulse);
        RigidBody& move(const glm::vec3& pos, const glm::quat& rot, float deltaTime = 0.F);
        RigidBody& movePosition(const glm::vec3& pos, float deltaTime = 0.F);

        RigidBody& addListener(ICollisionListener& listener) noexcept;
        bool removeListener(ICollisionListener& listener) noexcept;

    private:
        std::unique_ptr<RigidBodyImpl> _impl;
    };
}