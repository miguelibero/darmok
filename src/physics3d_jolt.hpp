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
#include <darmok/character.hpp>

#include <darmok/glm.hpp>
#include "jolt.hpp"
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/EPhysicsUpdateError.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
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
    class Character;
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


    enum class BroadPhaseLayerType
    {
        Moving,
        NonMoving,
        Count
    };

    struct JoltUtils final
    {
        using Shape = PhysicsShape;
        static JPH::Vec3 convert(const glm::vec3& v) noexcept;
        static JPH::Vec3 convertPosition(const glm::vec3& pos, const Shape& shape) noexcept;
        static glm::vec3 convertPosition(const JPH::Vec3& pos, const Shape& shape) noexcept;
        static glm::vec3 convert(const JPH::Vec3& v) noexcept;
        static JPH::Vec3 convertSize(const glm::vec3& v) noexcept;
        static JPH::Vec4 convert(const glm::vec4& v) noexcept;
        static glm::vec4 convert(const JPH::Vec4& v) noexcept;
        static JPH::Mat44 convert(const glm::mat4& v) noexcept;
        static glm::mat4 convert(const JPH::Mat44& v) noexcept;
        static JPH::Quat convert(const glm::quat& v) noexcept;
        static glm::quat convert(const JPH::Quat& v) noexcept;
        static RaycastHit convert(const JPH::RayCastResult& result, PhysicsBody& rb) noexcept;

        static JPH::ObjectLayer convert(uint16_t layer, BroadPhaseLayerType bpl) noexcept;
        static std::pair<uint16_t, BroadPhaseLayerType> convert(JPH::ObjectLayer objLayer) noexcept;

        static std::pair<JPH::Vec3, JPH::Quat> convert(const Shape& shape, OptionalRef<const Transform> trans) noexcept;
        static JPH::ShapeRefC convert(const Shape& shape) noexcept;
        static glm::vec3 getOrigin(const Shape& shape) noexcept;
        static glm::mat4 convert(const JPH::Mat44& mat, const Shape& shape, const Transform& trans) noexcept;

        template<typename T>
        static void addRefVector(std::vector<OptionalRef<T>>& vector, T& elm)
        {
            auto ptr = &elm;
            auto itr = std::find_if(vector.begin(), vector.end(), [ptr](auto& ref) { return ref.ptr() == ptr; });
            if (itr == vector.end())
            {
                vector.emplace_back(elm);
            }
        }

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

    class JoltObjectLayerFilter final : public JPH::ObjectLayerFilter
    {
    public:
        JoltObjectLayerFilter(uint16_t layerMask) noexcept;
        bool ShouldCollide(JPH::ObjectLayer objLayer) const noexcept override;
    private:
        uint16_t _layerMask;
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
        PhysicsSystemImpl(const Config& config, OptionalRef<bx::AllocatorI> alloc = nullptr) noexcept;
        void init(Scene& scene, App& app) noexcept;
        void shutdown() noexcept;
        void update(float deltaTime);
        void addUpdater(IPhysicsUpdater& updater) noexcept;
        bool removeUpdater(IPhysicsUpdater& updater) noexcept;
        void addListener(ICollisionListener& listener) noexcept;
        bool removeListener(ICollisionListener& listener) noexcept;

        const Config& getConfig() const noexcept;
        OptionalRef<Scene> getScene() const noexcept;
        JPH::PhysicsSystem& getJolt() noexcept;
        JPH::BodyInterface& getBodyInterface() const noexcept;
        JoltTempAllocator& getTempAllocator() noexcept;
        glm::vec3 getGravity() noexcept;

        void OnContactAdded(const JPH::Body& body1, const JPH::Body& body2, const JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
        void OnContactPersisted(const JPH::Body& body1, const JPH::Body& body2, const  JPH::ContactManifold& manifold, JPH::ContactSettings& settings) override;
        void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

        OptionalRef<PhysicsBody> getPhysicsBody(const JPH::BodyID& bodyId) const noexcept;

        std::optional<RaycastHit> raycast(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;
        std::vector<RaycastHit> raycastAll(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept;
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
        std::vector<OptionalRef<IPhysicsUpdater>> _updaters;
        std::vector<OptionalRef<ICollisionListener>> _listeners;

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
        void init(PhysicsBody& body, PhysicsSystemImpl& system) noexcept;
        void shutdown();
        void update(Entity entity, float deltaTime);

        const Shape& getShape() const noexcept;
        MotionType getMotionType() const noexcept;
        const JPH::BodyID& getBodyId() const noexcept;

        void setPosition(const glm::vec3& pos);
        glm::vec3 getPosition();
        void setRotation(const glm::quat& rot);
        glm::quat getRotation();
        void setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity();

        void addTorque(const glm::vec3& torque);
        void addForce(const glm::vec3& force);
        void addImpulse(const glm::vec3& impulse);
        void move(const glm::vec3& pos, const glm::quat& rot, float deltaTime );
        void movePosition(const glm::vec3& pos, float deltaTime);

        void addListener(ICollisionListener& listener) noexcept;
        bool removeListener(ICollisionListener& listener) noexcept;

        void onCollisionEnter(PhysicsBody& other, const Collision& collision);
        void onCollisionStay(PhysicsBody& other, const Collision& collision);
        void onCollisionExit(PhysicsBody& other);

    private:
        OptionalRef<JPH::BodyInterface> getBodyInterface() const noexcept;
        JPH::BodyID createBody(const JPH::Vec3& pos, const JPH::Quat& rot) noexcept;
        JPH::BodyID createCharacter(const JPH::Vec3& pos, const JPH::Quat& rot) noexcept;
        bool tryCreateBody(OptionalRef<Transform> transform) noexcept;

        OptionalRef<PhysicsBody> _body;
        OptionalRef<PhysicsSystemImpl> _system;
        Config _config;
        JPH::BodyID _bodyId;        
        std::vector<OptionalRef<ICollisionListener>> _listeners;

        std::optional<CharacterConfig> _characterConfig;
        JPH::Ref<JPH::Character> _character;
    };
}