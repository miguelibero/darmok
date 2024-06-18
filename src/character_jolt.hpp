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
        CharacterControllerImpl(const Config& config) noexcept;
        ~CharacterControllerImpl() noexcept;
        void init(CharacterController& ctrl, PhysicsSystemImpl& system);
        void shutdown();
        void update(Entity entity, float deltaTime);
        glm::vec3 getPosition() const noexcept;
        void setPosition(const glm::vec3& pos) noexcept;
        void setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity() const noexcept;

        void addListener(ICharacterControllerListener& listener) noexcept;
        bool removeListener(ICharacterControllerListener& listener) noexcept;

        void OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings) override;
    private:
        Config _config;
        OptionalRef<PhysicsSystemImpl> _system;
        OptionalRef<CharacterController> _ctrl;
        JPH::Ref<JPH::CharacterVirtual> _jolt;
        std::vector<OptionalRef<ICharacterControllerListener>> _listeners;

        using CollisionMap = std::unordered_map<JPH::BodyID, Collision>;
        CollisionMap _collisions;

        bool tryCreateCharacter(OptionalRef<Transform> transform) noexcept;
        void onCollisionEnter(PhysicsBody& other, const Collision& collision);
        void onCollisionStay(PhysicsBody& other, const Collision& collision);
        void onCollisionExit(PhysicsBody& other);
        void notifyCollisionListeners(const CollisionMap& oldCollisions);
        OptionalRef<PhysicsBody> getPhysicsBody() const noexcept;
    };
}