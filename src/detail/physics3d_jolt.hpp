#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <deque>
#include <unordered_map>
#include <darmok/expected.hpp>
#include <darmok/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/physics3d_character.hpp>
#include <darmok/glm.hpp>
#include <darmok/collection.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EPhysicsUpdateError.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Core/JobSystemWithBarrier.h>

#include <taskflow/taskflow.hpp>

namespace bx
{
    struct AllocatorI;
}

namespace bgfx
{
    struct Encoder;
}

namespace JPH
{
    class PhysicsSystem;
    class BroadPhaseLayer;
    class BodyInterface;
    class BodyLockInterface;
    class Character;
    class Float2;
    struct RRayCast;
}

namespace darmok
{
    class Transform;
    class Scene;
    class App;
}

namespace darmok::physics3d
{
    class IPhysicsUpdater;
    class PhysicsBody;

    struct JoltTransform final
    {
        JPH::Vec3 position = JPH::Vec3(0, 0, 0);
        JPH::Quat rotation = JPH::Quat::sIdentity();
        float scale = 1.F;
    };

    namespace JoltUtils
    {
        using ShapeDefinition = protobuf::PhysicsShape;
        JPH::Vec3 convert(const glm::vec3& v) noexcept;
        glm::vec3 convert(const JPH::Vec3& v) noexcept;
        JPH::Vec3 convertSize(const glm::vec3& v) noexcept;
        JPH::Vec4 convert(const glm::vec4& v) noexcept;
        glm::vec4 convert(const JPH::Vec4& v) noexcept;
        JPH::Mat44 convert(const glm::mat4& v) noexcept;
        glm::mat4 convert(const JPH::Mat44& v) noexcept;
        JPH::Quat convert(const glm::quat& v) noexcept;
        glm::quat convert(const JPH::Quat& v) noexcept;
        Color convert(const JPH::Color& v) noexcept;
        glm::vec2 convert(const JPH::Float2& v) noexcept;
        glm::vec3 convert(const JPH::Float3& v) noexcept;
        glm::vec4 convert(const JPH::Float4& v) noexcept;
        JPH::Triangle convert(const Triangle& v) noexcept;
        JPH::TriangleList convert(const Polygon& v) noexcept;
        JPH::Plane convert(const Plane& v) noexcept;
        JPH::AABox convert(const BoundingBox& v) noexcept;
        BoundingBox convert(const JPH::AABox& v) noexcept;
        JPH::RRayCast convert(const Ray& v) noexcept;
        RaycastHit convert(const JPH::RayCastResult& result, const Ray& ray, PhysicsBody& rb) noexcept;
        expected<JoltTransform, std::string> convertTransform(const glm::mat4& mat) noexcept;
        expected<JPH::ShapeRefC, std::string> convert(const PhysicsShape& shape, float scale = 1.F) noexcept;
        float getConvexRadius(const glm::vec3& size) noexcept;

        template<typename T>
        bool removeRefVector(std::vector<OptionalRef<T>>& vector, T& elm) noexcept
        {
            auto ptr = &elm;
            auto itr = std::find_if(vector.begin(), vector.end(), [ptr](auto& ref) { return ref.ptr() == ptr; });
            if (itr == vector.end())
            {
                return false;
            }
            vector.erase(itr);
            return true;
        }
    };

    class JoltJobSystemTaskflow final : public JPH::JobSystemWithBarrier
    {
    public:
        JoltJobSystemTaskflow() noexcept;
        ~JoltJobSystemTaskflow() noexcept;

        void init(tf::Executor& executor, JPH::uint maxBarriers = JPH::cMaxPhysicsBarriers) noexcept;
        void shutdown() noexcept;

        const tf::Taskflow& getTaskflow() const noexcept;

        int GetMaxConcurrency() const noexcept override;
        JobHandle CreateJob(const char* name, JPH::ColorArg color, const JobFunction& jobFunction, JPH::uint32 numDependencies = 0) noexcept override;
        void QueueJob(Job* job) noexcept override;
        void QueueJobs(Job** jobs, JPH::uint numJobs) noexcept override;
        void FreeJob(Job* job) noexcept override;
    private:
        OptionalRef<tf::Executor> _taskExecutor;
        tf::Taskflow _taskflow;
        tf::Future<void> _future;
        static const std::string _prefix;
    };

    class JoltBroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
    {
    public:
        using Definition = protobuf::PhysicsSystem;
        JoltBroadPhaseLayerInterface(const Definition& def) noexcept;
        JPH::uint GetNumBroadPhaseLayers() const noexcept override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const noexcept override;
        virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const noexcept
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            override
#endif
        ;
    private:
        ConstPhysicsSystemDefinitionWrapper _def;
    };

    class JoltObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        using Definition = protobuf::PhysicsSystem;
        JoltObjectVsBroadPhaseLayerFilter(const Definition& def) noexcept;
        bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const noexcept override;
    private:
        ConstPhysicsSystemDefinitionWrapper _def;
    };

    class JoltObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
    {
    public:
        using Definition = protobuf::PhysicsSystem;
        JoltObjectLayerPairFilter(const Definition& def) noexcept;
        bool ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const noexcept override;
    private:
        ConstPhysicsSystemDefinitionWrapper _def;
    };

    class JoltBroadPhaseLayerMaskFilter final : public JPH::BroadPhaseLayerFilter
    {
    public:
        JoltBroadPhaseLayerMaskFilter(BroadLayer layer = 0) noexcept;
        bool ShouldCollide(JPH::BroadPhaseLayer layer) const noexcept override;
    private:
        BroadLayer _layer;
    };

    class JoltObjectLayerMaskFilter final : public JPH::ObjectLayerFilter
    {
    public:
        JoltObjectLayerMaskFilter(LayerMask layers = kAllLayers) noexcept;
        bool ShouldCollide(JPH::ObjectLayer layer) const noexcept override;
    private:
        LayerMask _layers;
    };

    class JoltTempAllocator final : public JPH::TempAllocator
    {
    public:
        JoltTempAllocator(OptionalRef<bx::AllocatorI> alloc) noexcept;
        void* Allocate(JPH::uint size) noexcept override;
        void Free(void* address, JPH::uint size) noexcept override;
    private:
        OptionalRef<bx::AllocatorI> _alloc;
    };

    template <class ResultTypeArg, class TraitsType>
    class JoltVectorCollisionCollector final : public JPH::CollisionCollector<ResultTypeArg, TraitsType>
    {
    public:
        void AddHit(const ResultTypeArg& result) noexcept override
        {
            _hits.push_back(result);
        }

        const std::vector<ResultTypeArg>& getHits() const noexcept
        {
            return _hits;
        }
    private:
        std::vector<ResultTypeArg> _hits;
    };

    using JoltVectorCastRayCollector = JoltVectorCollisionCollector<JPH::RayCastResult, JPH::CollisionCollectorTraitsCastRay>;    

    class PhysicsSystemImpl final : public JPH::ContactListener
    {
    public:
        using Definition = protobuf::PhysicsSystem;
        using Shape = protobuf::PhysicsShape;

        PhysicsSystemImpl(PhysicsSystem& system, const Definition& def, OptionalRef<bx::AllocatorI> alloc = nullptr) noexcept;
        expected<void, std::string> load(const Definition& def, IComponentLoadContext& context) noexcept;
        expected<void, std::string> init(Scene& scene, App& app) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;

        bool isPaused() const noexcept;
        void setPaused(bool paused) noexcept;

        bool isValidEntity(Entity entity) noexcept;

        void addUpdater(std::unique_ptr<IPhysicsUpdater>&& updater) noexcept;
        void addUpdater(IPhysicsUpdater& updater) noexcept;
        bool removeUpdater(const IPhysicsUpdater& updater) noexcept;
        size_t removeUpdaters(const IPhysicsUpdaterFilter& filter) noexcept;

        void addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept;
        void addListener(ICollisionListener& listener) noexcept;
        bool removeListener(const ICollisionListener& listener) noexcept;
        size_t removeListeners(const ICollisionListenerFilter& filter) noexcept;

        float getFixedDeltaTime() const noexcept;
        const tf::Taskflow& getTaskflow() const noexcept;
        const Definition& getDefinition() const noexcept;
        OptionalRef<Scene> getScene() const noexcept;
        OptionalRef<JPH::PhysicsSystem> getJolt() noexcept;
        OptionalRef<const JPH::PhysicsSystem> getJolt() const noexcept;
        JPH::BodyInterface& getBodyInterface() const noexcept;
        const JPH::BodyLockInterface& getBodyLockInterface() const noexcept;
        JoltTempAllocator& getTempAllocator() noexcept;
        glm::vec3 getGravity() noexcept;

        void setRootTransform(OptionalRef<Transform> root) noexcept;
        OptionalRef<Transform> getRootTransform() noexcept;

        void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) noexcept override;
        void OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const  JPH::ContactManifold& manifold, JPH::ContactSettings& settings) noexcept override;
        void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) noexcept override;

        OptionalRef<PhysicsBody> getPhysicsBody(const JPH::BodyID& bodyId) const noexcept;
        static OptionalRef<PhysicsBody> getPhysicsBody(const JPH::Body& body) noexcept;

        std::optional<RaycastHit> raycast(const Ray& ray, LayerMask layers = kAllLayers) const noexcept;
        std::vector<RaycastHit> raycastAll(const Ray& ray, LayerMask layers = kAllLayers) const noexcept;
        void activateBodies(const BoundingBox& bbox, LayerMask layers = kAllLayers) noexcept;

        expected<JoltTransform, std::string> tryLoadTransform(Transform& trans) noexcept;
        void updateTransform(Transform& trans, const JPH::Mat44& mtx) noexcept;
    private:
        PhysicsSystem& _system;
        OptionalRef<Scene> _scene;
        OptionalRef<tf::Executor> _taskExecutor;
        float _deltaTimeRest;
        Definition _def;
        OptionalRef<Transform> _root;
        JoltBroadPhaseLayerInterface _broadPhaseLayer;
        JoltObjectVsBroadPhaseLayerFilter _objVsBroadPhaseLayerFilter;
        JoltObjectLayerPairFilter _objLayerPairFilter;
        JoltTempAllocator _alloc;
        std::unique_ptr<JPH::PhysicsSystem> _joltSystem;
        std::optional<JoltJobSystemTaskflow> _jobSystem;
        OwnRefCollection<IPhysicsUpdater> _updaters;
        OwnRefCollection<ICollisionListener> _listeners;
        bool _paused;

        enum class CollisionEventType
        {
            Enter, Stay, Exit
        };

        struct CollisionEvent
        {
            CollisionEventType type;
            JPH::BodyID bodyID1;
            JPH::BodyID bodyID2;
            Collision collision;
        };

        mutable std::mutex _collisionMutex;
        std::deque<CollisionEvent> _pendingCollisionEvents;

        expected<void, std::string> processPendingCollisionEvents() noexcept;
        static Collision createCollision(const JPH::ContactManifold& manifold) noexcept;
        
        expected<void, std::string> onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept;
        expected<void, std::string> onRigidbodyDestroyed(EntityRegistry& registry, Entity entity) noexcept;
        expected<void, std::string> onCharacterConstructed(EntityRegistry& registry, Entity entity) noexcept;
        expected<void, std::string> onCharacterDestroyed(EntityRegistry& registry, Entity entity) noexcept;

        static std::string_view getUpdateErrorString(JPH::EPhysicsUpdateError err) noexcept;

        expected<void, std::string> onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) noexcept;
        expected<void, std::string> onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) noexcept;
        expected<void, std::string> onCollisionExit(PhysicsBody& body1, PhysicsBody& body2) noexcept;

        JoltBroadPhaseLayerMaskFilter createBroadPhaseLayerFilter(LayerMask layer) const noexcept;
        expected<void, std::string> doInit() noexcept;
    };

    class PhysicsBodyImpl final
    {
    public:
        using Definition = protobuf::PhysicsBody;
		using CharacterDefinition = protobuf::Character;
        using ShapeDefinition = PhysicsBody::ShapeDefinition;
        using MotionType = Definition::MotionType;

        PhysicsBodyImpl(const Definition& def) noexcept;
        PhysicsBodyImpl(const CharacterDefinition& def) noexcept;
        ~PhysicsBodyImpl() noexcept;
        void init(PhysicsBody& body, PhysicsSystem& system) noexcept;
        void shutdown(bool systemShutdown = false) noexcept;
        expected<void, std::string> update(Entity entity, float deltaTime) noexcept;
        std::string toString() const noexcept;		

        OptionalRef<PhysicsSystem> getSystem() const noexcept;

        PhysicsShape getShape() const noexcept;
        BoundingBox getLocalBounds() const noexcept;
        BoundingBox getWorldBounds() const noexcept;

        MotionType getMotionType() const noexcept;
        const JPH::BodyID& getBodyId() const noexcept;

        bool isGrounded() const noexcept;
        GroundState getGroundState() const noexcept;

        void setPosition(const glm::vec3& pos) noexcept;
        glm::vec3 getPosition() const noexcept;
        void setRotation(const glm::quat& rot) noexcept;
        glm::quat getRotation() const noexcept;
        void setLinearVelocity(const glm::vec3& velocity) noexcept;
        glm::vec3 getLinearVelocity() const noexcept;
        void setAngularVelocity(const glm::vec3& velocity) noexcept;
        glm::vec3 getAngularVelocity() const noexcept;
        float getInverseMass() const noexcept;
        void setInverseMass(float v) noexcept;

        bool isActive() const noexcept;
        void activate() noexcept;
        void deactivate() noexcept;

        bool isEnabled() const noexcept;
        void setEnabled(bool enabled) noexcept;

        void addTorque(const glm::vec3& torque) noexcept;
        void addForce(const glm::vec3& force) noexcept;
        void addImpulse(const glm::vec3& impulse) noexcept;
        void move(const glm::vec3& pos, const glm::quat& rot, float deltaTime ) noexcept;
        void movePosition(const glm::vec3& pos, float deltaTime) noexcept;

        void addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept;
        void addListener(ICollisionListener& listener) noexcept;
        bool removeListener(const ICollisionListener& listener) noexcept;
        size_t removeListeners(const ICollisionListenerFilter& filter) noexcept;

        expected<void, std::string> onCollisionEnter(PhysicsBody& other, const Collision& collision) noexcept;
        expected<void, std::string> onCollisionStay(PhysicsBody& other, const Collision& collision) noexcept;
        expected<void, std::string> onCollisionExit(PhysicsBody& other) noexcept;

        static std::string_view getMotionTypeName(MotionType motion) noexcept;

        expected<void, std::string> load(const Definition& def, Entity entity) noexcept;
        expected<void, std::string> load(const CharacterDefinition& def, Entity entity) noexcept;

    private:
        OptionalRef<JPH::BodyInterface> getBodyInterface() const noexcept;
        OptionalRef<const JPH::BodyLockInterface> getBodyLockInterface() const noexcept;
        expected<JPH::BodyID, std::string> createBody(const Definition& def, const JoltTransform& trans) noexcept;
        expected<JPH::BodyID, std::string> createCharacter(const CharacterDefinition& def, const JoltTransform& trans) noexcept;
        expected<void, std::string> tryCreateBody(OptionalRef<Transform> transform) noexcept;
        PhysicsSystemImpl& getSystemImpl() noexcept;
        void updateJolt(const glm::mat4& worldMatrix) noexcept;
        void doShutdown() noexcept;
        expected<void, std::string> doLoad(Entity entity) noexcept;

        OptionalRef<PhysicsBody> _body;
        OptionalRef<PhysicsSystem> _system;
        JPH::BodyID _bodyId;        
        OwnRefCollection<ICollisionListener> _listeners;
        std::optional<std::variant<Definition, CharacterDefinition>> _def;
        JPH::Ref<JPH::Character> _character;
        float _maxSepDistance;
        std::optional<PhysicsShape> _shape;
    };
}