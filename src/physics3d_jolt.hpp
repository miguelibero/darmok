#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <darmok/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/physics3d.hpp>

#ifndef NDEBUG
// these seem to be missing in the library header in debug
#define JPH_PROFILE_ENABLED
#define JPH_DEBUG_RENDERER
#endif

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EPhysicsUpdateError.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Core/JobSystemThreadPool.h>

namespace bx
{
    struct AllocatorI;
}

namespace JPH
{
    class PhysicsSystem;
    class BroadPhaseLayer;
    class BodyInterface;
}

namespace darmok
{
    class Scene;
    class App;
    class IPhysics3dUpdater;
    class RigidBody3d;
    class Transform;

    struct JoltPysicsConfig final
    {
        JPH::uint maxBodies = 1024;
        JPH::uint numBodyMutexes = 0;
        JPH::uint maxBodyPairs = 1024;
        JPH::uint maxContactConstraints = 1024;
        float fixedDeltaTime = 1.F / 60.F;
        JPH::uint collisionSteps = 1;
        float planeLength = 1000.F;
        float planeThickness = 1.F;
        glm::vec3 gravity = { 0, -9.81F, 0 };
    };

    enum class JoltLayer : uint8_t
    {
        NonMoving,
        Moving,
        Count
    };

    class JoltBroadPhaseLayer final : public JPH::BroadPhaseLayerInterface
    {
    public:
        JPH::uint GetNumBroadPhaseLayers() const noexcept override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const noexcept override;
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const noexcept override;
#endif
    };

    class JoltObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const noexcept override;
    };

    class JoltObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const noexcept override;
    };

    class JoltTempAllocator final : public JPH::TempAllocator
    {
    public:
        JoltTempAllocator(bx::AllocatorI& alloc) noexcept;
        void* Allocate(JPH::uint inSize) noexcept override;
        void Free(void* inAddress, JPH::uint inSize) noexcept override;
    private:
        bx::AllocatorI& _alloc;
    };

    class Physics3dSystemImpl final
    {
    public:
        Physics3dSystemImpl(bx::AllocatorI& alloc) noexcept;
        void init(Scene& scene, App& app) noexcept;
        void shutdown() noexcept;
        void update(float deltaTime);
        void addUpdater(IPhysics3dUpdater& updater) noexcept;
        bool removeUpdater(IPhysics3dUpdater& updater) noexcept;

        const JoltPysicsConfig& getConfig() const noexcept;
        OptionalRef<Scene> getScene() const noexcept;
        JPH::BodyInterface& getBodyInterface() noexcept;
    private:
        OptionalRef<Scene> _scene;
        float _deltaTimeRest;
        JoltPysicsConfig _config;
        JoltBroadPhaseLayer _broadPhaseLayer;
        JoltObjectVsBroadPhaseLayerFilter _objVsBroadPhaseLayerFilter;
        JoltObjectLayerPairFilter _objLayerPairFilter;
        JoltTempAllocator _alloc;
        std::unique_ptr<JPH::PhysicsSystem> _system;
        std::unique_ptr<JPH::JobSystemThreadPool> _threadPool;
        std::vector<OptionalRef<IPhysics3dUpdater>> _updaters;

        void onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept;
        void onRigidbodyDestroyed(EntityRegistry& registry, Entity entity);
        static std::string getUpdateErrorString(JPH::EPhysicsUpdateError err) noexcept;
    };

    class RigidBody3dImpl final
    {
    public:
        using Shape = RigidBody3dShape;
        using MotionType = RigidBody3dMotionType;

        RigidBody3dImpl(const Shape& shape, float density = 0.F, MotionType motion = MotionType::Dynamic) noexcept;
        ~RigidBody3dImpl();
        void init(Physics3dSystemImpl& system) noexcept;
        void shutdown();
        void update(Entity entity, float deltaTime);

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;
        float getDensity() const noexcept;
        float getMass() const noexcept;

        void setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();
        void setRotation(const glm::quat& rot);
        glm::quat getRotation();

        void addTorque(const glm::vec3& torque);
        void addForce(const glm::vec3& force);
        void move(const glm::vec3& pos, const glm::quat& rot, float deltaTime );
        void movePosition(const glm::vec3& pos, float deltaTime);

    private:
        OptionalRef<JPH::BodyInterface> getBodyInterface() const noexcept;

        JPH::BodyID createBody(OptionalRef<Transform> transform) noexcept;
        OptionalRef<Physics3dSystemImpl> _system;
        Shape _shape;
        MotionType _motion;
        float _density;
        JPH::BodyID _bodyId;
    };
}