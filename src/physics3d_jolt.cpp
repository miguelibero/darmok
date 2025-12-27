#include "detail/physics3d_jolt.hpp"
#include "detail/physics3d_character_jolt.hpp"

#include <darmok/physics3d.hpp>
#include <darmok/transform.hpp>
#include <darmok/math.hpp>
#include <darmok/string.hpp>
#include <darmok/collection.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/stream.hpp>

#include <bx/allocator.h>
#include <glm/gtx/quaternion.hpp>
#include <fmt/format.h>

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
        auto flipped = Math::flipHandedness(v);
        return JPH::Mat44(
            convert(flipped[0]),
            convert(flipped[1]),
            convert(flipped[2]),
            convert(flipped[3])
        );
    }

    glm::mat4 JoltUtils::convert(const JPH::Mat44& v) noexcept
    {
        const glm::mat4 mat(
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
        const glm::quat quat(v.GetW(), v.GetX(), v.GetY(), v.GetZ());
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

    expected<JoltTransform, std::string> JoltUtils::convertTransform(const glm::mat4& mat) noexcept
    {
        glm::vec3 pos(0);
        glm::quat rot(1, 0, 0, 0);
        glm::vec3 scale(1);
        Math::decompose(mat, pos, rot, scale);
        static const int epsilonFactor = 100;
        if (!Math::almostEqual(scale.x, scale.y, epsilonFactor) || !Math::almostEqual(scale.x, scale.z, epsilonFactor))
        {
            return unexpected<std::string>("non-uniform scale not supported");
        }
        return JoltTransform{ JoltUtils::convert(pos), JoltUtils::convert(rot), scale.x };
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

    expected<JPH::Ref<JPH::Shape>, std::string> joltGetShape(JPH::ShapeSettings& settings) noexcept
    {
        auto result = settings.Create();
        if (result.HasError())
        {
            return unexpected<std::string>{ result.GetError().c_str() };
        }
        return result.Get();
    }

    expected<JPH::ShapeRefC, std::string> joltGetOffsetShape(JPH::ShapeSettings& settings, const glm::vec3& offset) noexcept
    {
        auto shapeResult = joltGetShape(settings);
        if (!shapeResult)
        {
            return unexpected{ std::move(shapeResult).error() };
        }
        if (offset == glm::vec3{ 0 })
        {
            return shapeResult.value();
        }
        JPH::RotatedTranslatedShapeSettings offsetSettings{ JoltUtils::convert(offset), JPH::Quat::sIdentity(), shapeResult.value() };
        return joltGetShape(offsetSettings);
    }

    expected<JPH::ShapeRefC, std::string> JoltUtils::convert(const PhysicsShape& shape, float scale) noexcept
    {
        std::optional<Cube> optCube;
        if (auto cubePtr = std::get_if<Cube>(&shape))
        {
            optCube = *cubePtr;
        }
        else if (auto bboxPtr = std::get_if<BoundingBox>(&shape))
        {
            optCube = Cube{ *bboxPtr };
        }
        if (optCube)
        {
            auto cube = optCube.value() * scale;
            auto radius = JoltUtils::getConvexRadius(cube.size);
            JPH::BoxShapeSettings settings{ JoltUtils::convertSize(cube.size * 0.5F), radius };
            return joltGetOffsetShape(settings, cube.origin);
        }

        if (auto spherePtr = std::get_if<Sphere>(&shape))
        {
            auto sphere = *spherePtr * scale;
            JPH::SphereShapeSettings settings{ sphere.radius };
            return joltGetOffsetShape(settings, sphere.origin);
        }
        if (auto capsPtr = std::get_if<Capsule>(&shape))
        {
            auto caps = *capsPtr * scale;
            JPH::CapsuleShapeSettings settings{ caps.cylinderHeight * 0.5f, caps.radius };
            return joltGetOffsetShape(settings, caps.origin);
        }
        if (auto polyPtr = std::get_if<Polygon>(&shape))
        {
            auto poly = *polyPtr * scale;
            auto tris = JoltUtils::convert(poly);
            JPH::MeshShapeSettings settings{ tris };
            return joltGetOffsetShape(settings, poly.origin);
        }
        return unexpected<std::string>{"unknown shape type"};
    }

    ConstPhysicsShapeDefinitionWrapper::ConstPhysicsShapeDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
    }

    glm::vec3 ConstPhysicsShapeDefinitionWrapper::getOrigin() const noexcept
    {
        if (_def.has_cube())
        {
            return darmok::convert<glm::vec3>(_def.cube().origin());
        }
        else if (_def.has_sphere())
        {
            return darmok::convert<glm::vec3>(_def.sphere().origin());
        }
        else if (_def.has_capsule())
        {
            return darmok::convert<glm::vec3>(_def.capsule().origin());
        }
        return glm::vec3{ 0 };
    }

    ConstPhysicsBodyDefinitionWrapper::ConstPhysicsBodyDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
    }

    ConstPhysicsBodyDefinitionWrapper::CharacterDefinition ConstPhysicsBodyDefinitionWrapper::toCharacter() noexcept
    {
        CharacterDefinition charDef;
        *charDef.mutable_shape() = _def.shape();
		charDef.set_mass(_def.mass());
		charDef.set_friction(_def.friction());
		charDef.set_gravity_factor(_def.gravity_factor());
		charDef.set_layer(_def.layer());

        return charDef;
    }

    const std::string JoltJobSystemTaskflow::_prefix = "JoltPhysics";

    JoltJobSystemTaskflow::JoltJobSystemTaskflow() noexcept
        : _taskflow(_prefix)
    {
    }

    JoltJobSystemTaskflow::~JoltJobSystemTaskflow() noexcept
    {
        shutdown();
    }

    void JoltJobSystemTaskflow::init(tf::Executor& executor, JPH::uint maxBarriers) noexcept
    {
        _taskExecutor = executor;
        JobSystemWithBarrier::Init(maxBarriers);
        _future = executor.run(_taskflow);
    }

    void JoltJobSystemTaskflow::shutdown() noexcept
    {
        _future.wait();
        _taskflow.clear();
    }

    const tf::Taskflow& JoltJobSystemTaskflow::getTaskflow() const noexcept
    {
        return _taskflow;
    }

    int JoltJobSystemTaskflow::GetMaxConcurrency() const noexcept
    {
        return _taskExecutor ? _taskExecutor->num_workers() : 0;
    }

    JoltJobSystemTaskflow::JobHandle JoltJobSystemTaskflow::CreateJob(const char* name, JPH::ColorArg color, const JobFunction& jobFunction, JPH::uint32 numDependencies) noexcept
    {
        return JobHandle{ new Job{name, color, this, jobFunction, numDependencies} };
    }

    void JoltJobSystemTaskflow::QueueJob(Job* job) noexcept
    {
        auto task = _taskflow.emplace([job]() { job->Execute();  });
        task.data(job);
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
        task.name(_prefix + " " + job->GetName());
#else
        task.name(_prefix + " " + fmt::to_string(fmt::ptr(job)));
#endif   
    }

    void JoltJobSystemTaskflow::QueueJobs(Job** jobs, JPH::uint numJobs) noexcept
    {
        for (JPH::uint i = 0; i < numJobs; ++i)
        {
            QueueJob(jobs[i]);
        }
    }

    void JoltJobSystemTaskflow::FreeJob(Job* job) noexcept
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

    JoltBroadPhaseLayerInterface::JoltBroadPhaseLayerInterface(const Definition& def) noexcept
		: _def{ def }
    {
    }

    JPH::uint JoltBroadPhaseLayerInterface::GetNumBroadPhaseLayers() const noexcept
    {
        return _def.getBroadLayerNum();
    }

    ConstPhysicsSystemDefinitionWrapper::ConstPhysicsSystemDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
	}

    std::size_t ConstPhysicsSystemDefinitionWrapper::getBroadLayerNum() const noexcept
    {
        return _def.layers_size();
    }

    BroadLayer ConstPhysicsSystemDefinitionWrapper::getBroadLayer(LayerMask layer) const noexcept
    {
        BroadLayer i = 0;
        BroadLayer defi = 0;
        for (const auto& [broadLayer, mask] : _def.layers())
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

    std::optional<std::string> ConstPhysicsSystemDefinitionWrapper::getBroadLayerName(BroadLayer layer) const noexcept
    {
        if (layer < 0 || layer >= _def.layers_size())
        {
            return std::nullopt;
        }
        auto itr = _def.layers().begin();
        std::advance(itr, layer);
        return itr->first;
    }

    JPH::BroadPhaseLayer JoltBroadPhaseLayerInterface::GetBroadPhaseLayer(JPH::ObjectLayer objLayer) const noexcept
    {

        return JPH::BroadPhaseLayer(_def.getBroadLayer(objLayer));
    }

    const char* JoltBroadPhaseLayerInterface::GetBroadPhaseLayerName(JPH::BroadPhaseLayer bpLayer) const noexcept
    {
        auto name = _def.getBroadLayerName(bpLayer.GetValue());
        return name ? name->c_str() : "";
    }

    JoltObjectVsBroadPhaseLayerFilter::JoltObjectVsBroadPhaseLayerFilter(const Definition& def) noexcept
        : _def{ def } 
    {
    }

    JoltBroadPhaseLayerMaskFilter::JoltBroadPhaseLayerMaskFilter(BroadLayer layer) noexcept
        : _layer{ layer }
    {
    }

    bool JoltBroadPhaseLayerMaskFilter::ShouldCollide(JPH::BroadPhaseLayer layer) const noexcept
    {
        return _layer == layer.GetValue();
    }

    bool JoltObjectVsBroadPhaseLayerFilter::ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const noexcept
    {
        return _def.getBroadLayer(layer1) == layer2.GetValue();
    }

    JoltObjectLayerPairFilter::JoltObjectLayerPairFilter(const Definition& def) noexcept
        : _def{ def }
    {
    }

    bool JoltObjectLayerPairFilter::ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const noexcept
    {
        return _def.getBroadLayer(object1) == _def.getBroadLayer(object2);
    }

    JoltObjectLayerMaskFilter::JoltObjectLayerMaskFilter(LayerMask layers) noexcept
        : _layers{ layers }
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
    bool joltAssertFailed(const char* expression, const char* message, const char* file, JPH::uint line) noexcept
    {
        std::stringstream ss;
        ss << file << ":" << line << ": (" << expression << ") " << (message != nullptr ? message : "");
        StreamUtils::log(ss.str(), true);

        return true; // breakpoint
    };

#endif // JPH_ENABLE_ASSERTS

    PhysicsSystemImpl::PhysicsSystemImpl(PhysicsSystem& system, const Definition& def, OptionalRef<bx::AllocatorI> alloc) noexcept
        : _system{ system }
        , _alloc{ alloc }
        , _def{ def }
        , _deltaTimeRest{ 0.F }
        , _broadPhaseLayer{ _def }
        , _objVsBroadPhaseLayerFilter{ _def }
        , _objLayerPairFilter{ _def }
        , _paused{ false }
    {
        JPH::RegisterDefaultAllocator();
        JPH::Trace = joltTraceImpl;
        JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = joltAssertFailed;)
    }

    expected<void, std::string> PhysicsSystemImpl::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
		auto result = shutdown();
        if (!result)
        {
            return result;
        }
        _def = def;
        _scene = context.getScene();
        return doInit();
    }

    expected<void, std::string> PhysicsSystemImpl::init(Scene& scene, App& app) noexcept
    {
        if (_scene)
        {
            auto result = shutdown();
            if(!result)
            {
                return unexpected{ std::move(result).error() };
			}
        }
        _scene = scene;
        _taskExecutor = app.getTaskExecutor();
        return doInit();
    }

    expected<void, std::string> PhysicsSystemImpl::doInit() noexcept
    {
        if (!_scene)
        {
            return unexpected<std::string>{"missing scene"};
        }
        JPH::Factory::sInstance = new JPH::Factory();
        JPH::RegisterTypes();
        if (_taskExecutor)
        {
            _jobSystem.emplace();
            _jobSystem->init(*_taskExecutor);
        }
        _joltSystem = std::make_unique<JPH::PhysicsSystem>();
        _joltSystem->Init(_def.max_bodies(), _def.num_body_mutexes(),
            _def.max_body_pairs(), _def.max_contact_constraints(),
            _broadPhaseLayer, _objVsBroadPhaseLayerFilter, _objLayerPairFilter);
        _deltaTimeRest = 0.F;
        _joltSystem->SetGravity(JoltUtils::convert(darmok::convert<glm::vec3>(_def.gravity())));
        _joltSystem->SetContactListener(this);

        _scene->onConstructComponent<PhysicsBody>().connect<&PhysicsSystemImpl::onRigidbodyConstructed>(*this);
        _scene->onDestroyComponent<PhysicsBody>().connect< &PhysicsSystemImpl::onRigidbodyDestroyed>(*this);
        _scene->onConstructComponent<CharacterController>().connect<&PhysicsSystemImpl::onCharacterConstructed>(*this);
        _scene->onDestroyComponent<CharacterController>().connect< &PhysicsSystemImpl::onCharacterDestroyed>(*this);

        auto rigidBodies = _scene->getComponents<PhysicsBody>();
        for (auto [entity, body] : rigidBodies.each())
        {
            body.getImpl().init(body, _system);
        }
        auto charCtrls = _scene->getComponents<CharacterController>();
        for (auto [entity, charCtrl] : charCtrls.each())
        {
            charCtrl.getImpl().init(charCtrl, _system);
        }
        return {};
    }

    expected<void, std::string> PhysicsSystemImpl::shutdown() noexcept
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
        if (_jobSystem)
        {
            _jobSystem->shutdown();
            _jobSystem.reset();
        }
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;

        return {};
    }

    expected<void, std::string> PhysicsSystemImpl::onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& body = registry.get<PhysicsBody>(entity);
        body.getImpl().init(body, _system);
        return {};
    }

    expected<void, std::string> PhysicsSystemImpl::onRigidbodyDestroyed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& body = registry.get<PhysicsBody>(entity).getImpl();
        body.shutdown();
        return {};
    }

    expected<void, std::string> PhysicsSystemImpl::onCharacterConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& character = registry.get<CharacterController>(entity);
        character.getImpl().init(character, _system);
        return {};
    }

    expected<void, std::string> PhysicsSystemImpl::onCharacterDestroyed(EntityRegistry& registry, Entity entity) noexcept
    {
        auto& character = registry.get<CharacterController>(entity).getImpl();
        character.shutdown();
        return {};
    }

    std::string_view PhysicsSystemImpl::getUpdateErrorString(JPH::EPhysicsUpdateError err) noexcept
    {
        return StringUtils::getEnumName(err);
    }
    
    expected<void, std::string> PhysicsSystemImpl::update(float deltaTime) noexcept
    {
        if (_paused)
        {
            return {};
        }
        if (!_joltSystem)
        {
            return {};
        }
        _deltaTimeRest += deltaTime;
        auto fdt = _def.fixed_delta_time();
        // try to do the same order as unity
        // https://docs.unity3d.com/Manual/ExecutionOrder.html
        while (_deltaTimeRest > fdt)
        {
            for (auto& updater : _updaters)
            {
                auto result = updater.fixedUpdate(fdt);
                if (!result)
                {
                    return result;
                }
            }

            // TODO: skeletal animations here? or maybe with an updater
            // probably important for ragdolls or inverse kinematics

            auto jobSystem = _jobSystem ? &*_jobSystem : nullptr;
            auto err = _joltSystem->Update(fdt, _def.collision_steps(), &_alloc, jobSystem);
            _deltaTimeRest -= fdt;
            if (err != JPH::EPhysicsUpdateError::None)
            {
                std::string errStr{ "physics update error " };
				errStr += getUpdateErrorString(err);
                return unexpected{ errStr };
            }

            auto result = processPendingCollisionEvents();
            if (!result)
            {
                return result;
            }
        }

        if (_scene)
        {
            auto entities = _scene->getUpdateEntities<PhysicsBody>();
            for (auto entity : entities)
            {
                auto& body = _scene->getComponent<PhysicsBody>(entity).value();
                auto result = body.getImpl().update(entity, deltaTime);
                if (!result)
                {
                    return result;
                }
            }
            entities = _scene->getUpdateEntities<CharacterController>();
            for (auto entity : entities)
            {
                auto& charCtrl = _scene->getComponent<CharacterController>(entity).value();
                auto result = charCtrl.getImpl().update(entity, deltaTime);
                if (!result)
                {
                    return result;
                }
            }
        }

		return {};
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

    float PhysicsSystemImpl::getFixedDeltaTime() const noexcept
    {
		return _def.fixed_delta_time();
    }

    const tf::Taskflow& PhysicsSystemImpl::getTaskflow() const noexcept
    {
        return _jobSystem->getTaskflow();
    }

    const PhysicsSystemImpl::Definition& PhysicsSystemImpl::getDefinition() const noexcept
    {
        return _def;
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
        return tryLoadTransform(trans.value()).has_value();
    }

    void PhysicsSystemImpl::setRootTransform(OptionalRef<Transform> root) noexcept
    {
        _root = root;
    }

    OptionalRef<Transform> PhysicsSystemImpl::getRootTransform() noexcept
    {
        return _root;
    }

    expected<JoltTransform, std::string> PhysicsSystemImpl::tryLoadTransform(Transform& trans) noexcept
    {
        trans.update();
        glm::mat4 mat = trans.getWorldMatrix();
        if (_root)
        {
            mat = _root->getWorldInverse() * mat;
        }
        return JoltUtils::convertTransform(mat);
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
        auto bphFilter = createBroadPhaseLayerFilter(layers);
        const JoltObjectLayerMaskFilter objFilter{ layers };
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

    JoltBroadPhaseLayerMaskFilter PhysicsSystemImpl::createBroadPhaseLayerFilter(LayerMask layer) const noexcept
    {
        auto broadLayer = ConstPhysicsSystemDefinitionWrapper{ _def }.getBroadLayer(layer);
		return { broadLayer };
	}

    std::vector<RaycastHit> PhysicsSystemImpl::raycastAll(const Ray& ray, LayerMask layers) const noexcept
    {
        std::vector<RaycastHit> hits;
        if (!_joltSystem)
        {
            return hits;
        }

        auto joltRay = JoltUtils::convert(ray);
        auto bphFilter = createBroadPhaseLayerFilter(layers);
        const JoltObjectLayerMaskFilter objFilter{ layers };
        const JPH::RayCastSettings settings;
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

        auto bphFilter = createBroadPhaseLayerFilter(layers);
        const JoltObjectLayerMaskFilter objFilter{ layers };
        iface.ActivateBodiesInAABox(JoltUtils::convert(bbox), bphFilter, objFilter);
    }

    expected<void, std::string> PhysicsSystemImpl::processPendingCollisionEvents() noexcept
    {
        std::lock_guard<std::mutex> guard{ _collisionMutex };
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
            expected<void, std::string> result;
            switch (ev.type)
            {
            case CollisionEventType::Enter:
                result = onCollisionEnter(rb1.value(), rb2.value(), ev.collision);
                break;
            case CollisionEventType::Stay:
                result = onCollisionStay(rb1.value(), rb2.value(), ev.collision);
                break;
            case CollisionEventType::Exit:
                result = onCollisionExit(rb1.value(), rb2.value());
                break;
            }
            if (!result)
            {
                return result;
            }
        }
        return {};
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

    void PhysicsSystemImpl::OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) noexcept
    {
        auto collision = createCollision(manifold);
        std::lock_guard<std::mutex> guard{ _collisionMutex };
        _pendingCollisionEvents.emplace_back(CollisionEventType::Enter, body1.GetID(), body2.GetID(), collision);
    }

    void PhysicsSystemImpl::OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) noexcept
    {
        auto collision = createCollision(manifold);
        std::lock_guard<std::mutex> guard{ _collisionMutex };
        _pendingCollisionEvents.emplace_back(CollisionEventType::Stay, body1.GetID(), body2.GetID(), collision);
    }

    void PhysicsSystemImpl::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) noexcept
    {
        std::lock_guard<std::mutex> guard{ _collisionMutex };
        _pendingCollisionEvents.emplace_back(CollisionEventType::Exit, inSubShapePair.GetBody1ID(), inSubShapePair.GetBody2ID());
    }

    expected<void, std::string> PhysicsSystemImpl::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) noexcept
    {
        auto result = body1.getImpl().onCollisionEnter(body2, collision);
        if (!result)
        {
            return result;
        }
        result = body2.getImpl().onCollisionEnter(body1, collision);
        if (!result)
        {
            return result;
        }
        for (auto& listener : _listeners)
        {
            result = listener.onCollisionEnter(body1, body2, collision);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    expected<void, std::string> PhysicsSystemImpl::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) noexcept
    {
        auto result = body1.getImpl().onCollisionStay(body2, collision);
        if (!result)
        {
            return result;
        }
        result = body2.getImpl().onCollisionStay(body1, collision);
        if (!result)
        {
            return result;
        }
        for (auto& listener : _listeners)
        {
            result = listener.onCollisionStay(body1, body2, collision);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    expected<void, std::string> PhysicsSystemImpl::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2) noexcept
    {
        auto result = body1.getImpl().onCollisionExit(body2);
        if (!result)
        {
            return result;
        }
        result = body2.getImpl().onCollisionExit(body1);
        if (!result)
        {
            return result;
        }
        for (auto& listener : _listeners)
        {
            result = listener.onCollisionExit(body1, body2);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }
    
    PhysicsSystem::PhysicsSystem(const Definition& def, bx::AllocatorI& alloc) noexcept
        : _impl{ std::make_unique<PhysicsSystemImpl>(*this, def, alloc) }
    {
    }

    PhysicsSystem::PhysicsSystem(const Definition& def) noexcept
        : _impl{ std::make_unique<PhysicsSystemImpl>(*this, def) }
    {
    }

    PhysicsSystem::PhysicsSystem(bx::AllocatorI& alloc) noexcept
        : _impl{ std::make_unique<PhysicsSystemImpl>(*this, createDefinition(), alloc) }
    {
    }

    PhysicsSystem::~PhysicsSystem() noexcept = default;

    PhysicsSystem::Definition PhysicsSystem::createDefinition() noexcept
    {
        Definition def;
		def.set_max_bodies(1024);
		def.set_max_body_pairs(1024);
		def.set_max_contact_constraints(1024);
		def.set_fixed_delta_time(1.F / 60.F);
		def.set_collision_steps(1);
		def.mutable_gravity()->set_y(-9.81F);
		def.mutable_layers()->insert({ "default", kAllLayers });
        return def;
    }

    expected<void, std::string> PhysicsSystem::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
		return _impl->load(def, context);
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

    glm::vec3 PhysicsSystem::getGravity() const noexcept
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
    
    expected<void, std::string> PhysicsSystem::init(Scene& scene, App& app) noexcept
    {
        return _impl->init(scene, app);
    }
    
    expected<void, std::string> PhysicsSystem::shutdown() noexcept
    {
        return _impl->shutdown();
    }
    
    expected<void, std::string> PhysicsSystem::update(float deltaTime) noexcept
    {
        return _impl->update(deltaTime);
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

    PhysicsBodyImpl::PhysicsBodyImpl(const Definition& def) noexcept
        : _initDef{def}
        , _maxSepDistance{ 0.F }
    {
    }

    PhysicsBodyImpl::PhysicsBodyImpl(const CharacterDefinition& def) noexcept
        : _initDef{ def }
        , _maxSepDistance(0.F)
    {
    }

    PhysicsBodyImpl::~PhysicsBodyImpl() noexcept
    {
        shutdown();
    }

    expected<void, std::string> PhysicsBodyImpl::load(const Definition& def, Entity entity) noexcept
    {
        _initDef = def;
        return doLoad(entity);
    }

    expected<void, std::string> PhysicsBodyImpl::load(const CharacterDefinition& def, Entity entity) noexcept
    {
        _initDef = def;
        return doLoad(entity);
    }

    expected<void, std::string> PhysicsBodyImpl::doLoad(Entity entity) noexcept
    {
        if (_system)
        {
            doShutdown();
        }
        if (entity != entt::null)
        {
            return update(entity, 0.F);
        }
        return {};
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

    void PhysicsBodyImpl::doShutdown() noexcept
    {
        auto& iface = getSystemImpl().getBodyInterface();
        if (!_bodyId.IsInvalid() && iface.IsAdded(_bodyId))
        {
            iface.RemoveBody(_bodyId);
        }
        if (_character)
        {
            _character = nullptr;
        }
        else if (!_bodyId.IsInvalid())
        {
            iface.DestroyBody(_bodyId);
        }
        _bodyId = JPH::BodyID{};
    }

    void PhysicsBodyImpl::shutdown(bool systemShutdown) noexcept
    {
        if (!_system)
        {
            return;
        }
        doShutdown();
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

    expected<JPH::BodyID, std::string> PhysicsBodyImpl::createCharacter(const CharacterDefinition& def, const JoltTransform& trans) noexcept
    {
        auto joltSystem = getSystemImpl().getJolt();
        if (!joltSystem)
        {
            return unexpected<std::string>{"missing jolt system"};
        }
        auto shapeResult = JoltUtils::convert(darmok::convert<PhysicsShape>(def.shape()), trans.scale);
        if (!shapeResult)
        {
            return unexpected{ std::move(shapeResult).error() };
        }
        const JPH::Ref<JPH::CharacterSettings> settings = new JPH::CharacterSettings{};
        settings->mMaxSlopeAngle = def.max_slope_angle();
        settings->mShape = shapeResult.value();
        settings->mFriction = def.friction();
        settings->mSupportingVolume = JoltUtils::convert(darmok::convert<Plane>(def.supporting_plane()));
        settings->mMass = def.mass();
        settings->mGravityFactor = def.gravity_factor();
        settings->mUp = JoltUtils::convert(darmok::convert<glm::vec3>(def.up()));
        settings->mLayer = def.layer();
        settings->mEnhancedInternalEdgeRemoval = true;
        auto userData = (uint64_t)_body.ptr();
        _character = new JPH::Character(settings, trans.position, trans.rotation, userData, joltSystem.ptr());
        _character->AddToPhysicsSystem();
        _maxSepDistance = def.max_separation_distance();
        _shape = darmok::convert<PhysicsShape>(def.shape());
        return _character->GetBodyID();
    }

    expected<JPH::BodyID, std::string> PhysicsBodyImpl::createBody(const Definition& def, const JoltTransform& trans) noexcept
    {
        if (!_system)
        {
            return unexpected<std::string>{"missing jolt system"};
        }

        JPH::EMotionType joltMotion = JPH::EMotionType::Dynamic;
        auto activation = JPH::EActivation::Activate;
        switch (def.motion())
        {
        case Definition::Kinematic:
            joltMotion = JPH::EMotionType::Kinematic;
            break;
        case Definition::Static:
            joltMotion = JPH::EMotionType::Static;
            break;
        default:
            break;
        }

        auto shapeResult = JoltUtils::convert(darmok::convert<PhysicsShape>(def.shape()), trans.scale);
        if (!shapeResult)
        {
            return unexpected{ std::move(shapeResult).error() };
        }
        JPH::BodyCreationSettings settings{ shapeResult.value(), trans.position, trans.rotation,
            joltMotion, static_cast<JPH::ObjectLayer>(def.layer()) };
        settings.mGravityFactor = def.gravity_factor();
        settings.mFriction = def.friction();
        settings.mUserData = (uint64_t)_body.ptr();
        settings.mIsSensor = def.trigger();
        settings.mInertiaMultiplier = def.inertia_factor();
        if (def.mass())
        {
            settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
            settings.mMassPropertiesOverride.mMass = def.mass();
        }
        _shape = darmok::convert<PhysicsShape>(def.shape());
        return getBodyInterface()->CreateAndAddBody(settings, activation);
    }

    expected<void, std::string> PhysicsBodyImpl::tryCreateBody(OptionalRef<Transform> trans) noexcept
    {
        if (!_bodyId.IsInvalid())
        {
            return {};
        }
        if (!_system)
        {
            return unexpected<std::string>{"missing system ref"};
        }
        if (!_initDef)
        {
            return unexpected<std::string>{"missing definition"};
        }

        JoltTransform joltTrans;
        if (trans)
        {
            auto transResult = getSystemImpl().tryLoadTransform(trans.value());
            if (!transResult)
            {
                return unexpected{ std::move(transResult).error() };
            }
            joltTrans = transResult.value();
        }
        auto defPtr = &_initDef.value();
        if (auto charDef = std::get_if<CharacterDefinition>(defPtr))
        {
            auto result = createCharacter(*charDef, joltTrans);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            _bodyId = result.value();
        }
        else if(auto bodyDef = std::get_if<Definition>(defPtr))
        {
            auto result = createBody(*bodyDef, joltTrans);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            _bodyId = result.value();
        }
        else
        {
            return unexpected<std::string>{"unknown definition"};
        }
        _initDef.reset();
        return {};
    }

    expected<void, std::string> PhysicsBodyImpl::update(Entity entity, float deltaTime) noexcept
    {
        if (!_system)
        {
            return {};
        }
        auto trans = _system->getScene()->getComponent<Transform>(entity);
        if (!trans && getMotionType() != Definition::Static)
        {
            trans = _system->getScene()->addComponent<Transform>(entity);
        }
        auto bodyResult = tryCreateBody(trans);
        if (!bodyResult)
        {
            return unexpected{ std::move(bodyResult).error() };
        }
        if (_character)
        {
            _character->PostSimulation(_maxSepDistance, false);
        }
        if (!isEnabled())
        {
            return {};
        }
        if (trans)
        {
            if (getMotionType() == Definition::Kinematic)
            {
                updateJolt(trans->getWorldMatrix());
            }
            else
            {
                getSystemImpl().updateTransform(trans.value(), getBodyInterface()->GetWorldTransform(_bodyId));
            }
        }
        return {};
    }

    void PhysicsBodyImpl::updateJolt(const glm::mat4& mtx) noexcept
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

    std::string_view PhysicsBodyImpl::getMotionTypeName(MotionType motion) noexcept
    {
        return StringUtils::getEnumName(motion);
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

    PhysicsSystemImpl& PhysicsBodyImpl::getSystemImpl() noexcept
    {
        return _system->getImpl();
    }

    PhysicsShape PhysicsBodyImpl::getShape() const noexcept
    {
        return _shape.value_or(BoundingBox{});
    }

    PhysicsBodyImpl::MotionType PhysicsBodyImpl::getMotionType() const noexcept
    {
        auto joltType = getBodyInterface()->GetMotionType(_bodyId);
        switch (joltType)
        {
        case JPH::EMotionType::Dynamic:
            return Definition::Dynamic;
            break;
        case JPH::EMotionType::Static:
            return Definition::Static;
            break;
        case JPH::EMotionType::Kinematic:
            return Definition::Kinematic;
            break;
        }
        return Definition::Static;
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

    void PhysicsBodyImpl::setPosition(const glm::vec3& pos) noexcept
    {
        auto jpos = JoltUtils::convert(pos);
        getBodyInterface()->SetPosition(_bodyId, jpos, JPH::EActivation::Activate);
    }

    glm::vec3 PhysicsBodyImpl::getPosition() const noexcept
    {
        auto iface = getBodyInterface();
        if (!iface)
        {
            return glm::vec3{ 0.F };
        }
        return JoltUtils::convert(iface->GetPosition(_bodyId));
    }

    void PhysicsBodyImpl::setRotation(const glm::quat& rot) noexcept
    {
        getBodyInterface()->SetRotation(_bodyId, JoltUtils::convert(rot), JPH::EActivation::Activate);
    }

    glm::quat PhysicsBodyImpl::getRotation() const noexcept
    {
        auto iface = getBodyInterface();
        if (!iface)
        {
            return glm::quat{ 0.F, 0.F, 0.F, 0.F };
        }
        return JoltUtils::convert(iface->GetRotation(_bodyId));
    }

    void PhysicsBodyImpl::setLinearVelocity(const glm::vec3& velocity) noexcept
    {
        getBodyInterface()->SetLinearVelocity(_bodyId, JoltUtils::convert(velocity));
    }

    glm::vec3 PhysicsBodyImpl::getLinearVelocity() const noexcept
    {
        auto iface = getBodyInterface();
        if (!iface)
        {
			return glm::vec3{ 0.F };
        }
        return JoltUtils::convert(iface->GetLinearVelocity(_bodyId));
    }

    void PhysicsBodyImpl::setAngularVelocity(const glm::vec3& velocity) noexcept
    {
        getBodyInterface()->SetAngularVelocity(_bodyId, JoltUtils::convert(velocity));
    }

    glm::vec3 PhysicsBodyImpl::getAngularVelocity() const noexcept
    {
        auto iface = getBodyInterface();
        if (!iface)
        {
            return glm::vec3{ 0.F };
        }
        return JoltUtils::convert(iface->GetAngularVelocity(_bodyId));
    }

    BoundingBox PhysicsBodyImpl::getLocalBounds() const noexcept
    {
        auto iface = getBodyInterface();
        if (!iface)
        {
            return {};
        }
        auto shape = iface->GetShape(_bodyId);
        if (!shape)
        {
            return {};
        }
        return JoltUtils::convert(shape->GetLocalBounds());
    }

    BoundingBox PhysicsBodyImpl::getWorldBounds() const noexcept
    {
        auto iface = getBodyInterface();
        if(!iface)
        {
            return {};
		}
        auto shape = iface->GetShape(_bodyId);
        if (!shape)
        {
            return {};
        }
        auto trans = iface->GetCenterOfMassTransform(_bodyId);
        static const JPH::Vec3 scale(1.F, 1.F, 1.F);
        return JoltUtils::convert(shape->GetWorldSpaceBounds(trans, scale));
    }

    float PhysicsBodyImpl::getInverseMass() const noexcept
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

    void PhysicsBodyImpl::setInverseMass(float v) noexcept
    {
        auto lockFace = getBodyLockInterface();
        if (!lockFace)
        {
            return;
        }
        const JPH::BodyLockWrite lock(lockFace.value(), _bodyId);
        if (lock.Succeeded())
        {
            auto& body = lock.GetBody();
            body.GetMotionProperties()->SetInverseMass(v);
        }
    }

    bool PhysicsBodyImpl::isActive() const noexcept
    {
        if (_bodyId.IsInvalid())
        {
            return false;
        }
        return getBodyInterface()->IsActive(_bodyId);
    }

    void PhysicsBodyImpl::activate() noexcept
    {
        getBodyInterface()->ActivateBody(_bodyId);
    }

    void PhysicsBodyImpl::deactivate() noexcept
    {
        getBodyInterface()->DeactivateBody(_bodyId);
    }

    bool PhysicsBodyImpl::isEnabled() const noexcept
    {
        if (_bodyId.IsInvalid())
        {
            return false;
        }
        return getBodyInterface()->IsAdded(_bodyId);
    }

    void PhysicsBodyImpl::setEnabled(bool enabled) noexcept
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

    void PhysicsBodyImpl::addTorque(const glm::vec3& torque) noexcept
    {
        getBodyInterface()->AddTorque(_bodyId, JoltUtils::convert(torque));
    }

    void PhysicsBodyImpl::addForce(const glm::vec3& force) noexcept
    {
        getBodyInterface()->AddForce(_bodyId, JoltUtils::convert(force));
    }

    void PhysicsBodyImpl::addImpulse(const glm::vec3& impulse) noexcept
    {
        getBodyInterface()->AddImpulse(_bodyId, JoltUtils::convert(impulse));
    }

    void PhysicsBodyImpl::move(const glm::vec3& pos, const glm::quat& rot, float deltaTime) noexcept
    {
        auto minTime = getSystemImpl().getFixedDeltaTime();
        if (deltaTime < minTime)
        {
            deltaTime = minTime;
        }
        getBodyInterface()->MoveKinematic(_bodyId,
            JoltUtils::convert(pos),
            JoltUtils::convert(rot),
            deltaTime);
    }

    void PhysicsBodyImpl::movePosition(const glm::vec3& pos, float deltaTime) noexcept
    {
        auto iface = getBodyInterface();
        auto rot = iface->GetRotation(_bodyId);

        auto minTime = getSystemImpl().getFixedDeltaTime();
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

    expected<void, std::string> PhysicsBodyImpl::onCollisionEnter(PhysicsBody& other, const Collision& collision) noexcept
    {
        for (auto& listener : _listeners)
        {
            auto result = listener.onCollisionEnter(_body.value(), other, collision);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    expected<void, std::string> PhysicsBodyImpl::onCollisionStay(PhysicsBody& other, const Collision& collision) noexcept
    {
        for (auto& listener : _listeners)
        {
            auto result = listener.onCollisionStay(_body.value(), other, collision);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    expected<void, std::string> PhysicsBodyImpl::onCollisionExit(PhysicsBody& other) noexcept
    {
        for (auto& listener : _listeners)
        {
            auto result = listener.onCollisionExit(_body.value(), other);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    PhysicsBody::PhysicsBody() noexcept
        : _impl{ std::make_unique<PhysicsBodyImpl>(createDefinition()) }
    {
    }

    PhysicsBody::PhysicsBody(const PhysicsShape& shape, MotionType motion) noexcept
    {
        auto def = createDefinition();
		def.set_motion(motion);
        *def.mutable_shape() = darmok::convert<protobuf::PhysicsShape>(shape);
        _impl = std::make_unique<PhysicsBodyImpl>(def);
    }

    PhysicsBody::PhysicsBody(const Definition& def) noexcept
        : _impl{ std::make_unique<PhysicsBodyImpl>(def) }
    {
    }

    PhysicsBody::PhysicsBody(const CharacterDefinition& def) noexcept
        : _impl{ std::make_unique<PhysicsBodyImpl>(def) }
    {
    }

    PhysicsBody::~PhysicsBody() noexcept = default;
    PhysicsBody::PhysicsBody(PhysicsBody&& other) noexcept = default;
    PhysicsBody& PhysicsBody::operator=(PhysicsBody&& other) noexcept = default;

    PhysicsBody::Definition PhysicsBody::createDefinition() noexcept
    {
        Definition def;
        def.set_mass(1.0F);
		def.set_motion(Definition::Dynamic);
		def.set_inertia_factor(1.0F);
		def.set_friction(0.2F);
		def.set_gravity_factor(1.0F);
        def.set_trigger(false);
        *def.mutable_shape() = createShapeDefinition();
		return def;
    }

    PhysicsBody::ShapeDefinition PhysicsBody::createShapeDefinition() noexcept
    {
        ShapeDefinition shape;
        auto& sphere = *shape.mutable_sphere();
        sphere.set_radius(0.5F);
        return shape;
    }

    PhysicsBody::ShapeDefinition PhysicsBody::createCharacterShapeDefinition() noexcept
    {
        ShapeDefinition shape;
        auto& capsule = *shape.mutable_capsule();
        capsule.set_cylinder_height(1.F);
        capsule.set_radius(0.25F);
        capsule.mutable_origin()->set_y(0.75F);
        return shape;
    }

    Plane::Definition PhysicsBody::createSupportingPlaneDefinition() noexcept
    {
		Plane::Definition def;
        def.mutable_normal()->set_y(1.0F);
        def.set_distance(-1.0e10f);
        return def;
    }

    PhysicsBody::CharacterDefinition PhysicsBody::createCharacterDefinition() noexcept
    {
        CharacterDefinition def;
		*def.mutable_shape() = createCharacterShapeDefinition();
		*def.mutable_supporting_plane() = createSupportingPlaneDefinition();
        def.mutable_up()->set_y(1.0F);
        def.set_max_slope_angle(glm::radians(50.F));

		def.set_mass(80.F);
		def.set_friction(1.F);
        def.set_gravity_factor(1.F);
		def.set_max_separation_distance(0.1F);
        return def;
    }

    PhysicsShape PhysicsBody::getShape() const noexcept
    {
        return _impl->getShape();
    }

    PhysicsBody::MotionType PhysicsBody::getMotionType() const noexcept
    {
        return _impl->getMotionType();
    }

    BoundingBox PhysicsBody::getLocalBounds() const noexcept
    {
        return _impl->getLocalBounds();
    }

    BoundingBox PhysicsBody::getWorldBounds() const noexcept
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

    PhysicsBody& PhysicsBody::setPosition(const glm::vec3& pos) noexcept
    {
        _impl->setPosition(pos);
        return *this;
    }

    glm::vec3 PhysicsBody::getPosition() const noexcept
    {
        return _impl->getPosition();
    }

    PhysicsBody& PhysicsBody::setRotation(const glm::quat& rot) noexcept
    {
        _impl->setRotation(rot);
        return *this;
    }

    glm::quat PhysicsBody::getRotation() const noexcept
    {
        return _impl->getRotation();
    }

    PhysicsBody& PhysicsBody::setLinearVelocity(const glm::vec3& velocity) noexcept
    {
        _impl->setLinearVelocity(velocity);
        return *this;
    }

    glm::vec3 PhysicsBody::getLinearVelocity() const noexcept
    {
        return _impl->getLinearVelocity();
    }

    float PhysicsBody::getInverseMass() const noexcept
    {
        return _impl->getInverseMass();
    }

    PhysicsBody& PhysicsBody::setInverseMass(float v) noexcept
    {
        _impl->setInverseMass(v);
        return *this;
    }

    bool PhysicsBody::isActive() const noexcept
    {
        return _impl->isActive();
    }

    PhysicsBody& PhysicsBody::activate() noexcept
    {
        _impl->activate();
        return *this;
    }

    PhysicsBody& PhysicsBody::deactivate() noexcept
    {
        _impl->deactivate();
        return *this;
    }

    bool PhysicsBody::isEnabled() const noexcept
    {
        return _impl->isEnabled();
    }

    PhysicsBody& PhysicsBody::setEnabled(bool enabled) noexcept
    {
        _impl->setEnabled(enabled);
        return *this;
    }

    PhysicsBody& PhysicsBody::addTorque(const glm::vec3& torque) noexcept
    {
        _impl->addTorque(torque);
        return *this;
    }

    PhysicsBody& PhysicsBody::addForce(const glm::vec3& force) noexcept
    {
        _impl->addForce(force);
        return *this;
    }

    PhysicsBody& PhysicsBody::addImpulse(const glm::vec3& impulse) noexcept
    {
        _impl->addImpulse(impulse);
        return *this;
    }

    PhysicsBody& PhysicsBody::move(const glm::vec3& pos, const glm::quat& rot, float deltaTime) noexcept
    {
        _impl->move(pos, rot, deltaTime);
        return *this;
    }

    PhysicsBody& PhysicsBody::movePosition(const glm::vec3& pos, float deltaTime) noexcept
    {
        _impl->movePosition(pos, deltaTime);
        return *this;
    }

    expected<void, std::string> PhysicsBody::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
        auto entity = context.getScene().getEntity(*this);
        return _impl->load(def, entity);
    }

    expected<void, std::string> PhysicsBody::load(const CharacterDefinition& def, IComponentLoadContext& context) noexcept
    {
        auto entity = context.getScene().getEntity(*this);
        return _impl->load(def, entity);
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
        ss << StringUtils::join(", ", contacts) << "))";
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

namespace darmok
{
    physics3d::PhysicsShape Converter<physics3d::PhysicsShape, physics3d::protobuf::PhysicsShape>::run(const physics3d::protobuf::PhysicsShape& v) noexcept
    {
        if (v.has_cube())
        {
            return convert<Cube>(v.cube());
        }
        else if (v.has_sphere())
        {
            return convert<Sphere>(v.sphere());
        }
        else if (v.has_capsule())
        {
            return convert<Capsule>(v.capsule());
        }
        else if (v.has_polygon())
        {
            return convert<Polygon>(v.polygon());
        }
        else if (v.has_bounding_box())
        {
            return convert<BoundingBox>(v.bounding_box());
        }
        return Cube{};
    }

    
    physics3d::protobuf::PhysicsShape Converter<physics3d::protobuf::PhysicsShape, physics3d::PhysicsShape>::run(const physics3d::PhysicsShape& v) noexcept
    {
        physics3d::protobuf::PhysicsShape def;
        if (auto cubePtr = std::get_if<Cube>(&v))
        {
            *def.mutable_cube() = convert<protobuf::Cube>(*cubePtr);
        }
        else if (auto spherePtr = std::get_if<Sphere>(&v))
        {
            *def.mutable_sphere() = convert<protobuf::Sphere>(*spherePtr);
        }
        else if (auto capsPtr = std::get_if<Capsule>(&v))
        {
            *def.mutable_capsule() = convert<protobuf::Capsule>(*capsPtr);
        }
        else if (auto polyPtr = std::get_if<Polygon>(&v))
        {
            *def.mutable_polygon() = convert<protobuf::Polygon>(*polyPtr);
        }
        else if (auto bboxPtr = std::get_if<BoundingBox>(&v))
        {
            *def.mutable_bounding_box() = convert<protobuf::BoundingBox>(*bboxPtr);
        }
        return def;
    }
}
