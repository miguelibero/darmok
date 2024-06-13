#pragma once

#include <vector>
#include <memory>
#include <darmok/utils.hpp>
#include <darmok/optional_ref.hpp>

#ifndef NDEBUG
// these seem to be missing in the library header in debug
#define JPH_PROFILE_ENABLED
#define JPH_DEBUG_RENDERER
#endif

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
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
}

namespace darmok
{
    class Scene;
    class App;
    class IPhysics3DUpdater;

    struct JoltPysicsConfig final
    {
        JPH::uint maxBodies = 1024;
        JPH::uint numBodyMutexes = 0;
        JPH::uint maxBodyPairs = 1024;
        JPH::uint maxContactConstraints = 1024;
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
        void update(float deltaTime) noexcept;
        void addUpdater(IPhysics3DUpdater& updater) noexcept;
        bool removeUpdater(IPhysics3DUpdater& updater) noexcept;
    private:
        JoltPysicsConfig _config;
        JoltBroadPhaseLayer _broadPhaseLayer;
        JoltObjectVsBroadPhaseLayerFilter _objVsBroadPhaseLayerFilter;
        JoltObjectLayerPairFilter _objLayerPairFilter;
        JoltTempAllocator _alloc;
        std::unique_ptr<JPH::PhysicsSystem> _system;
        std::unique_ptr<JPH::JobSystemThreadPool> _threadPool;
        std::vector<OptionalRef<IPhysics3DUpdater>> _updaters;
    };
}