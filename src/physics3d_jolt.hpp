#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <deque>
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
#include <Jolt/Physics/Collision/ContactListener.h>
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
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const noexcept override;
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const noexcept override;
#endif
    };

    class JoltObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const noexcept override;
    };

    class JoltObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
    {
    public:
        bool ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const noexcept override;
    };

    class JoltTempAllocator final : public JPH::TempAllocator
    {
    public:
        JoltTempAllocator(bx::AllocatorI& alloc) noexcept;
        void* Allocate(JPH::uint size) noexcept override;
        void Free(void* address, JPH::uint size) noexcept override;
    private:
        bx::AllocatorI& _alloc;
    };

    class Physics3dSystemImpl final : public JPH::ContactListener
    {
    public:
        using Config = Physics3dConfig;
        Physics3dSystemImpl(bx::AllocatorI& alloc, const Config& config) noexcept;
        void init(Scene& scene, App& app) noexcept;
        void shutdown() noexcept;
        void update(float deltaTime);
        void addUpdater(IPhysics3dUpdater& updater) noexcept;
        bool removeUpdater(IPhysics3dUpdater& updater) noexcept;
        void addListener(IPhysics3dCollisionListener& listener) noexcept;
        bool removeListener(IPhysics3dCollisionListener& listener) noexcept;

        const Config& getConfig() const noexcept;
        OptionalRef<Scene> getScene() const noexcept;
        JPH::BodyInterface& getBodyInterface() noexcept;

        void onBodyCreated(RigidBody3d& rigidBody) noexcept;

        void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
        void OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const  JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
        void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

    private:
        OptionalRef<Scene> _scene;
        float _deltaTimeRest;
        Config _config;
        JoltBroadPhaseLayer _broadPhaseLayer;
        JoltObjectVsBroadPhaseLayerFilter _objVsBroadPhaseLayerFilter;
        JoltObjectLayerPairFilter _objLayerPairFilter;
        JoltTempAllocator _alloc;
        std::unique_ptr<JPH::PhysicsSystem> _system;
        std::unique_ptr<JPH::JobSystemThreadPool> _threadPool;
        std::vector<OptionalRef<IPhysics3dUpdater>> _updaters;
        std::vector<OptionalRef<IPhysics3dCollisionListener>> _listeners;

        struct CollisionEnterEvent final
        {
            Physics3dCollision collision;
        };

        struct CollisionStayEvent final
        {
            Physics3dCollision collision;
        };

        struct CollisionExitEvent final
        {
            Physics3dCollision collision;
        };
        using CollisionEvent = std::variant<CollisionEnterEvent, CollisionStayEvent, CollisionExitEvent>;

        mutable std::mutex _rigidBodiesMutex;
        std::unordered_map<JPH::BodyID, OptionalRef<RigidBody3d>> _rigidBodies;

        mutable std::mutex _collisionEventsMutex;
        std::deque<CollisionEvent> _pendingCollisionEvents;

        void processPendingCollisionEvents();
        static Physics3dCollision createCollision(RigidBody3d& rigidBody1, RigidBody3d& rigidBody2, OptionalRef<const JPH::ContactManifold> manifold = nullptr) noexcept;
        OptionalRef<RigidBody3d> getRigidBody(const JPH::BodyID& bodyId) const noexcept;
        void onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept;
        void onRigidbodyDestroyed(EntityRegistry& registry, Entity entity);
        static std::string getUpdateErrorString(JPH::EPhysicsUpdateError err) noexcept;

        void onCollisionEnter(const Physics3dCollision& collision);
        void onCollisionStay(const Physics3dCollision& collision);
        void onCollisionExit(const Physics3dCollision& collision);
    };

    class RigidBody3dImpl final
    {
    public:
        using Shape = RigidBody3dShape;
        using MotionType = RigidBody3dMotionType;

        RigidBody3dImpl(const Shape& shape, float density = 0.F, MotionType motion = MotionType::Dynamic) noexcept;
        ~RigidBody3dImpl();
        void init(RigidBody3d& rigidBody, Physics3dSystemImpl& system) noexcept;
        void shutdown();
        void update(Entity entity, float deltaTime);

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;
        float getDensity() const noexcept;
        float getMass() const noexcept;
        const JPH::BodyID& getBodyId() const noexcept;

        void setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();
        void setRotation(const glm::quat& rot);
        glm::quat getRotation();

        void addTorque(const glm::vec3& torque);
        void addForce(const glm::vec3& force);
        void move(const glm::vec3& pos, const glm::quat& rot, float deltaTime );
        void movePosition(const glm::vec3& pos, float deltaTime);

        void addListener(IPhysics3dCollisionListener& listener) noexcept;
        bool removeListener(IPhysics3dCollisionListener& listener) noexcept;

        void onCollisionEnter(const Physics3dCollision& collision);
        void onCollisionStay(const Physics3dCollision& collision);
        void onCollisionExit(const Physics3dCollision& collision);

    private:
        OptionalRef<JPH::BodyInterface> getBodyInterface() const noexcept;
        JPH::BodyID createBody(OptionalRef<Transform> transform) noexcept;

        OptionalRef<RigidBody3d> _rigidBody;
        OptionalRef<Physics3dSystemImpl> _system;
        Shape _shape;
        MotionType _motion;
        float _density;
        JPH::BodyID _bodyId;
        std::vector<OptionalRef<IPhysics3dCollisionListener>> _listeners;
    };
}