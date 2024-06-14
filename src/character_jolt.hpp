#pragma once

#include "jolt.hpp"
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <darmok/character.hpp>
#include <darmok/optional_ref.hpp>
#include <memory>
#include <vector>

namespace JPH
{
    class CharacterVirtual;
}

namespace darmok
{
    class Physics3dSystemImpl;

    class CharacterControllerImpl final : public JPH::CharacterContactListener
    {
    public:
        using Config = CharacterControllerConfig;
        CharacterControllerImpl(const Config& config) noexcept;
        ~CharacterControllerImpl() noexcept;
        void init(CharacterController& ctrl, Physics3dSystemImpl& system);
        void shutdown();
        void update(Entity entity, float deltaTime);
        glm::vec3 getPosition() const noexcept;
        void setPosition(const glm::vec3& pos) noexcept;
        void setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity();

        void addListener(ICharacterListener& listener) noexcept;
        bool removeListener(ICharacterListener& listener) noexcept;

        void OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings) override;
    private:
        Config _config;
        OptionalRef<Physics3dSystemImpl> _system;
        OptionalRef<CharacterController> _ctrl;
        JPH::Ref<JPH::CharacterVirtual> _character;
        std::vector<OptionalRef<ICharacterListener>> _listeners;

        bool tryCreateCharacter(OptionalRef<Transform> transform) noexcept;
        void onCollisionEnter(RigidBody3d& other, const Physics3dCollision& collision);
        void onCollisionStay(RigidBody3d& other, const Physics3dCollision& collision);
        void onCollisionExit(RigidBody3d& other);
    };
}