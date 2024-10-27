#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <deque>
#include <unordered_map>
#include <expected>
#include <darmok/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/character.hpp>
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

    struct JoltTransform
    {
        JPH::Vec3 position = JPH::Vec3(0, 0, 0);
        JPH::Quat rotation = JPH::Quat::sIdentity();
        float scale = 1.F;
    };

    struct JoltUtils final
    {
        using Shape = PhysicsShape;
        static JPH::Vec3 convert(const glm::vec3& v) noexcept;
        static glm::vec3 convert(const JPH::Vec3& v) noexcept;
        static JPH::Vec3 convertSize(const glm::vec3& v) noexcept;
        static JPH::Vec4 convert(const glm::vec4& v) noexcept;
        static glm::vec4 convert(const JPH::Vec4& v) noexcept;
        static JPH::Mat44 convert(const glm::mat4& v) noexcept;
        static glm::mat4 convert(const JPH::Mat44& v) noexcept;
        static JPH::Quat convert(const glm::quat& v) noexcept;
        static glm::quat convert(const JPH::Quat& v) noexcept;
        static Color convert(const JPH::Color& v) noexcept;
        static glm::vec2 convert(const JPH::Float2& v) noexcept;
        static glm::vec3 convert(const JPH::Float3& v) noexcept;
        static glm::vec4 convert(const JPH::Float4& v) noexcept;
        static JPH::Triangle convert(const Triangle& v) noexcept;
        static JPH::TriangleList convert(const Polygon& v) noexcept;
        static JPH::Plane convert(const Plane& v) noexcept;
        static JPH::AABox convert(const BoundingBox& v) noexcept;
        static BoundingBox convert(const JPH::AABox& v) noexcept;
        static RaycastHit convert(const JPH::RayCastResult& result, const JPH::RRayCast& rayCast, PhysicsBody& rb) noexcept;
        static std::expected<JoltTransform, std::string> convertTransform(const glm::mat4& mat) noexcept;
        static JPH::ShapeRefC convert(const Shape& shape, float scale = 1.F);
        static glm::vec3 getOrigin(const Shape& shape) noexcept;
        static float getConvexRadius(const glm::vec3& size) noexcept;

        template<typename T>
        static bool removeRefVector(std::vector<OptionalRef<T>>& vector, T& elm) noexcept
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

    class JoltJobSystemTaskflow : public JPH::JobSystemWithBarrier
    {
    public:
        JoltJobSystemTaskflow() noexcept;
        ~JoltJobSystemTaskflow();

        void init(tf::Executor& executor, JPH::uint maxBarriers = JPH::cMaxPhysicsBarriers) noexcept;
        void shutdown();

        const tf::Taskflow& getTaskflow() const;

        int GetMaxConcurrency() const override;
        JobHandle CreateJob(const char* name, JPH::ColorArg color, const JobFunction& jobFunction, JPH::uint32 numDependencies = 0) override;
        void QueueJob(Job* job) override;
        void QueueJobs(Job** jobs, JPH::uint numJobs) override;
        void FreeJob(Job* job) override;
    private:
        OptionalRef<tf::Executor> _taskExecutor;
        tf::Taskflow _taskflow;
        tf::Future<void> _future;
        static const std::string _prefix;
    };

    class JoltBroadPhaseLayerInterface final : public JPH::BroadPhaseLayerInterface
    {
    public:
        using Config = PhysicsLayerConfig;
        JoltBroadPhaseLayerInterface(const Config& config) noexcept;
        JPH::uint GetNumBroadPhaseLayers() const noexcept override;
        JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const noexcept override;
        virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const
#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
            override
#endif
        ;
    private:
        Config _config;
    };

    class JoltObjectVsBroadPhaseLayerFilter final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        using Config = PhysicsLayerConfig;
        JoltObjectVsBroadPhaseLayerFilter(const Config& config);
        bool ShouldCollide(JPH::ObjectLayer layer1, JPH::BroadPhaseLayer layer2) const noexcept override;
    private:
        Config _config;
    };

    class JoltObjectLayerPairFilter final : public JPH::ObjectLayerPairFilter
    {
    public:
        using Config = PhysicsLayerConfig;
        JoltObjectLayerPairFilter(const Config& config);
        bool ShouldCollide(JPH::ObjectLayer object1, JPH::ObjectLayer object2) const noexcept override;
    private:
        Config _config;
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
        using Config = PhysicsSystemConfig;
        using Shape = PhysicsShape;

        PhysicsSystemImpl(PhysicsSystem& system, const Config& config, OptionalRef<bx::AllocatorI> alloc = nullptr) noexcept;
        void init(Scene& scene, App& app) noexcept;
        void shutdown() noexcept;
        void update(float deltaTime);

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

        const tf::Taskflow& getTaskflow() const;
        const Config& getConfig() const noexcept;
        OptionalRef<Scene> getScene() const noexcept;
        OptionalRef<JPH::PhysicsSystem> getJolt() noexcept;
        OptionalRef<const JPH::PhysicsSystem> getJolt() const noexcept;
        JPH::BodyInterface& getBodyInterface() const noexcept;
        const JPH::BodyLockInterface& getBodyLockInterface() const noexcept;
        JoltTempAllocator& getTempAllocator() noexcept;
        glm::vec3 getGravity() noexcept;

        void setRootTransform(OptionalRef<Transform> root) noexcept;
        OptionalRef<Transform> getRootTransform() noexcept;

        void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
        void OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const  JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
        void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

        OptionalRef<PhysicsBody> getPhysicsBody(const JPH::BodyID& bodyId) const noexcept;
        static OptionalRef<PhysicsBody> getPhysicsBody(const JPH::Body& body) noexcept;

        std::optional<RaycastHit> raycast(const Ray& ray, float maxDistance, LayerMask layers = kAllLayers) const noexcept;
        std::vector<RaycastHit> raycastAll(const Ray& ray, float maxDistance, LayerMask layers = kAllLayers) const noexcept;
        void activateBodies(const BoundingBox& bbox, LayerMask layers = kAllLayers) noexcept;

        JoltTransform loadTransform(Transform& trans);
        void updateTransform(Transform& trans, const JPH::Mat44& mtx) noexcept;
    private:
        PhysicsSystem& _system;
        OptionalRef<Scene> _scene;
        float _deltaTimeRest;
        Config _config;
        OptionalRef<Transform> _root;
        JoltBroadPhaseLayerInterface _broadPhaseLayer;
        JoltObjectVsBroadPhaseLayerFilter _objVsBroadPhaseLayerFilter;
        JoltObjectLayerPairFilter _objLayerPairFilter;
        JoltTempAllocator _alloc;
        std::unique_ptr<JPH::PhysicsSystem> _joltSystem;
        JoltJobSystemTaskflow _jobSystem;
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

        std::expected<JoltTransform, std::string> tryLoadTransform(Transform& trans) noexcept;

        void processPendingCollisionEvents();
        static Collision createCollision(const JPH::ContactManifold& manifold) noexcept;
        
        void onRigidbodyConstructed(EntityRegistry& registry, Entity entity) noexcept;
        void onRigidbodyDestroyed(EntityRegistry& registry, Entity entity);
        void onCharacterConstructed(EntityRegistry& registry, Entity entity) noexcept;
        void onCharacterDestroyed(EntityRegistry& registry, Entity entity);

        static std::string getUpdateErrorString(JPH::EPhysicsUpdateError err) noexcept;

        void onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision);
        void onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision);
        void onCollisionExit(PhysicsBody& body1, PhysicsBody& body2);
    };

    class PhysicsBodyImpl final
    {
    public:
        using Config = PhysicsBodyConfig;
        using Shape = PhysicsShape;
        using MotionType = PhysicsBodyMotionType;

        PhysicsBodyImpl(const Config& config) noexcept;
        PhysicsBodyImpl(const CharacterConfig& config) noexcept;
        ~PhysicsBodyImpl();
        void init(PhysicsBody& body, PhysicsSystem& system) noexcept;
        void shutdown(bool systemShutdown = false);
        void update(Entity entity, float deltaTime);
        std::string toString() const noexcept;

        OptionalRef<PhysicsSystem> getSystem() const noexcept;

        Shape getShape() const noexcept;
        BoundingBox getLocalBounds() const;
        BoundingBox getWorldBounds() const;

        MotionType getMotionType() const noexcept;
        const JPH::BodyID& getBodyId() const noexcept;

        bool isGrounded() const noexcept;
        GroundState getGroundState() const noexcept;

        void setPosition(const glm::vec3& pos);
        glm::vec3 getPosition() const;
        void setRotation(const glm::quat& rot);
        glm::quat getRotation() const;
        void setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity() const;
        void setAngularVelocity(const glm::vec3& velocity);
        glm::vec3 getAngularVelocity() const;
        float getInverseMass() const;
        void setInverseMass(float v);

        bool isActive() const;
        void activate();
        void deactivate();

        bool isEnabled() const;
        void setEnabled(bool enabled);

        void addTorque(const glm::vec3& torque);
        void addForce(const glm::vec3& force);
        void addImpulse(const glm::vec3& impulse);
        void move(const glm::vec3& pos, const glm::quat& rot, float deltaTime );
        void movePosition(const glm::vec3& pos, float deltaTime);

        void addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept;
        void addListener(ICollisionListener& listener) noexcept;
        bool removeListener(const ICollisionListener& listener) noexcept;
        size_t removeListeners(const ICollisionListenerFilter& filter) noexcept;

        void onCollisionEnter(PhysicsBody& other, const Collision& collision);
        void onCollisionStay(PhysicsBody& other, const Collision& collision);
        void onCollisionExit(PhysicsBody& other);

        static const std::string& getMotionTypeName(MotionType motion) noexcept;

    private:
        static const std::unordered_map<MotionType, std::string> _motionTypeNames;
        OptionalRef<JPH::BodyInterface> getBodyInterface() const noexcept;
        OptionalRef<const JPH::BodyLockInterface> getBodyLockInterface() const noexcept;
        JPH::BodyID createBody(const Config& config, const JoltTransform& trans);
        JPH::BodyID createCharacter(const CharacterConfig& config, const JoltTransform& trans);
        bool tryCreateBody(OptionalRef<Transform> transform);
        PhysicsSystemImpl& getSystemImpl();
        void updateJolt(const glm::mat4& worldMatrix);

        OptionalRef<PhysicsBody> _body;
        OptionalRef<PhysicsSystem> _system;
        JPH::BodyID _bodyId;        
        OwnRefCollection<ICollisionListener> _listeners;
        std::optional<std::variant<Config, CharacterConfig>> _initConfig;
        JPH::Ref<JPH::Character> _character;
        float _maxSepDistance;
        std::optional<PhysicsShape> _shape;
    };
}