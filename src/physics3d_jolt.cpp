#include "physics3d_jolt.hpp"
#include <darmok/physics3d.hpp>
#include <darmok/transform.hpp>
#include <darmok/math.hpp>
#include <bx/allocator.h>
#include <glm/gtx/quaternion.hpp>
#include <thread>
#include <cstdarg>
#include <stdexcept>
#include <iostream>
#include <sstream>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace darmok
{
    struct JoltUtils final
    {
        // Jolt is right-handed, darmok is left-handed

        static JPH::Vec3 convert(const glm::vec3& v) noexcept
        {
            return JPH::Vec3(v.x, v.y, -v.z);
        }

        static glm::vec3 convert(const JPH::Vec3& v) noexcept
        {
            return glm::vec3(v.GetX(), v.GetY(), -v.GetZ());
        }

        static JPH::Vec3 convertSize(const glm::vec3& v) noexcept
        {
            return JPH::Vec3(v.x, v.y, v.z);
        }

        static JPH::Vec4 convert(const glm::vec4& v) noexcept
        {
            return JPH::Vec4(v.x, v.y, v.z, v.w);
        }

        static glm::vec4 convert(const JPH::Vec4& v) noexcept
        {
            return glm::vec4(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
        }

        static JPH::Mat44 convert(const glm::mat4& v) noexcept
        {
            auto fv = Math::flipHandedness(v);
            return JPH::Mat44(
                convert(fv[0]),
                convert(fv[1]),
                convert(fv[2]),
                convert(fv[3])
            );
        }

        static glm::mat4 convert(const JPH::Mat44& v) noexcept
        {
            glm::mat4 mat(
                convert(v.GetColumn4(0)),
                convert(v.GetColumn4(1)),
                convert(v.GetColumn4(2)),
                convert(v.GetColumn4(3))
            );
            return Math::flipHandedness(mat);
        }

        static JPH::Quat convert(const glm::quat& v) noexcept
        {
            auto fv = Math::flipHandedness(v);
            return JPH::Quat(fv.x, fv.y, fv.z, fv.w);
        }

        static glm::quat convert(const JPH::Quat& v) noexcept
        {
            glm::quat quat(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
            return Math::flipHandedness(quat);
        }
    };

    JPH::uint JoltBroadPhaseLayer::GetNumBroadPhaseLayers() const noexcept
    {
        return to_underlying(JoltLayer::Count);
    }

    JPH::BroadPhaseLayer JoltBroadPhaseLayer::GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const noexcept
    {
        JPH_ASSERT(inLayer < GetNumBroadPhaseLayers());
        return JPH::BroadPhaseLayer(inLayer);
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* JoltBroadPhaseLayer::GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const noexcept
    {
        switch ((JoltLayer)inLayer.GetValue())
        {
        case JoltLayer::NonMoving:
            return "NonMoving";
        case JoltLayer::Moving:
            return "Moving";
        default:
            JPH_ASSERT(false);
            return "Invalid";
        }
    }
#endif

    bool JoltObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const noexcept
    {
        switch ((JoltLayer)inLayer1)
        {
        case JoltLayer::NonMoving:
            return inLayer2.GetValue() == to_underlying(JoltLayer::Moving);
        case JoltLayer::Moving:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }

    bool JoltObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const noexcept
    {
        switch ((JoltLayer)inObject1)
        {
        case JoltLayer::NonMoving:
            return inObject2 == to_underlying(JoltLayer::Moving);
        case JoltLayer::Moving:
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
        bx::free(&_alloc, inAddress);
    }

    static void joltTraceImpl(const char* inFMT, ...) noexcept
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
    static bool joltAssertFailed(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
    {
        std::stringstream ss;
        ss << inFile << ":" << inLine << ": (" << inExpression << ") " << (inMessage != nullptr ? inMessage : "");
        throw std::runtime_error(ss.str());
        return true; // breakpoint
    };

#endif // JPH_ENABLE_ASSERTS

    Physics3dSystemImpl::Physics3dSystemImpl(bx::AllocatorI& alloc) noexcept
        : _alloc(alloc)
        , _deltaTimeRest(0.F)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace = joltTraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = joltAssertFailed;)
    }

    void Physics3dSystemImpl::init(Scene& scene, App& app) noexcept
    {
        if (_scene)
        {
            shutdown();
        }
        _scene = scene;
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        _threadPool = std::make_unique<JPH::JobSystemThreadPool>(
            JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1
        );
        _system = std::make_unique<JPH::PhysicsSystem>();
        _system->Init(_config.maxBodies, _config.numBodyMutexes,
            _config.maxBodyPairs, _config.maxContactConstraints,
            _broadPhaseLayer, _objVsBroadPhaseLayerFilter, _objLayerPairFilter);
        _deltaTimeRest = 0.F;
        _system->SetGravity(JoltUtils::convert(_config.gravity));

        auto& registry = scene.getRegistry();
        registry.on_construct<RigidBody3d>().connect<&Physics3dSystemImpl::onRigidbodyConstructed>(*this);
        registry.on_destroy<RigidBody3d>().connect< &Physics3dSystemImpl::onRigidbodyDestroyed>(*this);
    }

    void Physics3dSystemImpl::shutdown() noexcept
    {
        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            registry.on_construct<RigidBody3d>().disconnect<&Physics3dSystemImpl::onRigidbodyConstructed>(*this);
            registry.on_destroy<RigidBody3d>().disconnect< &Physics3dSystemImpl::onRigidbodyDestroyed>(*this);
        }

        _system.reset();
        _threadPool.reset();
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
        _scene.reset();
    }

    void Physics3dSystemImpl::onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        registry.get<RigidBody3d>(entity).getImpl().init(*this);
    }

    void Physics3dSystemImpl::onRigidbodyDestroyed(EntityRegistry& registry, Entity entity)
    {
        registry.get<RigidBody3d>(entity).getImpl().shutdown();
    }

    std::string Physics3dSystemImpl::getUpdateErrorString(JPH::EPhysicsUpdateError err) noexcept
    {
        switch (err)
        {
        case JPH::EPhysicsUpdateError::None:
            return "None";
        case JPH::EPhysicsUpdateError::ManifoldCacheFull:
            return "ManifoldCacheFull";
        case JPH::EPhysicsUpdateError::BodyPairCacheFull:
            return "BodyPairCacheFull";
        case JPH::EPhysicsUpdateError::ContactConstraintsFull:
            return "ContactConstraintsFull";
        default:
            JPH_ASSERT(false);
            return "";
        }
    }
    
    void Physics3dSystemImpl::update(float deltaTime)
    {
        if (!_system)
        {
            return;
        }
        _deltaTimeRest += deltaTime;
        auto fdt = _config.fixedDeltaTime;
        while (_deltaTimeRest > fdt)
        {
            auto err = _system->Update(fdt, _config.collisionSteps, &_alloc, _threadPool.get());
            if (err != JPH::EPhysicsUpdateError::None)
            {
                throw std::runtime_error(std::string("physics update error ") + getUpdateErrorString(err));
            }
            for (auto& updater : _updaters)
            {
                updater->fixedUpdate(fdt);
            }
            _deltaTimeRest -= fdt;
        }
        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            auto rigidBodies = registry.view<RigidBody3d>();
            for (auto [entity, rigidBody] : rigidBodies.each())
            {
                rigidBody.getImpl().update(entity, deltaTime);
            }
        }

    }
    
    void Physics3dSystemImpl::addUpdater(IPhysics3dUpdater& updater) noexcept
    {
        _updaters.emplace_back(updater);
    }
    
    bool Physics3dSystemImpl::removeUpdater(IPhysics3dUpdater& updater) noexcept
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

    const JoltPysicsConfig& Physics3dSystemImpl::getConfig() const noexcept
    {
        return _config;
    }

    OptionalRef<Scene> Physics3dSystemImpl::getScene() const noexcept
    {
        return _scene;
    }

    JPH::BodyInterface& Physics3dSystemImpl::getBodyInterface() noexcept
    {
        return _system->GetBodyInterface();
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
    
    void Physics3dSystem::addUpdater(IPhysics3dUpdater& updater) noexcept
    {
        _impl->addUpdater(updater);
    }
    
    bool Physics3dSystem::removeUpdater(IPhysics3dUpdater& updater) noexcept
    {
        return _impl->removeUpdater(updater);
    }

    RigidBody3dImpl::RigidBody3dImpl(const Shape& shape, float density, MotionType motion) noexcept
        : _shape(shape)
        , _motion(motion)
        , _density(density)
    {
    }

    RigidBody3dImpl::~RigidBody3dImpl()
    {
        shutdown();
    }

    void RigidBody3dImpl::init(Physics3dSystemImpl& system) noexcept
    {
        if (_system)
        {
            shutdown();
        }
        _system = system;
        // delay the body creation to get the updated transform
    }

    void RigidBody3dImpl::shutdown()
    {
        if (!_system)
        {
            return;
        }
    }

    OptionalRef<JPH::BodyInterface> RigidBody3dImpl::getBodyInterface() const noexcept
    {
        if (!_system)
        {
            return nullptr;
        }
        return _system->getBodyInterface();
    }

    JPH::BodyID RigidBody3dImpl::createBody(OptionalRef<Transform> trans) noexcept
    {
        if (!_system)
        {
            return {};
        }
        glm::vec3 pos(0);
        glm::quat rot(1, 0, 0, 0);
        if (trans)
        {
            pos = trans->getWorldPosition();
            rot = trans->getWorldRotation();
        }

        JPH::ShapeRefC shape = nullptr;
        if (auto cuboid = std::get_if<Cuboid>(&_shape))
        {
            JPH::BoxShapeSettings settings(JoltUtils::convertSize(cuboid->size * 0.5F));
            pos += cuboid->origin;
            shape = settings.Create().Get();
        }
        else if (auto sphere = std::get_if<Sphere>(&_shape))
        {
            JPH::SphereShapeSettings settings(sphere->radius);
            pos += sphere->origin;
            shape = settings.Create().Get();
        }
        else if (auto plane = std::get_if<Plane>(&_shape))
        {
            auto& config = _system->getConfig();

            pos += plane->origin;
            pos.y -= config.planeThickness * 0.5;
            rot *= glm::rotation(glm::vec3(0, 1, 0), plane->normal);
            JPH::Vec3 size(config.planeLength, config.planeThickness, config.planeLength);
            JPH::BoxShapeSettings settings(size * 0.5);
            shape = settings.Create().Get();
        }

        JPH::EMotionType joltMotion = JPH::EMotionType::Dynamic;
        auto objLayer = (JPH::ObjectLayer)JoltLayer::Moving;
        auto activation = JPH::EActivation::Activate;
        switch (_motion)
        {
        case RigidBody3dMotionType::Kinematic:
            joltMotion = JPH::EMotionType::Kinematic;
            break;
        case RigidBody3dMotionType::Static:
            joltMotion = JPH::EMotionType::Static;
            objLayer = (JPH::ObjectLayer)JoltLayer::NonMoving;
            activation = JPH::EActivation::DontActivate;
            break;
        }

        JPH::BodyCreationSettings settings(shape,
            JoltUtils::convert(pos), JoltUtils::convert(rot),
            joltMotion, objLayer);

        if (_density != 0.F)
        {
            auto mass = shape->GetVolume() * _density;
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
            settings.mMassPropertiesOverride.mMass = mass;
        }
        return _system->getBodyInterface().CreateAndAddBody(settings, activation);
    }

    void RigidBody3dImpl::update(Entity entity, float deltaTime)
    {
        if (!_system)
        {
            return;
        }
        auto trans = _system->getScene()->getComponent<Transform>(entity);
        auto& iface = _system->getBodyInterface();
        if (_bodyId.IsInvalid())
        {
            _bodyId = createBody(trans);
        }
        /*
        else if (trans && _motion == MotionType::Kinematic)
        {
            auto pos = JoltUtils::convert(trans->getWorldPosition());
            auto rot = JoltUtils::convert(trans->getWorldRotation());
            iface.SetPosition(_bodyId, pos, JPH::EActivation::Activate);
            iface.SetRotation(_bodyId, rot, JPH::EActivation::Activate);
        }*/
        else if (trans)
        {
            auto mat = JoltUtils::convert(iface.GetWorldTransform(_bodyId));
            auto parent = trans->getParent();
            if (parent)
            {
                mat = parent->getWorldInverse() * mat;
            }
            trans->setLocalMatrix(mat);
        }
    }

    const RigidBody3dImpl::Shape& RigidBody3dImpl::getShape() const noexcept
    {
        return _shape;
    }

    RigidBody3dImpl::MotionType RigidBody3dImpl::getMotionType() const noexcept
    {
        return _motion;
    }

    float RigidBody3dImpl::getDensity() const noexcept
    {
        return _density;
    }

    float RigidBody3dImpl::getMass() const noexcept
    {
        if (!_system || _bodyId.IsInvalid())
        {
            return 0.F;
        }
        auto shape = getBodyInterface()->GetShape(_bodyId);
        if (!shape)
        {
            return 0.F;
        }
        return shape->GetVolume() * _density;
    }

    void RigidBody3dImpl::setPosition(const glm::vec3& pos)
    {
        getBodyInterface()->SetPosition(_bodyId, JoltUtils::convert(pos), JPH::EActivation::Activate);
    }

    glm::vec3 RigidBody3dImpl::getPosition()
    {
        return JoltUtils::convert(getBodyInterface()->GetPosition(_bodyId));
    }

    void RigidBody3dImpl::setRotation(const glm::quat& rot)
    {
        getBodyInterface()->SetRotation(_bodyId, JoltUtils::convert(rot), JPH::EActivation::Activate);
    }

    glm::quat RigidBody3dImpl::getRotation()
    {
        return JoltUtils::convert(getBodyInterface()->GetRotation(_bodyId));
    }

    void RigidBody3dImpl::addTorque(const glm::vec3& torque)
    {
        getBodyInterface()->AddTorque(_bodyId, JoltUtils::convert(torque));
    }

    void RigidBody3dImpl::addForce(const glm::vec3& force)
    {
        getBodyInterface()->AddForce(_bodyId, JoltUtils::convert(force));
    }

    void RigidBody3dImpl::move(const glm::vec3& pos, const glm::quat& rot, float deltaTime)
    {
        getBodyInterface()->MoveKinematic(_bodyId,
            JoltUtils::convert(pos),
            JoltUtils::convert(rot),
            deltaTime);
    }

    void RigidBody3dImpl::movePosition(const glm::vec3& pos, float deltaTime)
    {
        auto iface = getBodyInterface();
        auto rot = iface->GetRotation(_bodyId);
        iface->MoveKinematic(_bodyId,
            JoltUtils::convert(pos),
            rot, deltaTime);
    }

    RigidBody3d::RigidBody3d(const Shape& shape, MotionType motion) noexcept
        : RigidBody3d(shape, 0.F, motion)
    {
    }

    RigidBody3d::RigidBody3d(const Shape& shape, float density, MotionType motion) noexcept
        : _impl(std::make_unique<RigidBody3dImpl>(shape, density, motion))
    {
    }

    RigidBody3d::~RigidBody3d() noexcept
    {
        // empty for the impl forward declaration
    }

    const RigidBody3d::Shape& RigidBody3d::getShape() const noexcept
    {
        return _impl->getShape();
    }

    RigidBody3d::MotionType RigidBody3d::getMotionType() const noexcept
    {
        return _impl->getMotionType();
    }

    float RigidBody3d::getDensity() const noexcept
    {
        return _impl->getDensity();
    }

    float RigidBody3d::getMass() const noexcept
    {
        return _impl->getMass();
    }

    RigidBody3d& RigidBody3d::setPosition(const glm::vec3& pos)
    {
        _impl->setPosition(pos);
        return *this;
    }

    glm::vec3 RigidBody3d::getPosition()
    {
        return _impl->getPosition();
    }

    RigidBody3d& RigidBody3d::setRotation(const glm::quat& rot)
    {
        _impl->setRotation(rot);
        return *this;
    }

    glm::quat RigidBody3d::getRotation()
    {
        return _impl->getRotation();
    }

    RigidBody3d& RigidBody3d::addTorque(const glm::vec3& torque)
    {
        _impl->addTorque(torque);
        return *this;
    }

    RigidBody3d& RigidBody3d::addForce(const glm::vec3& force)
    {
        _impl->addForce(force);
        return *this;
    }

    RigidBody3d& RigidBody3d::move(const glm::vec3& pos, const glm::quat& rot, float deltaTime)
    {
        _impl->move(pos, rot, deltaTime);
        return *this;
    }
    RigidBody3d& RigidBody3d::movePosition(const glm::vec3& pos, float deltaTime)
    {
        _impl->movePosition(pos, deltaTime);
        return *this;
    }

    RigidBody3dImpl& RigidBody3d::getImpl() noexcept
    {
        return *_impl;
    }

    const RigidBody3dImpl& RigidBody3d::getImpl() const noexcept
    {
        return *_impl;
    }
}