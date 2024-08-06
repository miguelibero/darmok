#pragma once

#include <memory>
#include <darmok/glm.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/shape.hpp>
#include <darmok/character_fwd.hpp>
#include <darmok/export.h>
#include <bx/bx.h>

namespace darmok::physics3d
{
    // https://github.com/jrouwe/JoltPhysics/discussions/239

    struct DARMOK_EXPORT BaseCharacterConfig
    {
        using Shape = PhysicsShape;
        Shape shape = Capsule(1.F, 0.25F, glm::vec3(0, 0.75F, 0));
        glm::vec3 up = glm::vec3(0, 1, 0);
        Plane supportingPlane = Plane(glm::vec3(0, 1, 0), -1.0e10f);
        float maxSlopeAngle = glm::radians(50.F);
        uint16_t layer = 0;
    };

    struct DARMOK_EXPORT CharacterConfig final : public BaseCharacterConfig
    {
        float mass = 80.F;
        float friction = 1.F;
        float gravityFactor = 1.0F;
        float maxSeparationDistance = 0.1F;

        void load(const PhysicsBodyConfig& bodyConfig) noexcept;
    };

    struct DARMOK_EXPORT CharacterControllerConfig final : public BaseCharacterConfig
    {
        float maxStrength = 100.F;
        BackFaceMode backFaceMode = BackFaceMode::CollideWithBackFaces;
        float padding = 0.02f;
        float penetrationRecoverySpeed = 1.0f;
        float predictiveContactDistance = 0.1f;
    };

    class CharacterController;

    struct DARMOK_EXPORT CharacterContactSettings final
    {
        bool canPushCharacter;
        bool canReceiveImpulses;
    };

    struct DARMOK_EXPORT CharacterContact final
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 velocity;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ICharacterControllerDelegate
    {
    public:
        virtual ~ICharacterControllerDelegate() = default;
        using Contact = CharacterContact;
        using ContactSettings = CharacterContactSettings;
        virtual void onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity) { }
        virtual bool onContactValidate(CharacterController& character, PhysicsBody& body) { return true; }
        virtual void onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings) { }
        virtual void onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity) { }
    };

    class CharacterControllerImpl;

    class DARMOK_EXPORT CharacterController final
    {
    public:
        using Shape = PhysicsShape;
        using Config = CharacterControllerConfig;
        using Delegate = ICharacterControllerDelegate;
        CharacterController(const Config& config = {});
        CharacterController(const Shape& shape);
        ~CharacterController();

        CharacterControllerImpl& getImpl() noexcept;
        const CharacterControllerImpl& getImpl() const noexcept;

        bool isGrounded() const noexcept;
        GroundState getGroundState() const noexcept;

        CharacterController& setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity() const noexcept;

        CharacterController& setPosition(const glm::vec3& pos) noexcept;
        glm::vec3 getPosition() const noexcept;

        CharacterController& setRotation(const glm::quat& rot) noexcept;
        glm::quat getRotation() const noexcept;

        glm::vec3 getGroundNormal() const noexcept;
        glm::vec3 getGroundPosition() const noexcept;
        glm::vec3 getGroundVelocity() const noexcept;

        CharacterController& setDelegate(const OptionalRef<Delegate>& delegate) noexcept;

        static std::string getGroundStateName(GroundState state) noexcept;

    private:
        std::unique_ptr<CharacterControllerImpl> _impl;
    };
}