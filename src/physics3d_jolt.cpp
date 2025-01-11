#include "physics3d_jolt.hpp"
#include "character_jolt.hpp"
#include <darmok/physics3d.hpp>
#include <darmok/transform.hpp>
#include <darmok/math.hpp>
#include <darmok/string.hpp>
#include <darmok/collection.hpp>
#include <darmok/scene_filter.hpp>
#include <bx/allocator.h>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include <thread>
#include <cstdarg>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <utility>

#include <Jolt/Core/Factory.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Math/Float2.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Character/Character.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
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
        auto fv = Math::flipHandedness(glm::normalize(v));
        return JPH::Quat(fv.x, fv.y, fv.z, fv.w);
    }

    glm::quat JoltUtils::convert(const JPH::Quat& v) noexcept
    {
        glm::quat quat(v.GetW(), v.GetX(), v.GetY(), v.GetZ());
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

    JPH::Triangle JoltUtils::convert(const Triangle& v) noexcept
    {
        return JPH::Triangle(convert(v.vertices[0]), convert(v.vertices[1]), convert(v.vertices[2]));
    }

    JPH::TriangleList JoltUtils::convert(const Polygon& v) noexcept
    {
        JPH::TriangleList tris;
        tris.reserve(v.triangles.size());
        for (auto& tri : v.triangles)
        {
            tris.push_back(convert(tri));
        }
        return tris;
    }

    JPH::Plane JoltUtils::convert(const Plane& v) noexcept
    {
        return JPH::Plane(JoltUtils::convert(v.normal), v.distance);
    }

    JPH::AABox JoltUtils::convert(const BoundingBox& v) noexcept
    {
        return JPH::AABox(convert(v.min), convert(v.max));
    }

    BoundingBox JoltUtils::convert(const JPH::AABox& v) noexcept
    {
        return BoundingBox(convert(v.mMin), convert(v.mMax));
    }

    JPH::RRayCast JoltUtils::convert(const Ray& v) noexcept
    {
        return JPH::RRayCast(convert(v.origin), convert(v.direction));
    }

    RaycastHit JoltUtils::convert(const JPH::RayCastResult& result, const Ray& ray, PhysicsBody& rb) noexcept
    {
        // TODO: check how to get other RaycastHit properties
        // https://docs.unity3d.com/ScriptReference/RaycastHit.html
        return RaycastHit{
            .body = rb,
            .factor = result.mFraction,
            .distance = result.mFraction * glm::length(ray.direction),
            .point = ray.origin + result.mFraction * ray.direction
        };
    }

    Expected<JoltTransform, std::string> JoltUtils::convertTransform(const glm::mat4& mat) noexcept
    {
        glm::vec3 pos(0);
        glm::quat rot(1, 0, 0, 0);
        glm::vec3 scale(1);
        Math::decompose(mat, pos, rot, scale);
        static const int epsilonFactor = 100;
        if (!Math::almostEqual(scale.x, scale.y, epsilonFactor) || !Math::almostEqual(scale.x, scale.z, epsilonFactor))
        {
            return Unexpected<std::string>("non-uniform scale not supported");
        }
        return JoltTransform{ JoltUtils::convert(pos), JoltUtils::convert(rot), scale.x };
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

    float JoltUtils::getConvexRadius(const glm::vec3& size) noexcept
    {
        // https://github.com/jrouwe/JoltPhysics/blob/master/Jolt/Physics/Collision/Shape/BoxShape.cpp#L64
        auto radius = glm::compMin(size) * 0.5f;
        if (radius <= JPH::cDefaultConvexRadius)
        {
            return radius * 0.999F;
        }
        return JPH::cDefaultConvexRadius;
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
        std::optional<Cube> optCube;
        if (auto cubePtr = std::get_if<Cube>(&shape))
        {
            optCube = *cubePtr;
        }
        else if (auto bbPtr = std::get_if<BoundingBox>(&shape))
        {
            optCube = Cube(*bbPtr);
        }
        if (optCube)
        {
            auto cube = optCube.value() * scale;
            auto radius = JoltUtils::getConvexRadius(cube.size);
            JPH::BoxShapeSettings settings(JoltUtils::convertSize(cube.size * 0.5F), radius);
            return joltGetOffsetShape(settings, cube.origin);
        }

        if (auto spherePtr = std::get_if<Sphere>(&shape))
        {
            auto sphere = *spherePtr * scale;
            JPH::SphereShapeSettings settings(sphere.radius);
            return joltGetOffsetShape(settings, sphere.origin);
        }
        if (auto capsPtr = std::get_if<Capsule>(&shape))
        {
            auto caps = *capsPtr * scale;
            JPH::CapsuleShapeSettings settings(caps.cylinderHeight * 0.5, caps.radius);
            return joltGetOffsetShape(settings, caps.origin);
        }
        if (auto polyPtr = std::get_if<Polygon>(&shape))
        {
            auto poly = *polyPtr * scale;
            auto tris = JoltUtils::convert(poly);
            JPH::MeshShapeSettings settings(tris);
            return joltGetOffsetShape(settings, poly.origin);
        }
        return nullptr;
    }

    const std::string JoltJobSystemTaskflow::_prefix = "JoltPhysics";

    JoltJobSystemTaskflow::JoltJobSystemTaskflow() noexcept
        : _taskflow(_prefix)
    {
    }

    JoltJobSystemTaskflow::~JoltJobSystemTaskflow()
    {
        shutdown();
    }

    void JoltJobSystemTaskflow::init(tf::Executor& executor, JPH::uint maxBarriers) noexcept
    {
        _taskExecutor = executor;
        JobSystemWithBarrier::Init(maxBarriers);
        _future = executor.run(_taskflow);
    }

    void JoltJobSystemTaskflow::shutdown()
    {
        _future.wait();
        _taskflow.clear();
    }

    const tf::Taskflow& JoltJobSystemTaskflow::getTaskflow() const
    {
        return _taskflow;
    }

    int JoltJobSystemTaskflow::GetMaxConcurrency() const
    {
        return _taskExecutor ? _taskExecutor->num_workers() : 0;
    }

    JoltJobSystemTaskflow::JobHandle JoltJobSystemTaskflow::CreateJob(const char* name, JPH::ColorArg color, const JobFunction& jobFunction, JPH::uint32 numDependencies)
    {
        return JobHandle(new Job(name, color, this, jobFunction, numDependencies));
    }


    void JoltJobSystemTaskflow::QueueJob(Job* job)
    {
        auto task = _taskflow.emplace([job]() { job->Execute();  });
        task.data(job);
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        task.name(_prefix + " " + job->GetName());
#else
        task.name(_prefix + " " + StringUtils::binToHex(job));
#endif
    }

    void JoltJobSystemTaskflow::QueueJobs(Job** jobs, JPH::uint numJobs)
    {
        for (JPH::uint i = 0; i < numJobs; ++i)
        {
            QueueJob(jobs[i]);
        }
    }

    void JoltJobSystemTaskflow::FreeJob(Job* job)
    {
        std::vector<tf::Task> tasks;
        _taskflow.for_each_task([job, &tasks](auto task)
        {
            if (task.data() == job)
            {
                tasks.push_back(task);
            }
        });
        for (auto& task : tasks)
        {
            _taskflow.erase(task);
        }
        delete job;
    }

    JoltBroadPhaseLayerInterface::JoltBroadPhaseLayerInterface(const Config& config) noexcept
        : _config(config)
    {
    }

    JPH::uint JoltBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const noexcept
    {
        return _config.broad.size();
    }

    BroadLayer PhysicsLayerConfig::getBroad(LayerMask layer) const noexcept
    {
        BroadLayer i = 0;
        BroadLayer defi = 0;
        for (auto& [broadLayer, mask] : broad)
        {
            if (mask == kAllLayers || mask == 0)
            {
                defi = i;
            }
            else if (mask & layer)
            {
                return i;
            }
            ++i;
        }
        return defi;
    }

    const std::string& PhysicsLayerConfig::getBroadName(BroadLayer layer) const
    {
        if (layer < 0 || layer >= broad.size())
        {
            throw std::invalid_argument("broad layer not defined");
        }
        auto itr = broad.begin();
        std::advance(itr, layer);
        return itr->first;
    }

    JPH::BroadPhaseLayer JoltBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer objLayer) const noexcept
    {
        return JPH::BroadPhaseLayer(_config.getBroad(objLayer));
    }

    const char* JoltBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer bpLayer) const
    {
        return _config.getBroadName(bpLayer.GetValue()).c_str();
    }

    JoltObjectVsBroadPhaseLayerFilter::JoltObjectVsBroadPhaseLayerFilter(const Config& config)
        : _config(config)
    {
    }

    JoltBroadPhaseLayerMaskFilter::JoltBroadPhaseLayerMaskFilter(BroadLayer layer) noexcept
        : _layer(layer)
    {
    }

    bool JoltBroadPhaseLayerMaskFilter::ShouldCollide(JPH::BroadPhaseLayer layer) const noexcept
    {
        return _layer == layer.GetValue();
    }

    bool JoltObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const noexcept
    {
        return _config.getBroad(layer1) == layer2.GetValue();
    }

    JoltObjectLayerPairFilter::JoltObjectLayerPairFilter(const Config& config)
        : _config(config)
    {
    }

    bool JoltObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const noexcept
    {
        return _config.getBroad(object1) == _config.getBroad(object2);
    }

    JoltObjectLayerMaskFilter::JoltObjectLayerMaskFilter(LayerMask layers) noexcept
        : _layers(layers)
    {
    }

    bool JoltObjectLayerMaskFilter::ShouldCollide(JPH::ObjectLayer layer) const noexcept
    {
        if (_layers == 0 || _layers == kAllLayers)
        {
            return true;
        }
        return (_layers & layer);
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

    PhysicsSystemImpl::PhysicsSystemImpl(PhysicsSystem& system, const Config& config, OptionalRef<bx::AllocatorI> alloc) noexcept
        : _system(system)
        , _alloc(alloc)
        , _config(config)
        , _deltaTimeRest(0.F)
        , _broadPhaseLayer(config.layers)
        , _objVsBroadPhaseLayerFilter(config.layers)
        , _objLayerPairFilter(config.layers)
        , _paused(false)
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
        _jobSystem.init(app.getTaskExecutor());
        _joltSystem = std::make_unique<JPH::PhysicsSystem>();
        _joltSystem->Init(_config.maxBodies, _config.numBodyMutexes,
            _config.maxBodyPairs, _config.maxContactConstraints,
            _broadPhaseLayer, _objVsBroadPhaseLayerFilter, _objLayerPairFilter);
        _deltaTimeRest = 0.F;
        _joltSystem->SetGravity(JoltUtils::convert(_config.gravity));
        _joltSystem->SetContactListener(this);

        scene.onConstructComponent<PhysicsBody>().connect<&PhysicsSystemImpl::onRigidbodyConstructed>(*this);
        scene.onDestroyComponent<PhysicsBody>().connect< &PhysicsSystemImpl::onRigidbodyDestroyed>(*this);
        scene.onConstructComponent<CharacterController>().connect<&PhysicsSystemImpl::onCharacterConstructed>(*this);
        scene.onDestroyComponent<CharacterController>().connect< &PhysicsSystemImpl::onCharacterDestroyed>(*this);

        auto rigidBodies = scene.getComponents<PhysicsBody>();
        for (auto [entity, body] : rigidBodies.each())
        {
            body.getImpl().init(body, _system);
        }
        auto charCtrls = scene.getComponents<CharacterController>();
        for (auto [entity, charCtrl] : charCtrls.each())
        {
            charCtrl.getImpl().init(charCtrl, _system);
        }
    }

    void PhysicsSystemImpl::shutdown() noexcept
    {
        if (_scene)
        {
            auto bodies = _scene->getComponents<PhysicsBody>();
            _scene->removeComponents<PhysicsBody>(bodies.begin(), bodies.end());
            auto charCtrls = _scene->getComponents<CharacterController>();
            _scene->removeComponents<PhysicsBody>(charCtrls.begin(), charCtrls.end());

            _scene->onConstructComponent<PhysicsBody>().disconnect<&PhysicsSystemImpl::onRigidbodyConstructed>(*this);
            _scene->onDestroyComponent<PhysicsBody>().disconnect< &PhysicsSystemImpl::onRigidbodyDestroyed>(*this);
            _scene->onConstructComponent<CharacterController>().disconnect<&PhysicsSystemImpl::onCharacterConstructed>(*this);
            _scene->onDestroyComponent<CharacterController>().disconnect< &PhysicsSystemImpl::onCharacterDestroyed>(*this);
            _scene.reset();
        }

        _listeners.clear();
        _joltSystem.reset();
        _jobSystem.shutdown();
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    void PhysicsSystemImpl::onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& body = registry.get<PhysicsBody>(entity);
        body.getImpl().init(body, _system);
    }

    void PhysicsSystemImpl::onRigidbodyDestroyed(EntityRegistry& registry, Entity entity)
    {
        auto& body = registry.get<PhysicsBody>(entity).getImpl();
        body.shutdown();
    }

    void PhysicsSystemImpl::onCharacterConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& character = registry.get<CharacterController>(entity);
        character.getImpl().init(character, _system);
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
        if (_paused)
        {
            return;
        }
        if (!_joltSystem)
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
                updater.fixedUpdate(fdt);
            }

            // TODO: skeletal animations here? or maybe with an updater
            // probably important for ragdolls or inverse kinematics

            auto err = _joltSystem->Update(fdt, _config.collisionSteps, &_alloc, &_jobSystem);
            _deltaTimeRest -= fdt;
            if (err != JPH::EPhysicsUpdateError::None)
            {
                throw std::runtime_error(std::string("physics update error ") + getUpdateErrorString(err));
            }

            processPendingCollisionEvents();
        }

        if (_scene)
        {
            auto entities = _scene->getUpdateEntities<PhysicsBody>();
            for (auto entity : entities)
            {
                auto& body = _scene->getComponent<PhysicsBody>(entity).value();
                body.getImpl().update(entity, deltaTime);
            }
            entities = _scene->getUpdateEntities<CharacterController>();
            for (auto entity : entities)
            {
                auto& charCtrl = _scene->getComponent<CharacterController>(entity).value();
                charCtrl.getImpl().update(entity, deltaTime);
            }
        }
    }

    void PhysicsSystemImpl::addUpdater(std::unique_ptr<IPhysicsUpdater>&& updater) noexcept
    {
        _updaters.insert(std::move(updater));
    }

    void PhysicsSystemImpl::addUpdater(IPhysicsUpdater& updater) noexcept
    {
        _updaters.insert(updater);
    }
    
    bool PhysicsSystemImpl::removeUpdater(const IPhysicsUpdater& updater) noexcept
    {
        return _updaters.erase(updater);
    }

    size_t PhysicsSystemImpl::removeUpdaters(const IPhysicsUpdaterFilter& filter) noexcept
    {
        return _updaters.eraseIf(filter);
    }

    void PhysicsSystemImpl::addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept
    {
        _listeners.insert(std::move(listener));
    }

    void PhysicsSystemImpl::addListener(ICollisionListener& listener) noexcept
    {
        _listeners.insert(listener);
    }

    bool PhysicsSystemImpl::removeListener(const ICollisionListener& listener) noexcept
    {
        return _listeners.erase(listener);
    }

    size_t PhysicsSystemImpl::removeListeners(const ICollisionListenerFilter& filter) noexcept
    {
        return _listeners.eraseIf(filter);
    }

    const tf::Taskflow& PhysicsSystemImpl::getTaskflow() const
    {
        return _jobSystem.getTaskflow();
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
        return _joltSystem != nullptr ? OptionalRef<JPH::PhysicsSystem>(*_joltSystem) : nullptr;
    }

    OptionalRef<const JPH::PhysicsSystem> PhysicsSystemImpl::getJolt() const noexcept
    {
        return _joltSystem != nullptr ? OptionalRef<const JPH::PhysicsSystem>(*_joltSystem) : nullptr;
    }

    JPH::BodyInterface& PhysicsSystemImpl::getBodyInterface() const noexcept
    {
        return _joltSystem->GetBodyInterface();
    }

    const JPH::BodyLockInterface& PhysicsSystemImpl::getBodyLockInterface() const noexcept
    {
        return _joltSystem->GetBodyLockInterface();
    }

    JoltTempAllocator& PhysicsSystemImpl::getTempAllocator() noexcept
    {
        return _alloc;
    }

    glm::vec3 PhysicsSystemImpl::getGravity() noexcept
    {
        return JoltUtils::convert(_joltSystem->GetGravity());
    }

    bool PhysicsSystemImpl::isPaused() const noexcept
    {
        return _paused;
    }

    void PhysicsSystemImpl::setPaused(bool paused) noexcept
    {
        _paused = paused;
    }

    bool PhysicsSystemImpl::isValidEntity(Entity entity) noexcept
    {
        if (!_scene)
        {
            return false;
        }
        auto trans = _scene->getComponent<Transform>(entity);
        if (!trans)
        {
            return true;
        }
        return tryLoadTransform(trans.value()).hasValue();
    }

    void PhysicsSystemImpl::setRootTransform(OptionalRef<Transform> root) noexcept
    {
        _root = root;
    }

    OptionalRef<Transform> PhysicsSystemImpl::getRootTransform() noexcept
    {
        return _root;
    }

    Expected<JoltTransform, std::string> PhysicsSystemImpl::tryLoadTransform(Transform& trans) noexcept
    {
        trans.update();
        glm::mat4 mat = trans.getWorldMatrix();
        if (_root)
        {
            mat = _root->getWorldInverse() * mat;
        }
        return JoltUtils::convertTransform(mat);
    }

    JoltTransform PhysicsSystemImpl::loadTransform(Transform& trans)
    {
        auto joltTrans = tryLoadTransform(trans);
        if (joltTrans)
        {
            return joltTrans.value();
        }
        throw std::runtime_error(joltTrans.error());
    }

    void PhysicsSystemImpl::updateTransform(Transform& trans, const JPH::Mat44& jmtx) noexcept
    {
        auto mtx = JoltUtils::convert(jmtx);
        auto parent = trans.getParent();
        if (parent)
        {
            mtx = parent->getWorldInverse() * mtx;
        }
        glm::vec3 pos;
        glm::quat rot;
        glm::vec3 scale;
        Math::decompose(mtx, pos, rot, scale);
        // do not apply scale
        trans.setPosition(pos);
        trans.setRotation(rot);
    }

    OptionalRef<PhysicsBody> PhysicsSystemImpl::getPhysicsBody(const JPH::BodyID& bodyId) const noexcept
    {
        auto userData = getBodyInterface().GetUserData(bodyId);
        if (userData == 0)
        {
            return nullptr;
        }
        return reinterpret_cast<PhysicsBody*>(userData);
    }

    OptionalRef<PhysicsBody> PhysicsSystemImpl::getPhysicsBody(const JPH::Body& body) noexcept
    {
        auto userData = body.GetUserData();
        if (userData == 0)
        {
            return nullptr;
        }
        return reinterpret_cast<PhysicsBody*>(userData);
    }

    std::optional<RaycastHit> PhysicsSystemImpl::raycast(const Ray& ray, LayerMask layers) const noexcept
    {
        if (!_joltSystem)
        {
            return std::nullopt;
        }
        auto joltRay = JoltUtils::convert(ray);
        JoltBroadPhaseLayerMaskFilter bphFilter(_config.layers.getBroad(layers));
        JoltObjectLayerMaskFilter objFilter(layers);
        JPH::RayCastResult result;

        if (!_joltSystem->GetNarrowPhaseQuery().CastRay(joltRay, result, bphFilter, objFilter))
        {
            return std::nullopt;
        }
        auto rb = getPhysicsBody(result.mBodyID);
        if (!rb)
        {
            return std::nullopt;
        }
        return JoltUtils::convert(result, ray, rb.value());
    }

    std::vector<RaycastHit> PhysicsSystemImpl::raycastAll(const Ray& ray, LayerMask layers) const noexcept
    {
        std::vector<RaycastHit> hits;
        if (!_joltSystem)
        {
            return hits;
        }

        auto joltRay = JoltUtils::convert(ray);
        JoltBroadPhaseLayerMaskFilter bphFilter(_config.layers.getBroad(layers));
        JoltObjectLayerMaskFilter objFilter(layers);
        JPH::RayCastSettings settings;
        JoltVectorCastRayCollector collector;

        _joltSystem->GetNarrowPhaseQuery().CastRay(joltRay, settings, collector, bphFilter, objFilter);
        for (auto& result : collector.getHits())
        {
            auto rb = getPhysicsBody(result.mBodyID);
            if (rb)
            {
                hits.push_back(JoltUtils::convert(result, ray, rb.value()));
            }
        }

        return hits;
    }

    void PhysicsSystemImpl::activateBodies(const BoundingBox& bbox, LayerMask layers) noexcept
    {
        auto& iface = _joltSystem->GetBodyInterface();
        JoltBroadPhaseLayerMaskFilter bphFilter(_config.layers.getBroad(layers));
        JoltObjectLayerMaskFilter objFilter(layers);
        iface.ActivateBodiesInAABox(JoltUtils::convert(bbox), bphFilter, objFilter);
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
        for (size_t i = 0; i < manifold.mRelativeContactPointsOn1.size(); ++i)
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
            listener.onCollisionEnter(body1, body2, collision);
        }
        Collision collision2(collision);
    }

    void PhysicsSystemImpl::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        body1.getImpl().onCollisionStay(body2, collision);
        body2.getImpl().onCollisionStay(body1, collision);
        for (auto& listener : _listeners)
        {
            listener.onCollisionStay(body1, body2, collision);
        }
    }

    void PhysicsSystemImpl::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        body1.getImpl().onCollisionExit(body2);
        body2.getImpl().onCollisionExit(body1);
        for (auto& listener : _listeners)
        {
            listener.onCollisionExit(body1, body2);
        }
    }
    
    PhysicsSystem::PhysicsSystem(const Config& config, bx::AllocatorI& alloc) noexcept
        : _impl(std::make_unique<PhysicsSystemImpl>(*this, config, alloc))
    {
    }

    PhysicsSystem::PhysicsSystem(const Config& config) noexcept
        : _impl(std::make_unique<PhysicsSystemImpl>(*this, config))
    {
    }

    PhysicsSystem::PhysicsSystem(bx::AllocatorI& alloc) noexcept
        : _impl(std::make_unique<PhysicsSystemImpl>(*this, Config{}, alloc))
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


    OptionalRef<Scene> PhysicsSystem::getScene() noexcept
    {
        return _impl->getScene();
    }

    OptionalRef<const Scene> PhysicsSystem::getScene() const noexcept
    {
        return _impl->getScene();
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

    OptionalRef<const Transform> PhysicsSystem::getRootTransform() const noexcept
    {
        return _impl->getRootTransform();
    }

    glm::vec3 PhysicsSystem::getGravity() const
    {
        return _impl->getGravity();
    }

    bool PhysicsSystem::isValidEntity(Entity entity) noexcept
    {
        return _impl->isValidEntity(entity);
    }

    bool PhysicsSystem::isPaused() const noexcept
    {
        return _impl->isPaused();
    }

    PhysicsSystem& PhysicsSystem::setPaused(bool paused) noexcept
    {
        _impl->setPaused(paused);
        return *this;
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

    PhysicsSystem& PhysicsSystem::addUpdater(std::unique_ptr<IPhysicsUpdater>&& updater) noexcept
    {
        _impl->addUpdater(std::move(updater));
        return *this;
    }
    
    PhysicsSystem& PhysicsSystem::addUpdater(IPhysicsUpdater& updater) noexcept
    {
        _impl->addUpdater(updater);
        return *this;
    }

    bool PhysicsSystem::removeUpdater(const IPhysicsUpdater& updater) noexcept
    {
        return _impl->removeUpdater(updater);
    }
    
    size_t PhysicsSystem::removeUpdaters(const IPhysicsUpdaterFilter& filter) noexcept
    {
        return _impl->removeUpdaters(filter);
    }

    PhysicsSystem& PhysicsSystem::addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept
    {
        _impl->addListener(std::move(listener));
        return *this;
    }

    PhysicsSystem& PhysicsSystem::addListener(ICollisionListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool PhysicsSystem::removeListener(const ICollisionListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }

    size_t PhysicsSystem::removeListeners(const ICollisionListenerFilter& filter) noexcept
    {
        return _impl->removeListeners(filter);
    }

    std::optional<RaycastHit> PhysicsSystem::raycast(const Ray& ray, LayerMask layers) const noexcept
    {
        return _impl->raycast(ray, layers);
    }

    std::vector<RaycastHit> PhysicsSystem::raycastAll(const Ray& ray, LayerMask layers) const noexcept
    {
        return _impl->raycastAll(ray, layers);
    }

    void PhysicsSystem::activateBodies(const BoundingBox& bbox, LayerMask layers) noexcept
    {
        return _impl->activateBodies(bbox, layers);
    }

    PhysicsBodyImpl::PhysicsBodyImpl(const Config& config) noexcept
        : _initConfig(config)
        , _maxSepDistance(0.F)
    {
    }

    PhysicsBodyImpl::PhysicsBodyImpl(const CharacterConfig& config) noexcept
        : _initConfig(config)
        , _maxSepDistance(0.F)
    {
    }

    PhysicsBodyImpl::~PhysicsBodyImpl()
    {
        shutdown();
    }

    void PhysicsBodyImpl::init(PhysicsBody& body, PhysicsSystem& system) noexcept
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
        auto& iface = getSystemImpl().getBodyInterface();
        if (!_bodyId.IsInvalid() && iface.IsAdded(_bodyId))
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
        return _system->getImpl().getBodyInterface();
    }

    OptionalRef<const JPH::BodyLockInterface> PhysicsBodyImpl::getBodyLockInterface() const noexcept
    {
        if (!_system)
        {
            return nullptr;
        }
        return _system->getImpl().getBodyLockInterface();
    }

    JPH::BodyID PhysicsBodyImpl::createCharacter(const CharacterConfig& config, const JoltTransform& trans)
    {
        auto joltSystem = getSystemImpl().getJolt();
        if (!joltSystem)
        {
            return {};
        }
        JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings();
        settings->mMaxSlopeAngle = config.maxSlopeAngle;
        settings->mShape = JoltUtils::convert(config.shape, trans.scale);
        settings->mFriction = config.friction;
        settings->mSupportingVolume = JoltUtils::convert(config.supportingPlane);
        settings->mMass = config.mass;
        settings->mGravityFactor = config.gravityFactor;
        settings->mUp = JoltUtils::convert(config.up);
        settings->mLayer = config.layer;
        settings->mEnhancedInternalEdgeRemoval = true;
        auto userData = (uint64_t)_body.ptr();
        _character = new JPH::Character(settings, trans.position, trans.rotation, userData, joltSystem.ptr());
        _character->AddToPhysicsSystem();
        _maxSepDistance = config.maxSeparationDistance;
        _shape = config.shape;
        return _character->GetBodyID();
    }

    JPH::BodyID PhysicsBodyImpl::createBody(const Config& config, const JoltTransform& trans)
    {
        if (!_system)
        {
            return {};
        }

        JPH::EMotionType joltMotion = JPH::EMotionType::Dynamic;
        auto activation = JPH::EActivation::Activate;
        switch (config.motion)
        {
        case PhysicsBodyMotionType::Kinematic:
            joltMotion = JPH::EMotionType::Kinematic;
            break;
        case PhysicsBodyMotionType::Static:
            joltMotion = JPH::EMotionType::Static;
            break;
        default:
            break;
        }

        auto shape = JoltUtils::convert(config.shape, trans.scale);
        JPH::BodyCreationSettings settings(shape, trans.position, trans.rotation,
            joltMotion, config.layer);
        settings.mGravityFactor = config.gravityFactor;
        settings.mFriction = config.friction;
        settings.mUserData = (uint64_t)_body.ptr();
        settings.mIsSensor = config.trigger;
        settings.mInertiaMultiplier = config.inertiaFactor;
        if (config.mass)
        {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
            settings.mMassPropertiesOverride.mMass = config.mass.value();
        }
        _shape = config.shape;
        return getBodyInterface()->CreateAndAddBody(settings, activation);
    }

    bool PhysicsBodyImpl::tryCreateBody(OptionalRef<Transform> trans)
    {
        if (!_bodyId.IsInvalid() || !_system || !_initConfig)
        {
            return false;
        }

        JoltTransform joltTrans;
        if (trans)
        {
            joltTrans = getSystemImpl().loadTransform(trans.value());
        }
        auto configPtr = &_initConfig.value();
        if (auto charConfig = std::get_if<CharacterConfig>(configPtr))
        {
            _bodyId = createCharacter(*charConfig, joltTrans);
        }
        else if(auto bodyConfig = std::get_if<Config>(configPtr))
        {
            _bodyId = createBody(*bodyConfig, joltTrans);
        }
        _initConfig.reset();
        return true;
    }

    void PhysicsBodyImpl::update(Entity entity, float deltaTime)
    {
        if (!_system)
        {
            return;
        }
        auto trans = _system->getScene()->getComponent<Transform>(entity);
        if (!trans && getMotionType() != MotionType::Static)
        {
            trans = _system->getScene()->addComponent<Transform>(entity);
        }
        tryCreateBody(trans);
        if (_character)
        {
            _character->PostSimulation(_maxSepDistance, false);
        }
        if (!isEnabled())
        {
            return;
        }
        if (trans)
        {
            if (getMotionType() == MotionType::Kinematic)
            {
                updateJolt(trans->getWorldMatrix());
            }
            else
            {
                getSystemImpl().updateTransform(trans.value(), getBodyInterface()->GetWorldTransform(_bodyId));
            }
        }
    }

    void PhysicsBodyImpl::updateJolt(const glm::mat4& mtx)
    {
        auto iface = getBodyInterface();
        if (!iface)
        {
            return;
        }
        auto jmtx = JoltUtils::convert(mtx);
        JPH::Vec3 scale;
        jmtx = jmtx.Decompose(scale);
        auto rot = jmtx.GetQuaternion();
        if(!rot.IsNormalized())
        {
            rot = JPH::Quat::sIdentity();
        }
        iface->SetPositionAndRotation(_bodyId, jmtx.GetTranslation(), rot, JPH::EActivation::Activate);
    }

    const std::unordered_map<PhysicsBodyImpl::MotionType, std::string> PhysicsBodyImpl::_motionTypeNames = {
        { MotionType::Static, "Static"},
        { MotionType::Dynamic, "Dynamic"},
        { MotionType::Kinematic, "Kinematic"}
    };

    const std::string& PhysicsBodyImpl::getMotionTypeName(MotionType motion) noexcept
    {
        auto itr = _motionTypeNames.find(motion);
        if (itr == _motionTypeNames.end())
        {
            static const std::string empty;
            return empty;
        }
        return itr->second;
    }

    std::string PhysicsBodyImpl::toString() const noexcept
    {
        std::ostringstream ss;
        ss << "PhysicsBody(" << _bodyId.GetIndex();
        auto seq = _bodyId.GetSequenceNumber();
        if (seq != 0)
        {
            ss << ":" << (int)seq;
        }
        ss << ", type=" << getMotionTypeName(getMotionType());
        ss << ", pos=" << glm::to_string(getPosition());
        ss << ", rot=" << glm::to_string(getRotation()) << ")";
        return ss.str();
    }

    OptionalRef<PhysicsSystem> PhysicsBodyImpl::getSystem() const noexcept
    {
        return _system;
    }

    PhysicsSystemImpl& PhysicsBodyImpl::getSystemImpl()
    {
        return _system->getImpl();
    }

    PhysicsBodyImpl::Shape PhysicsBodyImpl::getShape() const noexcept
    {
        return _shape.value_or(BoundingBox());
    }

    PhysicsBodyImpl::MotionType PhysicsBodyImpl::getMotionType() const noexcept
    {
        auto joltType = getBodyInterface()->GetMotionType(_bodyId);
        switch (joltType)
        {
        case JPH::EMotionType::Dynamic:
            return MotionType::Dynamic;
            break;
        case JPH::EMotionType::Static:
            return MotionType::Static;
            break;
        case JPH::EMotionType::Kinematic:
            return MotionType::Kinematic;
            break;
        }
        return MotionType::Static;
    }

    const JPH::BodyID& PhysicsBodyImpl::getBodyId() const noexcept
    {
        return _bodyId;
    }

    bool PhysicsBodyImpl::isGrounded() const noexcept
    {
        return getGroundState() == GroundState::Grounded;
    }

    GroundState PhysicsBodyImpl::getGroundState() const noexcept
    {
        if (!_character)
        {
            return GroundState::NotSupported;
        }
        return (GroundState)_character->GetGroundState();
    }

    void PhysicsBodyImpl::setPosition(const glm::vec3& pos)
    {
        auto jpos = JoltUtils::convert(pos);
        getBodyInterface()->SetPosition(_bodyId, jpos, JPH::EActivation::Activate);
    }

    glm::vec3 PhysicsBodyImpl::getPosition() const
    {
        return JoltUtils::convert(getBodyInterface()->GetPosition(_bodyId));
    }

    void PhysicsBodyImpl::setRotation(const glm::quat& rot)
    {
        getBodyInterface()->SetRotation(_bodyId, JoltUtils::convert(rot), JPH::EActivation::Activate);
    }

    glm::quat PhysicsBodyImpl::getRotation() const
    {
        return JoltUtils::convert(getBodyInterface()->GetRotation(_bodyId));
    }

    void PhysicsBodyImpl::setLinearVelocity(const glm::vec3& velocity)
    {
        getBodyInterface()->SetLinearVelocity(_bodyId, JoltUtils::convert(velocity));
    }

    glm::vec3 PhysicsBodyImpl::getLinearVelocity() const
    {
        return JoltUtils::convert(getBodyInterface()->GetLinearVelocity(_bodyId));
    }

    void PhysicsBodyImpl::setAngularVelocity(const glm::vec3& velocity)
    {
        getBodyInterface()->SetAngularVelocity(_bodyId, JoltUtils::convert(velocity));
    }

    glm::vec3 PhysicsBodyImpl::getAngularVelocity() const
    {
        return JoltUtils::convert(getBodyInterface()->GetAngularVelocity(_bodyId));
    }

    BoundingBox PhysicsBodyImpl::getLocalBounds() const
    {
        auto shape = getBodyInterface()->GetShape(_bodyId);
        return JoltUtils::convert(shape->GetLocalBounds());
    }

    BoundingBox PhysicsBodyImpl::getWorldBounds() const
    {
        auto shape = getBodyInterface()->GetShape(_bodyId);
        auto trans = getBodyInterface()->GetCenterOfMassTransform(_bodyId);
        JPH::Vec3 scale(1.F, 1.F, 1.F);
        return JoltUtils::convert(shape->GetWorldSpaceBounds(trans, scale));
    }

    float PhysicsBodyImpl::getInverseMass() const
    {
        auto lockFace = getBodyLockInterface();
        if (!lockFace)
        {
            return 0.F;
        }
        JPH::BodyLockRead lock(lockFace.value(), _bodyId);
        if (lock.Succeeded())
        {
            auto& body = lock.GetBody();
            return body.GetMotionProperties()->GetInverseMass();
        }
        return 0.F;
    }

    void PhysicsBodyImpl::setInverseMass(float v)
    {
        auto lockFace = getBodyLockInterface();
        if (!lockFace)
        {
            return;
        }
        JPH::BodyLockWrite lock(lockFace.value(), _bodyId);
        if (lock.Succeeded())
        {
            auto& body = lock.GetBody();
            body.GetMotionProperties()->SetInverseMass(v);
        }
    }

    bool PhysicsBodyImpl::isActive() const
    {
        if (_bodyId.IsInvalid())
        {
            return false;
        }
        return getBodyInterface()->IsActive(_bodyId);
    }

    void PhysicsBodyImpl::activate()
    {
        getBodyInterface()->ActivateBody(_bodyId);
    }

    void PhysicsBodyImpl::deactivate()
    {
        getBodyInterface()->DeactivateBody(_bodyId);
    }

    bool PhysicsBodyImpl::isEnabled() const
    {
        if (_bodyId.IsInvalid())
        {
            return false;
        }
        return getBodyInterface()->IsAdded(_bodyId);
    }

    void PhysicsBodyImpl::setEnabled(bool enabled)
    {
        auto iface = getBodyInterface();
        auto added = iface->IsAdded(_bodyId);
        if (enabled)
        {
            if (!added)
            {
                iface->AddBody(_bodyId, JPH::EActivation::Activate);
            }
        }
        else if(added)
        {
            iface->RemoveBody(_bodyId);
        }
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
        auto minTime = getSystemImpl().getConfig().fixedDeltaTime;
        if (deltaTime < minTime)
        {
            deltaTime = minTime;
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

        auto minTime = getSystemImpl().getConfig().fixedDeltaTime;
        if (deltaTime < minTime)
        {
            deltaTime = minTime;
        }
        iface->MoveKinematic(_bodyId,
            JoltUtils::convert(pos),
            rot, deltaTime);
    }

    void PhysicsBodyImpl::addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept
    {
        _listeners.insert(std::move(listener));
    }

    void PhysicsBodyImpl::addListener(ICollisionListener& listener) noexcept
    {
        _listeners.insert(listener);
    }

    bool PhysicsBodyImpl::removeListener(const ICollisionListener& listener) noexcept
    {
        return _listeners.erase(listener);
    }

    size_t PhysicsBodyImpl::removeListeners(const ICollisionListenerFilter& filter) noexcept
    {
        return _listeners.eraseIf(filter);
    }

    void PhysicsBodyImpl::onCollisionEnter(PhysicsBody& other, const Collision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener.onCollisionEnter(_body.value(), other, collision);
        }
    }

    void PhysicsBodyImpl::onCollisionStay(PhysicsBody& other, const Collision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener.onCollisionStay(_body.value(), other, collision);
        }
    }

    void PhysicsBodyImpl::onCollisionExit(PhysicsBody& other)
    {
        for (auto& listener : _listeners)
        {
            listener.onCollisionExit(_body.value(), other);
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

    PhysicsBody::Shape PhysicsBody::getShape() const noexcept
    {
        return _impl->getShape();
    }

    PhysicsBody::MotionType PhysicsBody::getMotionType() const noexcept
    {
        return _impl->getMotionType();
    }

    BoundingBox PhysicsBody::getLocalBounds() const
    {
        return _impl->getLocalBounds();
    }

    BoundingBox PhysicsBody::getWorldBounds() const
    {
        return _impl->getWorldBounds();
    }

    bool PhysicsBody::isGrounded() const noexcept
    {
        return _impl->isGrounded();
    }

    GroundState PhysicsBody::getGroundState() const noexcept
    {
        return _impl->getGroundState();
    }

    PhysicsBody& PhysicsBody::setPosition(const glm::vec3& pos)
    {
        _impl->setPosition(pos);
        return *this;
    }

    glm::vec3 PhysicsBody::getPosition() const
    {
        return _impl->getPosition();
    }

    PhysicsBody& PhysicsBody::setRotation(const glm::quat& rot)
    {
        _impl->setRotation(rot);
        return *this;
    }

    glm::quat PhysicsBody::getRotation() const
    {
        return _impl->getRotation();
    }

    PhysicsBody& PhysicsBody::setLinearVelocity(const glm::vec3& velocity)
    {
        _impl->setLinearVelocity(velocity);
        return *this;
    }

    glm::vec3 PhysicsBody::getLinearVelocity() const
    {
        return _impl->getLinearVelocity();
    }

    float PhysicsBody::getInverseMass() const
    {
        return _impl->getInverseMass();
    }

    PhysicsBody& PhysicsBody::setInverseMass(float v)
    {
        _impl->setInverseMass(v);
        return *this;
    }

    bool PhysicsBody::isActive() const
    {
        return _impl->isActive();
    }

    PhysicsBody& PhysicsBody::activate()
    {
        _impl->activate();
        return *this;
    }

    PhysicsBody& PhysicsBody::deactivate()
    {
        _impl->deactivate();
        return *this;
    }

    bool PhysicsBody::isEnabled() const
    {
        return _impl->isEnabled();
    }

    PhysicsBody& PhysicsBody::setEnabled(bool enabled)
    {
        _impl->setEnabled(enabled);
        return *this;
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

    OptionalRef<PhysicsSystem> PhysicsBody::getSystem() const noexcept
    {
        return _impl->getSystem();
    }

    PhysicsBody& PhysicsBody::addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept
    {
        _impl->addListener(std::move(listener));
        return *this;
    }

    PhysicsBody& PhysicsBody::addListener(ICollisionListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool PhysicsBody::removeListener(const ICollisionListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }

    size_t PhysicsBody::removeListeners(const ICollisionListenerFilter& filter) noexcept
    {
        return _impl->removeListeners(filter);
    }

    std::string PhysicsBody::toString() const noexcept
    {
        return _impl->toString();
    }

    std::string Collision::toString() const noexcept
    {
        std::ostringstream ss;
        ss << "Collision(normal=" << glm::to_string(normal) << " contacts=(";
        ss << StringUtils::join(", ", contacts, [](auto& contact) { return glm::to_string(contact);  }) << "))";
        return ss.str();
    }

    std::string RaycastHit::toString() const noexcept
    {
        std::ostringstream ss;
        ss << "RaycastHit(dist=" << distance << ", ";
        ss << body.get().toString() << ")";
        return ss.str();
    }
}