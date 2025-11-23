#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/shape.hpp>
#include <darmok/convert.hpp>
#include <darmok/protobuf/physics3d.pb.h>
#include <bx/bx.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <variant>
#include <vector>
#include <optional>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class Camera;
    class Program;
}

namespace darmok::physics3d
{
    using PhysicsShape = std::variant<Cube, Sphere, Capsule, Polygon, BoundingBox>;   

    enum class GroundState
    {
        Grounded,
        GroundedSteep,
        NotSupported,
        Air,
    };

	class DARMOK_EXPORT BX_NO_VTABLE IPhysicsUpdater
	{
	public:
		virtual ~IPhysicsUpdater() = default;
        virtual entt::id_type getPhysicsUpdaterType() const noexcept { return 0; };
		virtual void fixedUpdate(float fixedDeltaTime) = 0;
	};

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypePhysicsUpdater : public IPhysicsUpdater
    {
    public:
        entt::id_type getPhysicsUpdaterType() const noexcept override
        {
            return entt::type_hash<T>::value();
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE IPhysicsUpdaterFilter
    {
    public:
        virtual ~IPhysicsUpdaterFilter() = default;
        virtual bool operator()(const IPhysicsUpdater& updater) const = 0;
    };

    class PhysicsBody;

    struct DARMOK_EXPORT Collision final
    {
        glm::vec3 normal;
        std::vector<glm::vec3> contacts;

        std::string toString() const noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ICollisionListener
    {
    public:
        virtual ~ICollisionListener() = default;
        virtual entt::id_type getCollisionListenerType() const noexcept { return 0; };
        virtual void onCollisionEnter(PhysicsBody& physicsBody1, PhysicsBody& physicsBody2, const Collision& collision) {};
        virtual void onCollisionStay(PhysicsBody& physicsBody1, PhysicsBody& physicsBody2, const Collision& collision) {};
        virtual void onCollisionExit(PhysicsBody& physicsBody1, PhysicsBody& physicsBody2) {};
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypeCollisionListener : public ICollisionListener
    {
    public:
        entt::id_type getCollisionListenerType() const noexcept override
        {
            return entt::type_hash<T>::value();
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE ICollisionListenerFilter
    {
    public:
        virtual ~ICollisionListenerFilter() = default;
        virtual bool operator()(const ICollisionListener& listener) const = 0;
    };

    using BroadLayer = uint8_t;
    using LayerMask = uint16_t;
    static const LayerMask kAllLayers = 65535;

    struct ConstPhysicsShapeDefinitionWrapper
    {
        using Definition = protobuf::PhysicsShape;
		using CharacterDefinition = protobuf::Character;
        ConstPhysicsShapeDefinitionWrapper(const Definition& def) noexcept;
        glm::vec3 getOrigin() const noexcept;
    private:
        const Definition& _def;
    };

    struct ConstPhysicsBodyDefinitionWrapper
    {
        using Definition = protobuf::PhysicsBody;
        using CharacterDefinition = protobuf::Character;
        ConstPhysicsBodyDefinitionWrapper(const Definition& def) noexcept;
        CharacterDefinition toCharacter() noexcept;
    private:
        const Definition& _def;
    };

    struct ConstPhysicsSystemDefinitionWrapper
    {
		using Definition = protobuf::PhysicsSystem;
        ConstPhysicsSystemDefinitionWrapper(const Definition& def) noexcept;
		std::size_t getBroadLayerNum() const noexcept;
        BroadLayer getBroadLayer(LayerMask layer) const noexcept;
        const std::string& getBroadLayerName(BroadLayer layer) const;
    private:
		const Definition& _def;
    };

    struct RaycastHit final
    {
        std::reference_wrapper<PhysicsBody> body;
        float factor;
        float distance;
        glm::vec3 point;

        std::string toString() const noexcept;
    };

    class PhysicsSystemImpl;

    class DARMOK_EXPORT PhysicsSystem final : public ITypeSceneComponent<PhysicsSystem>
    {
    public:
        using Definition = protobuf::PhysicsSystem;

        static Definition createDefinition() noexcept;

        PhysicsSystem(const Definition& def = createDefinition()) noexcept;
        PhysicsSystem(const Definition& def, bx::AllocatorI& alloc) noexcept;
        PhysicsSystem(bx::AllocatorI& alloc) noexcept;
        ~PhysicsSystem() noexcept;

        PhysicsSystemImpl& getImpl() noexcept;
        const PhysicsSystemImpl& getImpl() const noexcept;

        OptionalRef<Scene> getScene() noexcept;
        OptionalRef<const Scene> getScene() const noexcept;

        PhysicsSystem& setRootTransform(OptionalRef<Transform> root) noexcept;
        OptionalRef<Transform> getRootTransform() noexcept;
        OptionalRef<const Transform> getRootTransform() const noexcept;

        glm::vec3 getGravity() const;
        bool isValidEntity(Entity entity) noexcept;

        bool isPaused() const noexcept;
        PhysicsSystem& setPaused(bool paused) noexcept;

        void init(Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) noexcept override;

        PhysicsSystem& addUpdater(std::unique_ptr<IPhysicsUpdater>&& updater) noexcept;
        PhysicsSystem& addUpdater(IPhysicsUpdater& updater) noexcept;
        bool removeUpdater(const IPhysicsUpdater& updater) noexcept;
        size_t removeUpdaters(const IPhysicsUpdaterFilter& filter) noexcept;
        
        PhysicsSystem& addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept;
        PhysicsSystem& addListener(ICollisionListener& listener) noexcept;
        bool removeListener(const ICollisionListener& listener) noexcept;
        size_t removeListeners(const ICollisionListenerFilter& filter) noexcept;

        std::optional<RaycastHit> raycast(const Ray& ray, LayerMask layers = kAllLayers) const noexcept;
        std::vector<RaycastHit> raycastAll(const Ray& ray, LayerMask layers = kAllLayers) const noexcept;

        void activateBodies(const BoundingBox& bbox, LayerMask layers = kAllLayers) noexcept;
    private:
        std::unique_ptr<PhysicsSystemImpl> _impl;
    };


    class PhysicsBodyImpl;

    class PhysicsBody final
    {
    public:
		using Definition = protobuf::PhysicsBody;
        using CharacterDefinition = protobuf::Character;        
        using MotionType = Definition::MotionType;
        using ShapeDefinition = protobuf::PhysicsShape;

        PhysicsBody(const PhysicsShape& shape, MotionType motion = Definition::Dynamic) noexcept;
        PhysicsBody(const Definition& def = createDefinition()) noexcept;
        PhysicsBody(const CharacterDefinition& def) noexcept;
        ~PhysicsBody() noexcept;

        static Definition createDefinition() noexcept;
        static ShapeDefinition createCharacterShape() noexcept;
		static Plane::Definition createSupportingPlaneDefinition() noexcept;
        static CharacterDefinition createCharacterDefinition() noexcept;

        PhysicsBodyImpl& getImpl() noexcept;
        const PhysicsBodyImpl& getImpl() const noexcept;

        OptionalRef<PhysicsSystem> getSystem() const noexcept;

        PhysicsShape getShape() const noexcept;
        MotionType getMotionType() const noexcept;
        BoundingBox getLocalBounds() const;
        BoundingBox getWorldBounds() const;

        bool isGrounded() const noexcept;
        GroundState getGroundState() const noexcept;

        PhysicsBody& setPosition(const glm::vec3& pos);
        glm::vec3 getPosition() const;
        PhysicsBody& setRotation(const glm::quat& rot);
        glm::quat getRotation() const;
        PhysicsBody& setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity() const;
        float getInverseMass() const;
        PhysicsBody& setInverseMass(float v);

        bool isActive() const;
        PhysicsBody& activate();
        PhysicsBody& deactivate();
        bool isEnabled() const;
        PhysicsBody& setEnabled(bool enabled);

        PhysicsBody& addTorque(const glm::vec3& torque);
        PhysicsBody& addForce(const glm::vec3& force);
        PhysicsBody& addImpulse(const glm::vec3& impulse);
        PhysicsBody& move(const glm::vec3& pos, const glm::quat& rot, float deltaTime = 0.F);
        PhysicsBody& movePosition(const glm::vec3& pos, float deltaTime = 0.F);

        PhysicsBody& addListener(std::unique_ptr<ICollisionListener>&& listener) noexcept;
        PhysicsBody& addListener(ICollisionListener& listener) noexcept;
        bool removeListener(const ICollisionListener& listener) noexcept;
        size_t removeListeners(const ICollisionListenerFilter& filter) noexcept;

        std::string toString() const noexcept;

    private:
        std::unique_ptr<PhysicsBodyImpl> _impl;
    };
}

namespace darmok
{
    template<>
    struct Converter<physics3d::PhysicsShape, physics3d::protobuf::PhysicsShape>
    {
        static physics3d::PhysicsShape run(const physics3d::protobuf::PhysicsShape& v);
    };

    template<>
    struct Converter<physics3d::protobuf::PhysicsShape, physics3d::PhysicsShape>
    {
        static physics3d::protobuf::PhysicsShape run(const physics3d::PhysicsShape& v);
    };

}