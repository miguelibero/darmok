#include "physics3d_jolt.hpp"
#include "character_jolt.hpp"
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
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

namespace darmok
{
    // Jolt is right-handed, darmok is left-handed

    JPH::Vec3 JoltUtils::convert(const glm::vec3& v) noexcept
    {
        return JPH::Vec3(v.x, v.y, -v.z);
    }

    glm::vec3 JoltUtils::convert(const JPH::Vec3& v) noexcept
    {
        return glm::vec3(v.GetX(), v.GetY(), -v.GetZ());
    }

    JPH::Vec3 JoltUtils::convertSize(const glm::vec3& v) noexcept
    {
        return JPH::Vec3(v.x, v.y, v.z);
    }

    JPH::Vec4 JoltUtils::convert(const glm::vec4& v) noexcept
    {
        return JPH::Vec4(v.x, v.y, v.z, v.w);
    }

    glm::vec4 JoltUtils::convert(const JPH::Vec4& v) noexcept
    {
        return glm::vec4(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
    }

    JPH::Mat44 JoltUtils::convert(const glm::mat4& v) noexcept
    {
        auto fv = Math::flipHandedness(v);
        return JPH::Mat44(
            convert(fv[0]),
            convert(fv[1]),
            convert(fv[2]),
            convert(fv[3])
        );
    }

    glm::mat4 JoltUtils::convert(const JPH::Mat44& v) noexcept
    {
        glm::mat4 mat(
            convert(v.GetColumn4(0)),
            convert(v.GetColumn4(1)),
            convert(v.GetColumn4(2)),
            convert(v.GetColumn4(3))
        );
        return Math::flipHandedness(mat);
    }

    JPH::Quat JoltUtils::convert(const glm::quat& v) noexcept
    {
        auto fv = Math::flipHandedness(v);
        return JPH::Quat(fv.x, fv.y, fv.z, fv.w);
    }

    glm::quat JoltUtils::convert(const JPH::Quat& v) noexcept
    {
        glm::quat quat(v.GetX(), v.GetY(), v.GetZ(), v.GetW());
        return Math::flipHandedness(quat);
    }

    static JPH::ShapeRefC physics3dAdjustShapeOrigin(const glm::vec3& origin, JPH::ShapeSettings& settings) noexcept
    {
        JPH::ShapeRefC shape = settings.Create().Get();
        if (origin == glm::vec3(0))
        {
            return shape;
        }
        JPH::RotatedTranslatedShapeSettings transSettings(JoltUtils::convert(origin), JPH::Quat::sIdentity(), shape);
        return transSettings.Create().Get();
    };

    JPH::ShapeRefC JoltUtils::convert(const Shape& shape) noexcept
    {
        if (auto cuboid = std::get_if<Cuboid>(&shape))
        {
            JPH::BoxShapeSettings settings(JoltUtils::convertSize(cuboid->size * 0.5F));
            return physics3dAdjustShapeOrigin(cuboid->origin, settings);
        }
        else if (auto sphere = std::get_if<Sphere>(&shape))
        {
            JPH::SphereShapeSettings settings(sphere->radius);
            return physics3dAdjustShapeOrigin(sphere->origin, settings);
        }
        else if (auto caps = std::get_if<Capsule>(&shape))
        {
            JPH::CapsuleShapeSettings settings(caps->cylinderHeight * 0.5, caps->radius);
            return physics3dAdjustShapeOrigin(caps->origin, settings);
        }
        return nullptr;
    }

    JPH::uint JoltBroadPhaseLayer::GetNumBroadPhaseLayers() const noexcept
    {
        return to_underlying(JoltLayer::Count);
    }

    JPH::BroadPhaseLayer JoltBroadPhaseLayer::GetBroadPhaseLayer(JPH::ObjectLayer layer) const noexcept
    {
        JPH_ASSERT(layer < GetNumBroadPhaseLayers());
        return JPH::BroadPhaseLayer(layer);
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* JoltBroadPhaseLayer::GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const noexcept
    {
        switch ((JoltLayer)layer.GetValue())
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

    bool JoltObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const noexcept
    {
        switch ((JoltLayer)layer1)
        {
        case JoltLayer::NonMoving:
            return layer2.GetValue() == to_underlying(JoltLayer::Moving);
        case JoltLayer::Moving:
            return true;
        default:
            JPH_ASSERT(false);
            return false;
        }
    }

    bool JoltObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const noexcept
    {
        switch ((JoltLayer)object1)
        {
        case JoltLayer::NonMoving:
            return object2 == to_underlying(JoltLayer::Moving);
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

    void* JoltTempAllocator::Allocate(JPH::uint size) noexcept
    {
        return bx::alloc(&_alloc, size);
    }

    void JoltTempAllocator::Free(void* addr, JPH::uint size) noexcept
    {
        bx::free(&_alloc, addr);
    }

    static void joltTraceImpl(const char* fmt, ...) noexcept
    {
        va_list list;
        va_start(list, fmt);
        char buffer[1024];
        bx::vsnprintf(buffer, sizeof(buffer), fmt, list);
        va_end(list);

        std::cout << buffer << std::endl;
    }

#ifdef JPH_ENABLE_ASSERTS

    // Callback for asserts, connect this to your own assert handler if you have one
    static bool joltAssertFailed(const char* expression, const char* message, const char* file, JPH::uint line)
    {
        std::stringstream ss;
        ss << file << ":" << line << ": (" << expression << ") " << (message != nullptr ? message : "");
        // throw std::runtime_error(ss.str());
        return true; // breakpoint
    };

#endif // JPH_ENABLE_ASSERTS

    Physics3dSystemImpl::Physics3dSystemImpl(bx::AllocatorI& alloc, const Config& config) noexcept
        : _alloc(alloc)
        , _config(config)
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
        _system->SetContactListener(this);

        auto& registry = scene.getRegistry();
        registry.on_construct<RigidBody3d>().connect<&Physics3dSystemImpl::onRigidbodyConstructed>(*this);
        registry.on_destroy<RigidBody3d>().connect< &Physics3dSystemImpl::onRigidbodyDestroyed>(*this);
        registry.on_construct<CharacterController>().connect<&Physics3dSystemImpl::onCharacterConstructed>(*this);
        registry.on_destroy<CharacterController>().connect< &Physics3dSystemImpl::onCharacterDestroyed>(*this);
    }

    void Physics3dSystemImpl::shutdown() noexcept
    {
        processPendingCollisionEvents();
        for (auto& elm : _rigidBodies)
        {
            elm.second->getImpl().shutdown();
        }
        _rigidBodies.clear();

        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            registry.on_construct<RigidBody3d>().disconnect<&Physics3dSystemImpl::onRigidbodyConstructed>(*this);
            registry.on_destroy<RigidBody3d>().disconnect< &Physics3dSystemImpl::onRigidbodyDestroyed>(*this);
            registry.on_construct<CharacterController>().disconnect<&Physics3dSystemImpl::onCharacterConstructed>(*this);
            registry.on_destroy<CharacterController>().disconnect< &Physics3dSystemImpl::onCharacterDestroyed>(*this);
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
        auto& rigidBody = registry.get<RigidBody3d>(entity);
        rigidBody.getImpl().init(rigidBody, *this);
    }

    void Physics3dSystemImpl::onRigidbodyDestroyed(EntityRegistry& registry, Entity entity)
    {
        auto& rigidBody = registry.get<RigidBody3d>(entity).getImpl();
        _rigidBodies.erase(rigidBody.getBodyId());
        rigidBody.shutdown();
    }

    void Physics3dSystemImpl::onCharacterConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& character = registry.get<CharacterController>(entity);
        character.getImpl().init(character, *this);
    }

    void Physics3dSystemImpl::onCharacterDestroyed(EntityRegistry& registry, Entity entity)
    {
        auto& character = registry.get<CharacterController>(entity).getImpl();
        character.shutdown();
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
        processPendingCollisionEvents();
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
            auto charCtrls = registry.view<CharacterController>();
            for (auto [entity, charCtrl] : charCtrls.each())
            {
                charCtrl.getImpl().update(entity, deltaTime);
            }
        }
    }

    void Physics3dSystemImpl::addUpdater(IPhysics3dUpdater& updater) noexcept
    {
        JoltUtils::addRefVector(_updaters, updater);
    }
    
    bool Physics3dSystemImpl::removeUpdater(IPhysics3dUpdater& updater) noexcept
    {
        return JoltUtils::removeRefVector(_updaters, updater);
    }

    void Physics3dSystemImpl::addListener(IPhysics3dCollisionListener& listener) noexcept
    {
        JoltUtils::addRefVector(_listeners, listener);
    }

    bool Physics3dSystemImpl::removeListener(IPhysics3dCollisionListener& listener) noexcept
    {
        return JoltUtils::removeRefVector(_listeners, listener);
    }

    const Physics3dSystemImpl::Config& Physics3dSystemImpl::getConfig() const noexcept
    {
        return _config;
    }

    OptionalRef<Scene> Physics3dSystemImpl::getScene() const noexcept
    {
        return _scene;
    }

    JPH::PhysicsSystem& Physics3dSystemImpl::getJolt() noexcept
    {
        return *_system;
    }

    JPH::BodyInterface& Physics3dSystemImpl::getBodyInterface() noexcept
    {
        return _system->GetBodyInterface();
    }

    JoltTempAllocator& Physics3dSystemImpl::getTempAllocator() noexcept
    {
        return _alloc;
    }

    glm::vec3 Physics3dSystemImpl::getGravity() noexcept
    {
        return JoltUtils::convert(_system->GetGravity());
    }

    void Physics3dSystemImpl::onBodyCreated(RigidBody3d& rigidBody) noexcept
    {
        std::lock_guard<std::mutex> guard(_rigidBodiesMutex);
        _rigidBodies.emplace(rigidBody.getImpl().getBodyId(), rigidBody);
    }

    OptionalRef<RigidBody3d> Physics3dSystemImpl::getRigidBody(const JPH::BodyID& bodyId) const noexcept
    {
        std::lock_guard<std::mutex> guard(_rigidBodiesMutex);
        auto itr = _rigidBodies.find(bodyId);
        if (itr == _rigidBodies.end())
        {
            return nullptr;
        }
        return itr->second;
    }

    void Physics3dSystemImpl::processPendingCollisionEvents()
    {
        while (!_pendingCollisionEvents.empty())
        {
            auto& ev = _pendingCollisionEvents.front();
            switch (ev.type)
            {
            case CollisionEventType::Enter:
                onCollisionEnter(ev.rigidBody1, ev.rigidBody2, ev.collision);
                break;
            case CollisionEventType::Stay:
                onCollisionStay(ev.rigidBody1, ev.rigidBody2, ev.collision);
                break;
            case CollisionEventType::Exit:
                onCollisionExit(ev.rigidBody1, ev.rigidBody2);
                break;
            }
            _pendingCollisionEvents.pop_front();
        }
    }

    Physics3dCollision Physics3dSystemImpl::createCollision(const JPH::ContactManifold& manifold) noexcept
    {
        Physics3dCollision collision;
        collision.normal = JoltUtils::convert(manifold.mWorldSpaceNormal);
        for (size_t i = 0; i < manifold.mRelativeContactPointsOn1.size(); i++)
        {
            auto p = manifold.GetWorldSpaceContactPointOn1(i);
            collision.contacts.emplace_back(JoltUtils::convert(p));
        }
        return collision;
    }

    void Physics3dSystemImpl::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings)
    {
        // TODO: thread safety on _rigidBodies;
        auto rb1 = getRigidBody(body1.GetID());
        if (!rb1)
        {
            return;
        }
        auto rb2 = getRigidBody(body2.GetID());
        if (!rb2)
        {
            return;
        }

        // queue events to main thread
        auto collision = createCollision(manifold);
        std::lock_guard<std::mutex> guard(_collisionEventsMutex);
        auto evType = collision.contacts.size() <= 1 ? CollisionEventType::Enter : CollisionEventType::Stay;
        _pendingCollisionEvents.emplace_back(evType, rb1.value(), rb2.value(), collision);
    }

    void Physics3dSystemImpl::OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings)
    {
        auto rb1 = getRigidBody(body1.GetID());
        if (!rb1)
        {
            return;
        }
        auto rb2 = getRigidBody(body2.GetID());
        if (!rb2)
        {
            return;
        }
        auto collision = createCollision(manifold);
        std::lock_guard<std::mutex> guard(_collisionEventsMutex);
        _pendingCollisionEvents.emplace_back(CollisionEventType::Stay, rb1.value(), rb2.value(), collision);
    }

    void Physics3dSystemImpl::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
    {
        auto rb1 = getRigidBody(inSubShapePair.GetBody1ID());
        if (!rb1)
        {
            return;
        }
        auto rb2 = getRigidBody(inSubShapePair.GetBody2ID());
        if (!rb2)
        {
            return;
        }
        _pendingCollisionEvents.emplace_back(CollisionEventType::Exit, rb1.value(), rb2.value());
    }

    void Physics3dSystemImpl::onCollisionEnter(RigidBody3d& rigidBody1, RigidBody3d& rigidBody2, const Collision& collision)
    {
        rigidBody1.getImpl().onCollisionEnter(rigidBody2, collision);
        rigidBody2.getImpl().onCollisionEnter(rigidBody1, collision);
        for (auto& listener : _listeners)
        {
            listener->onCollisionEnter(rigidBody1, rigidBody2, collision);
        }
        Physics3dCollision collision2(collision);
    }

    void Physics3dSystemImpl::onCollisionStay(RigidBody3d& rigidBody1, RigidBody3d& rigidBody2, const Collision& collision)
    {
        rigidBody1.getImpl().onCollisionStay(rigidBody2, collision);
        rigidBody2.getImpl().onCollisionStay(rigidBody1, collision);
        for (auto& listener : _listeners)
        {
            listener->onCollisionStay(rigidBody1, rigidBody2, collision);
        }
    }

    void Physics3dSystemImpl::onCollisionExit(RigidBody3d& rigidBody1, RigidBody3d& rigidBody2)
    {
        rigidBody1.getImpl().onCollisionExit(rigidBody2);
        rigidBody2.getImpl().onCollisionExit(rigidBody1);
        for (auto& listener : _listeners)
        {
            listener->onCollisionExit(rigidBody1, rigidBody2);
        }
    }
    
    Physics3dSystem::Physics3dSystem(bx::AllocatorI& alloc, const Config& config) noexcept
        : _impl(std::make_unique<Physics3dSystemImpl>(alloc, config))
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

    void Physics3dSystem::addListener(IPhysics3dCollisionListener& listener) noexcept
    {
        _impl->addListener(listener);
    }

    bool Physics3dSystem::removeListener(IPhysics3dCollisionListener& listener) noexcept
    {
        return _impl->removeListener(listener);
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

    void RigidBody3dImpl::init(RigidBody3d& rigidBody, Physics3dSystemImpl& system) noexcept
    {
        if (_system)
        {
            shutdown();
        }
        _rigidBody = rigidBody;
        _system = system;
    }

    void RigidBody3dImpl::shutdown()
    {
        if (!_system)
        {
            return;
        }
        if (!_bodyId.IsInvalid())
        {
            auto& iface = _system->getBodyInterface();
            iface.RemoveBody(_bodyId);
            iface.DestroyBody(_bodyId);
            _bodyId = JPH::BodyID();
        }
        _system.reset();
        _rigidBody.reset();
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

        auto shape = JoltUtils::convert(_shape);
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

    bool RigidBody3dImpl::tryCreateBody(OptionalRef<Transform> trans) noexcept
    {
        if (!_bodyId.IsInvalid())
        {
            return false;
        }
        _bodyId = createBody(trans);
        if (_rigidBody)
        {
            _system->onBodyCreated(_rigidBody.value());
        }
        return true;
    }

    void RigidBody3dImpl::update(Entity entity, float deltaTime)
    {
        if (!_system)
        {
            return;
        }
        auto trans = _system->getScene()->getComponent<Transform>(entity);
        auto& iface = _system->getBodyInterface();
        tryCreateBody(trans);

        if (trans)
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

    const JPH::BodyID& RigidBody3dImpl::getBodyId() const noexcept
    {
        return _bodyId;
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

    void RigidBody3dImpl::setLinearVelocity(const glm::vec3& velocity)
    {
        getBodyInterface()->SetLinearVelocity(_bodyId, JoltUtils::convert(velocity));
    }

    glm::vec3 RigidBody3dImpl::getLinearVelocity()
    {
        return JoltUtils::convert(getBodyInterface()->GetLinearVelocity(_bodyId));
    }

    void RigidBody3dImpl::addTorque(const glm::vec3& torque)
    {
        getBodyInterface()->AddTorque(_bodyId, JoltUtils::convert(torque));
    }

    void RigidBody3dImpl::addForce(const glm::vec3& force)
    {
        getBodyInterface()->AddForce(_bodyId, JoltUtils::convert(force));
    }

    void RigidBody3dImpl::addImpulse(const glm::vec3& impulse)
    {
        getBodyInterface()->AddImpulse(_bodyId, JoltUtils::convert(impulse));
    }

    void RigidBody3dImpl::move(const glm::vec3& pos, const glm::quat& rot, float deltaTime)
    {
        if (_system)
        {
            auto minTime = _system->getConfig().fixedDeltaTime;
            if (deltaTime < minTime)
            {
                deltaTime = minTime;
            }
        }
        getBodyInterface()->MoveKinematic(_bodyId,
            JoltUtils::convert(pos),
            JoltUtils::convert(rot),
            deltaTime);
    }

    void RigidBody3dImpl::movePosition(const glm::vec3& pos, float deltaTime)
    {
        auto iface = getBodyInterface();
        auto rot = iface->GetRotation(_bodyId);
        if (_system)
        {
            auto minTime = _system->getConfig().fixedDeltaTime;
            if (deltaTime < minTime)
            {
                deltaTime = minTime;
            }
        }
        iface->MoveKinematic(_bodyId,
            JoltUtils::convert(pos),
            rot, deltaTime);
    }

    void RigidBody3dImpl::addListener(IPhysics3dCollisionListener& listener) noexcept
    {
        JoltUtils::addRefVector(_listeners, listener);
    }

    bool RigidBody3dImpl::removeListener(IPhysics3dCollisionListener& listener) noexcept
    {
        return JoltUtils::removeRefVector(_listeners, listener);
    }

    void RigidBody3dImpl::onCollisionEnter(RigidBody3d& other, const Physics3dCollision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionEnter(_rigidBody.value(), other, collision);
        }
    }

    void RigidBody3dImpl::onCollisionStay(RigidBody3d& other, const Physics3dCollision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionStay(_rigidBody.value(), other, collision);
        }
    }

    void RigidBody3dImpl::onCollisionExit(RigidBody3d& other)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionExit(_rigidBody.value(), other);
        }
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

    RigidBody3d& RigidBody3d::setLinearVelocity(const glm::vec3& velocity)
    {
        _impl->setLinearVelocity(velocity);
        return *this;
    }

    glm::vec3 RigidBody3d::getLinearVelocity()
    {
        return _impl->getLinearVelocity();
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

    RigidBody3d& RigidBody3d::addImpulse(const glm::vec3& impulse)
    {
        _impl->addImpulse(impulse);
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

    RigidBody3d& RigidBody3d::addListener(IPhysics3dCollisionListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool RigidBody3d::removeListener(IPhysics3dCollisionListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }
}