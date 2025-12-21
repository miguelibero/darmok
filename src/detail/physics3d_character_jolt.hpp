#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <darmok/physics3d_character.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

namespace JPH
{
    class CharacterVirtual;
    class Character;
}

namespace darmok::physics3d
{
    class PhysicsSystemImpl;

    class CharacterControllerImpl final : public JPH::CharacterContactListener
    {
    public:
        using Definition = protobuf::CharacterController;
        using Delegate = ICharacterDelegate;
        using Contact = CharacterContact;
        CharacterControllerImpl(const Definition& def) noexcept;
        ~CharacterControllerImpl() noexcept;
        void init(CharacterController& ctrl, PhysicsSystem& system) noexcept;
        void shutdown() noexcept;
        expected<void, std::string> update(Entity entity, float deltaTime) noexcept;
        bool isGrounded() const noexcept;
        GroundState getGroundState() const noexcept;

        glm::vec3 getPosition() const noexcept;
        void setPosition(const glm::vec3& pos) noexcept;
        void setLinearVelocity(const glm::vec3& velocity) noexcept;
        glm::vec3 getLinearVelocity() const noexcept;
        glm::quat getRotation() const noexcept;
        void setRotation(const glm::quat& rot) noexcept;
        glm::vec3 getGroundNormal() const noexcept;
        glm::vec3 getGroundPosition() const noexcept;
        glm::vec3 getGroundVelocity() const noexcept;
        PhysicsShape getShape() const noexcept;

        void setDelegate(Delegate& dlg) noexcept;
        void setDelegate(std::unique_ptr<Delegate>&& dlg) noexcept;
        OptionalRef<Delegate> getDelegate() const noexcept;

        void OnAdjustBodyVelocity(const JPH::CharacterVirtual* character, const JPH::Body& inBody2, JPH::Vec3& linearVelocity, JPH::Vec3& angularVelocity) noexcept override;
        bool OnContactValidate(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2) noexcept override;
        void OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings) noexcept override;
        void OnContactSolve(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg cntactNormal, JPH::Vec3Arg contactVelocity, const JPH::PhysicsMaterial* contactMaterial, JPH::Vec3Arg characterVelocity, JPH::Vec3& newCharacterVelocity) noexcept override;
    
    private:
        Definition _def;
        OptionalRef<PhysicsSystem> _system;
        OptionalRef<CharacterController> _ctrl;
        JPH::Ref<JPH::CharacterVirtual> _jolt;
        OptionalRef<Delegate> _delegate;
        std::unique_ptr<Delegate> _delegatePointer;
        std::vector<std::string> _pendingErrors;

        PhysicsSystemImpl& getSystemImpl() noexcept;

        expected<void, std::string> tryCreateCharacter(Transform& transform) noexcept;
        OptionalRef<PhysicsBody> getPhysicsBody() const noexcept;
    };
}