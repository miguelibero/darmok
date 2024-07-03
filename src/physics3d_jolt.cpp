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

#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Math/Float2.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>

namespace darmok::physics3d
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

    Color JoltUtils::convert(const JPH::Color& v) noexcept
    {
        return Color(v.r, v.g, v.b, v.a);
    }

    glm::vec2 JoltUtils::convert(const JPH::Float2& v) noexcept
    {
        return glm::vec2(v.x, v.y);
    }

    glm::vec3 JoltUtils::convert(const JPH::Float3& v) noexcept
    {
        return glm::vec3(v.x, v.y, v.z);
    }

    glm::vec4 JoltUtils::convert(const JPH::Float4& v) noexcept
    {
        return glm::vec4(v.x, v.y, v.z, v.w);
    }

    RaycastHit JoltUtils::convert(const JPH::RayCastResult& result, PhysicsBody& rb) noexcept
    {
        // TODO: check how to get other RaycastHit properties
        // https://docs.unity3d.com/ScriptReference/RaycastHit.html
        return RaycastHit{ rb, result.mFraction };
    }

    glm::vec3 JoltUtils::getOrigin(const Shape& shape) noexcept
    {
        if (auto cube = std::get_if<Cube>(&shape))
        {
            return cube->origin;
        }
        else if (auto sphere = std::get_if<Sphere>(&shape))
        {
            return sphere->origin;
        }
        else if (auto caps = std::get_if<Capsule>(&shape))
        {
            return caps->origin;
        }
        return glm::vec3(0);
    }

    glm::mat4 JoltUtils::convert(const JPH::Mat44& jmat, const Transform& trans) noexcept
    {
        auto mat = convert(jmat);
        auto parent = trans.getParent();
        if (parent)
        {
            // TODO check this
            mat = parent->getWorldInverse() * mat;
        }
        return mat;
    }

    JoltTransform JoltUtils::convert(OptionalRef<const Transform> trans, OptionalRef<const Transform> root)
    {
        glm::mat4 mat;
        if (trans)
        {
            mat = trans->getWorldMatrix();
        }
        if (root)
        {
            mat = root->getWorldInverse() * mat;
        }

        glm::vec3 pos(0);
        glm::quat rot(1, 0, 0, 0);
        glm::vec3 scale(1);
        Math::decompose(mat, pos, rot, scale);
        static const int epsilonFactor = 100;
        if (!Math::almostEqual(scale.x, scale.y, epsilonFactor) || !Math::almostEqual(scale.x, scale.z, epsilonFactor))
        {
            throw std::runtime_error("non-uniform scale not supported");
        }
        return JoltTransform{ convert(pos), convert(rot), scale.x };
    }

    static JPH::Ref<JPH::Shape> getShape(JPH::ShapeSettings& settings)
    {
        auto result = settings.Create();
        if (result.HasError())
        {
            throw std::runtime_error(result.GetError().c_str());
        }
        return result.Get();
    }

    JPH::ShapeRefC joltGetOffsetShape(JPH::ShapeSettings& settings, const glm::vec3& offset)
    {
        auto shape = getShape(settings);
        if (offset == glm::vec3(0))
        {
            return shape;
        }
        JPH::RotatedTranslatedShapeSettings offsetSettings(JoltUtils::convert(offset), JPH::Quat::sIdentity(), shape);
        return getShape(offsetSettings);
    }

    JPH::ShapeRefC JoltUtils::convert(const Shape& shape, float scale)
    {
        if (auto cubePtr = std::get_if<Cube>(&shape))
        {
            auto cube = *cubePtr * scale;
            JPH::BoxShapeSettings settings(JoltUtils::convertSize(cube.size * 0.5F));
            return joltGetOffsetShape(settings, cube.origin);
        }
        else if (auto spherePtr = std::get_if<Sphere>(&shape))
        {
            auto sphere = *spherePtr * scale;
            JPH::SphereShapeSettings settings(sphere.radius);
            return joltGetOffsetShape(settings, sphere.origin);
        }
        else if (auto capsPtr = std::get_if<Capsule>(&shape))
        {
            auto caps = *capsPtr * scale;
            JPH::CapsuleShapeSettings settings(caps.cylinderHeight * 0.5, caps.radius);
            return joltGetOffsetShape(settings, caps.origin);
        }
        return nullptr;
    }

    JoltBroadPhaseLayerInterface::JoltBroadPhaseLayerInterface(const std::vector<std::string>& layers) noexcept
        : _layers(layers)
    {
    }

    JPH::uint JoltBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const noexcept
    {
        return _layers.size();
    }

    JPH::BroadPhaseLayer JoltBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer objLayer) const noexcept
    {
        return JPH::BroadPhaseLayer((JPH::BroadPhaseLayer::Type)objLayer);
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    const char* JoltBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer bpLayer) const noexcept
    {
        auto idx = bpLayer.GetValue();
        if (idx < 0 || idx >= _layers.size())
        {
            JPH_ASSERT(false);
            return "Invalid";
        }
        return _layers[idx].c_str();
    }
#endif

    bool JoltObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const noexcept
    {
        return layer1 == layer2.GetValue();
    }

    bool JoltObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const noexcept
    {
        return object1 == object2;
    }

    JoltObjectLayerMaskFilter::JoltObjectLayerMaskFilter(uint16_t layerMask) noexcept
        : _layerMask(layerMask)
    {
    }

    bool JoltObjectLayerMaskFilter::ShouldCollide(JPH::ObjectLayer layer) const
    {
        return _layerMask | layer;
    }

    JoltTempAllocator::JoltTempAllocator(OptionalRef<bx::AllocatorI> alloc) noexcept
        : _alloc(alloc)
    {
    }

    void* JoltTempAllocator::Allocate(JPH::uint size) noexcept
    {
        if (!_alloc)
        {
            return malloc(size);
        }
        return bx::alloc(_alloc.ptr(), size);
    }

    void JoltTempAllocator::Free(void* addr, JPH::uint size) noexcept
    {
        if (!_alloc)
        {
            return free(addr);
        }
        bx::free(_alloc.ptr(), addr);
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

    PhysicsSystemImpl::PhysicsSystemImpl(const Config& config, OptionalRef<bx::AllocatorI> alloc) noexcept
        : _alloc(alloc)
        , _config(config)
        , _deltaTimeRest(0.F)
        , _broadPhaseLayer(config.layers)
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace = joltTraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = joltAssertFailed;)
    }

    void PhysicsSystemImpl::init(Scene& scene, App& app) noexcept
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
        registry.on_construct<PhysicsBody>().connect<&PhysicsSystemImpl::onRigidbodyConstructed>(*this);
        registry.on_destroy<PhysicsBody>().connect< &PhysicsSystemImpl::onRigidbodyDestroyed>(*this);
        registry.on_construct<CharacterController>().connect<&PhysicsSystemImpl::onCharacterConstructed>(*this);
        registry.on_destroy<CharacterController>().connect< &PhysicsSystemImpl::onCharacterDestroyed>(*this);
    }

    void PhysicsSystemImpl::shutdown() noexcept
    {
        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            registry.on_construct<PhysicsBody>().disconnect<&PhysicsSystemImpl::onRigidbodyConstructed>(*this);
            registry.on_destroy<PhysicsBody>().disconnect< &PhysicsSystemImpl::onRigidbodyDestroyed>(*this);
            registry.on_construct<CharacterController>().disconnect<&PhysicsSystemImpl::onCharacterConstructed>(*this);
            registry.on_destroy<CharacterController>().disconnect< &PhysicsSystemImpl::onCharacterDestroyed>(*this);

            auto bodies = registry.view<PhysicsBody>();
            for (auto [entity, body] : bodies.each())
            {
                body.getImpl().shutdown(true);
            }

            registry.erase<PhysicsBody>(bodies.begin(), bodies.end());
            auto charCtrls = registry.view<CharacterController>();
            registry.erase<CharacterController>(charCtrls.begin(), charCtrls.end());
        }

        _system.reset();
        _threadPool.reset();
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
        _scene.reset();
    }

    void PhysicsSystemImpl::onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& body = registry.get<PhysicsBody>(entity);
        body.getImpl().init(body, *this);
    }

    void PhysicsSystemImpl::onRigidbodyDestroyed(EntityRegistry& registry, Entity entity)
    {
        auto& body = registry.get<PhysicsBody>(entity).getImpl();
        body.shutdown();
    }

    void PhysicsSystemImpl::onCharacterConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& character = registry.get<CharacterController>(entity);
        character.getImpl().init(character, *this);
    }

    void PhysicsSystemImpl::onCharacterDestroyed(EntityRegistry& registry, Entity entity)
    {
        auto& character = registry.get<CharacterController>(entity).getImpl();
        character.shutdown();
    }

    std::string PhysicsSystemImpl::getUpdateErrorString(JPH::EPhysicsUpdateError err) noexcept
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
    
    void PhysicsSystemImpl::update(float deltaTime)
    {
        if (!_system)
        {
            return;
        }
        _deltaTimeRest += deltaTime;
        auto fdt = _config.fixedDeltaTime;
        // try to do the same order as unity
        // https://docs.unity3d.com/Manual/ExecutionOrder.html
        while (_deltaTimeRest > fdt)
        {
            for (auto& updater : _updaters)
            {
                updater->fixedUpdate(fdt);
            }

            // TODO: skeletal animations here? or maybe with an updater
            // probably important for ragdolls or inverse kinematics

            auto err = _system->Update(fdt, _config.collisionSteps, &_alloc, _threadPool.get());
            _deltaTimeRest -= fdt;
            if (err != JPH::EPhysicsUpdateError::None)
            {
                throw std::runtime_error(std::string("physics update error ") + getUpdateErrorString(err));
            }

            processPendingCollisionEvents();
        }

        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            auto rigidBodies = registry.view<PhysicsBody>();
            for (auto [entity, body] : rigidBodies.each())
            {
                body.getImpl().update(entity, deltaTime);
            }
            auto charCtrls = registry.view<CharacterController>();
            for (auto [entity, charCtrl] : charCtrls.each())
            {
                charCtrl.getImpl().update(entity, deltaTime);
            }
        }
    }

    void PhysicsSystemImpl::addUpdater(IPhysicsUpdater& updater) noexcept
    {
        JoltUtils::addRefVector(_updaters, updater);
    }
    
    bool PhysicsSystemImpl::removeUpdater(IPhysicsUpdater& updater) noexcept
    {
        return JoltUtils::removeRefVector(_updaters, updater);
    }

    void PhysicsSystemImpl::addListener(ICollisionListener& listener) noexcept
    {
        JoltUtils::addRefVector(_listeners, listener);
    }

    bool PhysicsSystemImpl::removeListener(ICollisionListener& listener) noexcept
    {
        return JoltUtils::removeRefVector(_listeners, listener);
    }

    const PhysicsSystemImpl::Config& PhysicsSystemImpl::getConfig() const noexcept
    {
        return _config;
    }

    OptionalRef<Scene> PhysicsSystemImpl::getScene() const noexcept
    {
        return _scene;
    }

    OptionalRef<JPH::PhysicsSystem> PhysicsSystemImpl::getJolt() noexcept
    {
        return _system != nullptr ? OptionalRef<JPH::PhysicsSystem>(*_system) : nullptr;
    }

    OptionalRef<const JPH::PhysicsSystem> PhysicsSystemImpl::getJolt() const noexcept
    {
        return _system != nullptr ? OptionalRef<const JPH::PhysicsSystem>(*_system) : nullptr;
    }

    JPH::BodyInterface& PhysicsSystemImpl::getBodyInterface() const noexcept
    {
        return _system->GetBodyInterface();
    }

    JoltTempAllocator& PhysicsSystemImpl::getTempAllocator() noexcept
    {
        return _alloc;
    }

    glm::vec3 PhysicsSystemImpl::getGravity() noexcept
    {
        return JoltUtils::convert(_system->GetGravity());
    }

    void PhysicsSystemImpl::setRootTransform(OptionalRef<Transform> root) noexcept
    {
        _root = root;
    }

    OptionalRef<Transform> PhysicsSystemImpl::getRootTransform() noexcept
    {
        return _root;
    }

    OptionalRef<PhysicsBody> PhysicsSystemImpl::getPhysicsBody(const JPH::BodyID& bodyId) const noexcept
    {
        auto userData = getBodyInterface().GetUserData(bodyId);
        if (userData == 0)
        {
            return nullptr;
        }
        return *(PhysicsBody*)userData;
    }

    OptionalRef<PhysicsBody> PhysicsSystemImpl::getPhysicsBody(const JPH::Body& body) noexcept
    {
        auto userData = body.GetUserData();
        if (userData == 0)
        {
            return nullptr;
        }
        return (PhysicsBody*)userData;
    }

    std::optional<RaycastHit> PhysicsSystemImpl::raycast(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        if (!_system)
        {
            return std::nullopt;
        }
        JPH::RRayCast rc(JoltUtils::convert(ray.origin), JoltUtils::convert(ray.direction) * maxDistance);
        JPH::BroadPhaseLayerFilter bpLayerFilter;
        JoltObjectLayerMaskFilter objLayerFilter(layerMask);
        JPH::RayCastResult result;

        if (!_system->GetNarrowPhaseQuery().CastRay(rc, result, bpLayerFilter, objLayerFilter))
        {
            return std::nullopt;
        }
        auto rb = getPhysicsBody(result.mBodyID);
        if (!rb)
        {
            return std::nullopt;
        }
        return JoltUtils::convert(result, rb.value());
    }

    std::vector<RaycastHit> PhysicsSystemImpl::raycastAll(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        std::vector<RaycastHit> hits;
        if (!_system)
        {
            return hits;
        }

        JPH::RRayCast rc(JoltUtils::convert(ray.origin), JoltUtils::convert(ray.direction) * maxDistance);
        JPH::BroadPhaseLayerFilter bpLayerFilter;
        JoltObjectLayerMaskFilter objLayerFilter(layerMask);

        JPH::RayCastSettings settings;
        JoltVectorCastRayCollector collector;

        _system->GetNarrowPhaseQuery().CastRay(rc, settings, collector, bpLayerFilter, objLayerFilter);
        for (auto& result : collector.getHits())
        {
            auto rb = getPhysicsBody(result.mBodyID);
            if (rb)
            {
                hits.push_back(JoltUtils::convert(result, rb.value()));
            }
        }

        return hits;
    }

    void PhysicsSystemImpl::processPendingCollisionEvents()
    {
        std::lock_guard<std::mutex> guard(_collisionMutex);
        while (!_pendingCollisionEvents.empty())
        {
            CollisionEvent ev = _pendingCollisionEvents.front();
            _pendingCollisionEvents.pop_front();

            auto rb1 = getPhysicsBody(ev.bodyID1);
            if (!rb1)
            {
                continue;
            }
            auto rb2 = getPhysicsBody(ev.bodyID2);
            if (!rb2)
            {
                continue;
            }
            switch (ev.type)
            {
            case CollisionEventType::Enter:
                onCollisionEnter(rb1.value(), rb2.value(), ev.collision);
                break;
            case CollisionEventType::Stay:
                onCollisionStay(rb1.value(), rb2.value(), ev.collision);
                break;
            case CollisionEventType::Exit:
                onCollisionExit(rb1.value(), rb2.value());
                break;
            }
        }
    }

    Collision PhysicsSystemImpl::createCollision(const JPH::ContactManifold& manifold) noexcept
    {
        Collision collision;
        collision.normal = JoltUtils::convert(manifold.mWorldSpaceNormal);
        for (size_t i = 0; i < manifold.mRelativeContactPointsOn1.size(); i++)
        {
            auto p = manifold.GetWorldSpaceContactPointOn1(i);
            collision.contacts.emplace_back(JoltUtils::convert(p));
        }
        return collision;
    }

    void PhysicsSystemImpl::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings)
    {
        auto collision = createCollision(manifold);
        std::lock_guard<std::mutex> guard(_collisionMutex);
        _pendingCollisionEvents.emplace_back(CollisionEventType::Enter, body1.GetID(), body2.GetID(), collision);
    }

    void PhysicsSystemImpl::OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings)
    {
        auto collision = createCollision(manifold);
        std::lock_guard<std::mutex> guard(_collisionMutex);
        _pendingCollisionEvents.emplace_back(CollisionEventType::Stay, body1.GetID(), body2.GetID(), collision);
    }

    void PhysicsSystemImpl::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
    {
        std::lock_guard<std::mutex> guard(_collisionMutex);
        _pendingCollisionEvents.emplace_back(CollisionEventType::Exit, inSubShapePair.GetBody1ID(), inSubShapePair.GetBody2ID());
    }

    void PhysicsSystemImpl::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        body1.getImpl().onCollisionEnter(body2, collision);
        body2.getImpl().onCollisionEnter(body1, collision);
        for (auto& listener : _listeners)
        {
            listener->onCollisionEnter(body1, body2, collision);
        }
        Collision collision2(collision);
    }

    void PhysicsSystemImpl::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        body1.getImpl().onCollisionStay(body2, collision);
        body2.getImpl().onCollisionStay(body1, collision);
        for (auto& listener : _listeners)
        {
            listener->onCollisionStay(body1, body2, collision);
        }
    }

    void PhysicsSystemImpl::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        body1.getImpl().onCollisionExit(body2);
        body2.getImpl().onCollisionExit(body1);
        for (auto& listener : _listeners)
        {
            listener->onCollisionExit(body1, body2);
        }
    }
    
    PhysicsSystem::PhysicsSystem(const Config& config, bx::AllocatorI& alloc) noexcept
        : _impl(std::make_unique<PhysicsSystemImpl>(config, alloc))
    {
    }

    PhysicsSystem::PhysicsSystem(const Config& config) noexcept
        : _impl(std::make_unique<PhysicsSystemImpl>(config))
    {
    }

    PhysicsSystem::PhysicsSystem(bx::AllocatorI& alloc) noexcept
        : _impl(std::make_unique<PhysicsSystemImpl>(Config{}, alloc))
    {
    }

    PhysicsSystem::~PhysicsSystem() noexcept
    {
        // implemented to do forward declaration of impl
    }

    PhysicsSystemImpl& PhysicsSystem::getImpl() noexcept
    {
        return *_impl;
    }

    const PhysicsSystemImpl& PhysicsSystem::getImpl() const noexcept
    {
        return *_impl;
    }

    PhysicsSystem& PhysicsSystem::setRootTransform(OptionalRef<Transform> root) noexcept
    {
        _impl->setRootTransform(root);
        return *this;
    }

    OptionalRef<Transform> PhysicsSystem::getRootTransform() noexcept
    {
        return _impl->getRootTransform();
    }
    
    void PhysicsSystem::init(Scene& scene, App& app) noexcept
    {
        _impl->init(scene, app);
    }
    
    void PhysicsSystem::shutdown() noexcept
    {
        _impl->shutdown();
    }
    
    void PhysicsSystem::update(float deltaTime) noexcept
    {
        _impl->update(deltaTime);
    }
    
    void PhysicsSystem::addUpdater(IPhysicsUpdater& updater) noexcept
    {
        _impl->addUpdater(updater);
    }
    
    bool PhysicsSystem::removeUpdater(IPhysicsUpdater& updater) noexcept
    {
        return _impl->removeUpdater(updater);
    }

    void PhysicsSystem::addListener(ICollisionListener& listener) noexcept
    {
        _impl->addListener(listener);
    }

    bool PhysicsSystem::removeListener(ICollisionListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }

    std::optional<RaycastHit> PhysicsSystem::raycast(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _impl->raycast(ray, maxDistance, layerMask);
    }

    std::vector<RaycastHit> PhysicsSystem::raycastAll(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _impl->raycastAll(ray, maxDistance, layerMask);
    }

    PhysicsBodyImpl::PhysicsBodyImpl(const Config& config) noexcept
        : _config(config)
    {
    }

    PhysicsBodyImpl::PhysicsBodyImpl(const CharacterConfig& config) noexcept
        : _characterConfig(config)
    {
    }

    PhysicsBodyImpl::~PhysicsBodyImpl()
    {
        shutdown();
    }

    void PhysicsBodyImpl::init(PhysicsBody& body, PhysicsSystemImpl& system) noexcept
    {
        if (_system)
        {
            shutdown();
        }
        _body = body;
        _system = system;
    }

    void PhysicsBodyImpl::shutdown(bool systemShutdown)
    {
        if (!_system)
        {
            return;
        }
        auto& iface = _system->getBodyInterface();
        if (!_bodyId.IsInvalid())
        {
            iface.RemoveBody(_bodyId);
        }
        if (_character)
        {
            _character = nullptr;
        }
        else if(!_bodyId.IsInvalid())
        {
            iface.DestroyBody(_bodyId);
        }
        _bodyId = JPH::BodyID();
        _system.reset();
        _body.reset();
    }

    OptionalRef<JPH::BodyInterface> PhysicsBodyImpl::getBodyInterface() const noexcept
    {
        if (!_system)
        {
            return nullptr;
        }
        return _system->getBodyInterface();
    }

    JPH::BodyID PhysicsBodyImpl::createCharacter(const JoltTransform& trans)
    {
        if (!_characterConfig)
        {
            return {};
        }
        auto joltSystem = _system->getJolt();
        if (!joltSystem)
        {
            return {};
        }

        auto& config = _characterConfig.value();
        JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings();
        settings->mMaxSlopeAngle = config.maxSlopeAngle;
        settings->mShape = JoltUtils::convert(config.shape, trans.scale);
        settings->mFriction = config.friction;
        settings->mSupportingVolume = JPH::Plane(JoltUtils::convert(config.supportingPlane.normal), -config.supportingPlane.constant);
        settings->mMass = config.mass;
        settings->mGravityFactor = config.gravityFactor;
        settings->mUp = JoltUtils::convert(config.up);
        settings->mLayer = config.layer;
        auto userData = (uint64_t)_body.ptr();
        _character = new JPH::Character(settings, trans.position, trans.rotation, userData, joltSystem.ptr());
        _character->AddToPhysicsSystem();
        return _character->GetBodyID();
    }

    JPH::BodyID PhysicsBodyImpl::createBody(const JoltTransform& trans)
    {
        if (!_system)
        {
            return {};
        }

        JPH::EMotionType joltMotion = JPH::EMotionType::Dynamic;
        JPH::ObjectLayer objLayer = _config.layer;
        auto activation = JPH::EActivation::Activate;
        switch (_config.motion)
        {
        case PhysicsBodyMotionType::Kinematic:
            joltMotion = JPH::EMotionType::Kinematic;
            break;
        case PhysicsBodyMotionType::Static:
            joltMotion = JPH::EMotionType::Static;
            break;
        }

        auto shape = JoltUtils::convert(_config.shape, trans.scale);
        JPH::BodyCreationSettings settings(shape, trans.position, trans.rotation,
            joltMotion, objLayer);
        settings.mGravityFactor = _config.gravityFactor;
        settings.mFriction = _config.friction;
        settings.mUserData = (uint64_t)_body.ptr();
        settings.mObjectLayer = _config.layer;
        settings.mIsSensor = _config.trigger;
        if (_config.mass)
        {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
            settings.mMassPropertiesOverride.mMass = _config.mass.value();
        }
        return _system->getBodyInterface().CreateAndAddBody(settings, activation);
    }

    bool PhysicsBodyImpl::tryCreateBody(OptionalRef<Transform> trans)
    {
        if (!_bodyId.IsInvalid())
        {
            return false;
        }

        OptionalRef<const Transform> root;
        if (_system)
        {
            root = _system->getRootTransform();
        }
        auto joltTrans = JoltUtils::convert(trans, root);
        if (!_characterConfig && _config.motion == MotionType::Character)
        {
            _characterConfig.emplace().load(_config);
        }
        if (_characterConfig)
        {
            _bodyId = createCharacter(joltTrans);
        }
        else
        {
            _bodyId = createBody(joltTrans);
        }
        return true;
    }

    void PhysicsBodyImpl::update(Entity entity, float deltaTime)
    {
        if (!_system)
        {
            return;
        }
        auto trans = _system->getScene()->getComponent<Transform>(entity);
        tryCreateBody(trans);
        if (_character)
        {
            _character->PostSimulation(_characterConfig->maxSeparationDistance, false);
        }
        if (trans)
        {
            auto jmat = _system->getBodyInterface().GetWorldTransform(_bodyId);
            auto mat = JoltUtils::convert(jmat, trans.value());
            trans->setLocalMatrix(mat);
        }
    }

    const PhysicsBodyImpl::Shape& PhysicsBodyImpl::getShape() const noexcept
    {
        if (_characterConfig)
        {
            return _characterConfig->shape;
        }
        return _config.shape;
    }

    PhysicsBodyImpl::MotionType PhysicsBodyImpl::getMotionType() const noexcept
    {
        if (_characterConfig)
        {
            return PhysicsBodyImpl::MotionType::Dynamic;
        }
        return _config.motion;
    }

    const JPH::BodyID& PhysicsBodyImpl::getBodyId() const noexcept
    {
        return _bodyId;
    }

    void PhysicsBodyImpl::setPosition(const glm::vec3& pos)
    {
        auto jpos = JoltUtils::convert(pos);
        getBodyInterface()->SetPosition(_bodyId, jpos, JPH::EActivation::Activate);
    }

    glm::vec3 PhysicsBodyImpl::getPosition()
    {
        return JoltUtils::convert(getBodyInterface()->GetPosition(_bodyId));
    }

    void PhysicsBodyImpl::setRotation(const glm::quat& rot)
    {
        getBodyInterface()->SetRotation(_bodyId, JoltUtils::convert(rot), JPH::EActivation::Activate);
    }

    glm::quat PhysicsBodyImpl::getRotation()
    {
        return JoltUtils::convert(getBodyInterface()->GetRotation(_bodyId));
    }

    void PhysicsBodyImpl::setLinearVelocity(const glm::vec3& velocity)
    {
        getBodyInterface()->SetLinearVelocity(_bodyId, JoltUtils::convert(velocity));
    }

    glm::vec3 PhysicsBodyImpl::getLinearVelocity()
    {
        return JoltUtils::convert(getBodyInterface()->GetLinearVelocity(_bodyId));
    }

    void PhysicsBodyImpl::addTorque(const glm::vec3& torque)
    {
        getBodyInterface()->AddTorque(_bodyId, JoltUtils::convert(torque));
    }

    void PhysicsBodyImpl::addForce(const glm::vec3& force)
    {
        getBodyInterface()->AddForce(_bodyId, JoltUtils::convert(force));
    }

    void PhysicsBodyImpl::addImpulse(const glm::vec3& impulse)
    {
        getBodyInterface()->AddImpulse(_bodyId, JoltUtils::convert(impulse));
    }

    void PhysicsBodyImpl::move(const glm::vec3& pos, const glm::quat& rot, float deltaTime)
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

    void PhysicsBodyImpl::movePosition(const glm::vec3& pos, float deltaTime)
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

    void PhysicsBodyImpl::addListener(ICollisionListener& listener) noexcept
    {
        JoltUtils::addRefVector(_listeners, listener);
    }

    bool PhysicsBodyImpl::removeListener(ICollisionListener& listener) noexcept
    {
        return JoltUtils::removeRefVector(_listeners, listener);
    }

    void PhysicsBodyImpl::onCollisionEnter(PhysicsBody& other, const Collision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionEnter(_body.value(), other, collision);
        }
    }

    void PhysicsBodyImpl::onCollisionStay(PhysicsBody& other, const Collision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionStay(_body.value(), other, collision);
        }
    }

    void PhysicsBodyImpl::onCollisionExit(PhysicsBody& other)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionExit(_body.value(), other);
        }
    }

    PhysicsBody::PhysicsBody(const Shape& shape, MotionType motion) noexcept
        : PhysicsBody(Config{ shape, motion })
    {
    }

    PhysicsBody::PhysicsBody(const Config& config) noexcept
        : _impl(std::make_unique<PhysicsBodyImpl>(config))
    {
    }

    PhysicsBody::PhysicsBody(const CharacterConfig& config) noexcept
        : _impl(std::make_unique<PhysicsBodyImpl>(config))
    {
    }

    PhysicsBody::~PhysicsBody() noexcept
    {
        // empty for the impl forward declaration
    }

    const PhysicsBody::Shape& PhysicsBody::getShape() const noexcept
    {
        return _impl->getShape();
    }

    PhysicsBody::MotionType PhysicsBody::getMotionType() const noexcept
    {
        return _impl->getMotionType();
    }

    PhysicsBody& PhysicsBody::setPosition(const glm::vec3& pos)
    {
        _impl->setPosition(pos);
        return *this;
    }

    glm::vec3 PhysicsBody::getPosition()
    {
        return _impl->getPosition();
    }

    PhysicsBody& PhysicsBody::setRotation(const glm::quat& rot)
    {
        _impl->setRotation(rot);
        return *this;
    }

    glm::quat PhysicsBody::getRotation()
    {
        return _impl->getRotation();
    }

    PhysicsBody& PhysicsBody::setLinearVelocity(const glm::vec3& velocity)
    {
        _impl->setLinearVelocity(velocity);
        return *this;
    }

    glm::vec3 PhysicsBody::getLinearVelocity()
    {
        return _impl->getLinearVelocity();
    }

    PhysicsBody& PhysicsBody::addTorque(const glm::vec3& torque)
    {
        _impl->addTorque(torque);
        return *this;
    }

    PhysicsBody& PhysicsBody::addForce(const glm::vec3& force)
    {
        _impl->addForce(force);
        return *this;
    }

    PhysicsBody& PhysicsBody::addImpulse(const glm::vec3& impulse)
    {
        _impl->addImpulse(impulse);
        return *this;
    }

    PhysicsBody& PhysicsBody::move(const glm::vec3& pos, const glm::quat& rot, float deltaTime)
    {
        _impl->move(pos, rot, deltaTime);
        return *this;
    }

    PhysicsBody& PhysicsBody::movePosition(const glm::vec3& pos, float deltaTime)
    {
        _impl->movePosition(pos, deltaTime);
        return *this;
    }

    PhysicsBodyImpl& PhysicsBody::getImpl() noexcept
    {
        return *_impl;
    }

    const PhysicsBodyImpl& PhysicsBody::getImpl() const noexcept
    {
        return *_impl;
    }

    PhysicsBody& PhysicsBody::addListener(ICollisionListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool PhysicsBody::removeListener(ICollisionListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }
}