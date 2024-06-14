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

namespace darmok
{
	class DARMOK_EXPORT BX_NO_VTABLE IPhysics3dUpdater
	{
	public:
		virtual ~IPhysics3dUpdater() = default;
		virtual void fixedUpdate(float fixedDeltaTime) = 0;
	};

    class RigidBody3d;

    struct Physics3dCollision final
    {
        glm::vec3 normal;
        std::vector<glm::vec3> contacts;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IPhysics3dCollisionListener
    {
    public:
        using Collision = Physics3dCollision;
        virtual ~IPhysics3dCollisionListener() = default;
        virtual void onCollisionEnter(RigidBody3d& rigidBody1, RigidBody3d& rigidBody2, const Collision& collision) {};
        virtual void onCollisionStay(RigidBody3d& rigidBody1, RigidBody3d& rigidBody2, const Collision& collision) {};
        virtual void onCollisionExit(RigidBody3d& rigidBody1, RigidBody3d& rigidBody2) {};
    };

    struct Physics3dConfig final
    {
        uint16_t maxBodies = 1024;
        uint16_t numBodyMutexes = 0;
        uint16_t maxBodyPairs = 1024;
        uint16_t maxContactConstraints = 1024;
        float fixedDeltaTime = 1.F / 60.F;
        uint16_t collisionSteps = 1;
        glm::vec3 gravity = { 0, -9.81F, 0 };
    };

    class Physics3dSystemImpl;

    class DARMOK_EXPORT Physics3dSystem final : public ISceneLogicUpdater
    {
    public:
        using Config = Physics3dConfig;
        Physics3dSystem(bx::AllocatorI& alloc, const Config& config = {}) noexcept;
        ~Physics3dSystem() noexcept;
        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;
        void addUpdater(IPhysics3dUpdater& updater) noexcept;
        bool removeUpdater(IPhysics3dUpdater& updater) noexcept;
        void addListener(IPhysics3dCollisionListener& listener) noexcept;
        bool removeListener(IPhysics3dCollisionListener& listener) noexcept;
    private:
        std::unique_ptr<Physics3dSystemImpl> _impl;
    };

    using Physics3dShape = std::variant<Cuboid, Sphere, Capsule>;

    class RigidBody3dImpl;

    class RigidBody3d final
    {
    public:
        using MotionType = RigidBody3dMotionType;
        using Shape = Physics3dShape;

        RigidBody3d(const Shape& shape, MotionType motion = MotionType::Dynamic) noexcept;
        RigidBody3d(const Shape& shape, float density, MotionType motion = MotionType::Dynamic) noexcept;
        ~RigidBody3d() noexcept;

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;
        float getDensity() const noexcept;
        float getMass() const noexcept;
        RigidBody3dImpl& getImpl() noexcept;
        const RigidBody3dImpl& getImpl() const noexcept;

        RigidBody3d& setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();
        RigidBody3d& setRotation(const glm::quat& rot);
        glm::quat getRotation();
        RigidBody3d& setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity();

        RigidBody3d& addTorque(const glm::vec3& torque);
        RigidBody3d& addForce(const glm::vec3& force);
        RigidBody3d& addImpulse(const glm::vec3& impulse);
        RigidBody3d& move(const glm::vec3& pos, const glm::quat& rot, float deltaTime = 0.F);
        RigidBody3d& movePosition(const glm::vec3& pos, float deltaTime = 0.F);

        RigidBody3d& addListener(IPhysics3dCollisionListener& listener) noexcept;
        bool removeListener(IPhysics3dCollisionListener& listener) noexcept;

    private:
        std::unique_ptr<RigidBody3dImpl> _impl;
    };
}