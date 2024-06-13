#include "physics3d_jolt.hpp"
#include <darmok/physics3d.hpp>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <bx/allocator.h>
#include <thread>
#include <cstdarg>
#include <stdexcept>
#include <iostream>
#include <sstream>

namespace darmok
{
    JPH::uint JoltBroadPhaseLayer::GetNumBroadPhaseLayers() const noexcept
    {
        return to_underlying(Physics3dLayer::Count);
    }

    JPH::BroadPhaseLayer JoltBroadPhaseLayer::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const noexcept
    {
        JPH_ASSERT(inLayer < GetNumBroadPhaseLayers());
        return JPH::BroadPhaseLayer(inLayer);
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* JoltBroadPhaseLayer::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const noexcept
    {
        switch ((Physics3dLayer)inLayer.GetValue())
        {
        case Physics3dLayer::NonMoving:
            return "NonMoving";
        case Physics3dLayer::Moving:
            return "Moving";
        default:
            JPH_ASSERT(false);
            return "Invalid";
        }
    }
#endif

    bool JoltObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const noexcept
    {
        switch ((Physics3dLayer)inLayer1)
        {
        case Physics3dLayer::NonMoving:
            return inLayer2.GetValue() == to_underlying(Physics3dLayer::Moving);
        case Physics3dLayer::Moving:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }

    bool JoltObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const noexcept
    {
        switch ((Physics3dLayer)inObject1)
        {
        case Physics3dLayer::NonMoving:
            return inObject2 == to_underlying(Physics3dLayer::Moving);
        case Physics3dLayer::Moving:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }

    JoltTempAllocator::JoltTempAllocator(bx::AllocatorI& alloc) noexcept
        : _alloc(alloc)
    {
    }

    void* JoltTempAllocator::Allocate(JPH::uint inSize) noexcept
    {
        return bx::alloc(&_alloc, inSize);
    }

    void JoltTempAllocator::Free(void* inAddress, JPH::uint inSize) noexcept
    {
        bx::free(&_alloc, inAddress, inSize);
    }

    static void JoltTraceImpl(const char* inFMT, ...) noexcept
    {
        va_list list;
        va_start(list, inFMT);
        char buffer[1024];
        bx::vsnprintf(buffer, sizeof(buffer), inFMT, list);
        va_end(list);

        std::cout << buffer << std::endl;
    }

#ifdef JPH_ENABLE_ASSERTS

    // Callback for asserts, connect this to your own assert handler if you have one
    static bool JoltAssertFailed(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
    {
        std::stringstream ss;
        ss << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "");
        throw std::runtime_error(ss.str());
        return true; // breakpoint
    };

#endif // JPH_ENABLE_ASSERTS

    Physics3dSystemImpl::Physics3dSystemImpl(bx::AllocatorI& alloc) noexcept
        : _alloc(alloc)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace = JoltTraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = JoltAssertFailed;)
    }

    void Physics3dSystemImpl::init(Scene& scene, App& app) noexcept
    {
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        _threadPool = std::make_unique<JPH::JobSystemThreadPool>(
            JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1
        );
        _system = std::make_unique<JPH::PhysicsSystem>();
        _system->Init(_config.maxBodies, _config.numBodyMutexes,
            _config.maxBodyPairs, _config.maxContactConstraints,
            _broadPhaseLayer, _objVsBroadPhaseLayerFilter, _objLayerPairFilter);
    }

    void Physics3dSystemImpl::shutdown() noexcept
    {
        _system.reset();
        _threadPool.reset();
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
    
    void Physics3dSystemImpl::update(float deltaTime) noexcept
    {
    }
    
    void Physics3dSystemImpl::addUpdater(IPhysics3DUpdater& updater) noexcept
    {
        _updaters.emplace_back(updater);
    }
    
    bool Physics3dSystemImpl::removeUpdater(IPhysics3DUpdater& updater) noexcept
    {
        auto ptr = &updater;
        auto itr = std::find_if(_updaters.begin(), _updaters.end(), [ptr](auto& ref){ return ref.ptr() == ptr; });
        if(itr == _updaters.end())
        {
            return false;
        }
        _updaters.erase(itr);
        return true;
    }
    
    Physics3dSystem::Physics3dSystem(bx::AllocatorI& alloc) noexcept
        : _impl(std::make_unique<Physics3dSystemImpl>(alloc))
    {
    }

    Physics3dSystem::~Physics3dSystem() noexcept
    {
        // implemented to do forward declaration of impl
    }
    
    void Physics3dSystem::init(Scene& scene, App& app) noexcept
    {
        _impl->init(scene, app);
    }
    
    void Physics3dSystem::shutdown() noexcept
    {
        _impl->shutdown();
    }
    
    void Physics3dSystem::update(float deltaTime) noexcept
    {
        _impl->update(deltaTime);
    }
    
    void Physics3dSystem::addUpdater(IPhysics3DUpdater& updater) noexcept
    {
        _impl->addUpdater(updater);
    }
    
    bool Physics3dSystem::removeUpdater(IPhysics3DUpdater& updater) noexcept
    {
        return _impl->removeUpdater(updater);
    }
}