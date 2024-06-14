#pragma once

#include <memory>
#include "glm.hpp"
#include <darmok/physics3d.hpp>
#include <darmok/shape.hpp>
#include <bx/bx.h>

namespace darmok
{
    struct DARMOK_EXPORT CharacterControllerConfig final
    {
        using Shape = Physics3dShape;
        Shape shape = Capsule(1.F, 0.25F);
        glm::vec3 up = glm::vec3(0, 1, 0);
        float maxSlopeAngle = glm::radians(50.F);
        float maxStrength = 100.F;
        Plane supportingPlane = Plane(glm::vec3(0, 1, 0), -1.0e10f);
        Physics3dBackFaceMode backFaceMode = Physics3dBackFaceMode::CollideWithBackFaces;
        float padding = 0.02f;
        float penetrationRecoverySpeed = 1.0f;
        float predictiveContactDistance = 0.1f;
    };

    class CharacterController;

    class DARMOK_EXPORT BX_NO_VTABLE ICharacterListener
    {
    public:
        using Collision = Physics3dCollision;
        virtual ~ICharacterListener() = default;

        virtual void onCollisionEnter(CharacterController& character, RigidBody3d& rigidBody, const Collision& collision) {};
        virtual void onCollisionStay(CharacterController& character, RigidBody3d& rigidBody, const Collision& collision) {};
        virtual void onCollisionExit(CharacterController& character, RigidBody3d& rigidBody) {};
    };

    class CharacterControllerImpl;

    class DARMOK_EXPORT CharacterController final
    {
    public:
        using Shape = Physics3dShape;
        using Config = CharacterControllerConfig;
        CharacterController(const Config& config = {});
        CharacterController(const Shape& shape);
        ~CharacterController();

        CharacterControllerImpl& getImpl() noexcept;
        const CharacterControllerImpl& getImpl() const noexcept;

        CharacterController& setLinearVelocity(const glm::vec3& velocity);
        glm::vec3 getLinearVelocity();

        CharacterController& setPosition(const glm::vec3& pos) noexcept;
        glm::vec3 getPosition() const noexcept;

        CharacterController& addListener(ICharacterListener& listener) noexcept;
        bool removeListener(ICharacterListener& listener) noexcept;

    private:
        std::unique_ptr<CharacterControllerImpl> _impl;
    };
}