#pragma once

#include "jolt.hpp"
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <darmok/character.hpp>
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
        using Config = CharacterControllerConfig;
        using Delegate = ICharacterControllerDelegate;
        using Contact = CharacterContact;
        CharacterControllerImpl(const Config& config) noexcept;
        ~CharacterControllerImpl() noexcept;
        void init(CharacterController& ctrl, PhysicsSystemImpl& system);
        void shutdown();
        void update(Entity entity, float deltaTime);
        bool isGrounded() const noexcept;
        GroundState getGroundState() const noexcept;
        glm::vec3 getPosition() const noexcept;
        void setPosition(const glm::vec3& pos) noexcept;
        void setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity() const noexcept;

        void setDelegate(const OptionalRef<Delegate>& delegate) noexcept;

        void OnAdjustBodyVelocity(const JPH::CharacterVirtual* character, const JPH::Body& inBody2, JPH::Vec3& linearVelocity, JPH::Vec3& angularVelocity) override;
        bool OnContactValidate(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2) override;
        void OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings) override;
        void OnContactSolve(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg cntactNormal, JPH::Vec3Arg contactVelocity, const JPH::PhysicsMaterial* contactMaterial, JPH::Vec3Arg characterVelocity, JPH::Vec3& newCharacterVelocity) override;
    
    private:
        Config _config;
        OptionalRef<PhysicsSystemImpl> _system;
        OptionalRef<CharacterController> _ctrl;
        JPH::Ref<JPH::CharacterVirtual> _jolt;
        OptionalRef<Delegate> _delegate;

        bool tryCreateCharacter(OptionalRef<Transform> transform) noexcept;
        OptionalRef<PhysicsBody> getPhysicsBody() const noexcept;
    };
}