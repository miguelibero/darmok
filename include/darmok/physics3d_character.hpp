#pragma once

#include <memory>
#include <darmok/glm.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/protobuf/physics3d.pb.h>
#include <darmok/shape.hpp>
#include <darmok/export.h>
#include <bx/bx.h>

namespace darmok::physics3d
{
    // https://github.com/jrouwe/JoltPhysics/discussions/239

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

    class DARMOK_EXPORT BX_NO_VTABLE ICharacterDelegate
    {
    public:
        virtual ~ICharacterDelegate() = default;
        using Contact = CharacterContact;
        using ContactSettings = CharacterContactSettings;
        virtual entt::id_type getCharacterDelegateType() const noexcept { return 0; };
        virtual expected<void, std::string> onAdjustBodyVelocity(CharacterController& character, PhysicsBody& body, glm::vec3& linearVelocity, glm::vec3& angularVelocity) noexcept { return {}; }
        virtual expected<bool, std::string> onContactValidate(CharacterController& character, PhysicsBody& body) noexcept { return true; }
        virtual expected<void, std::string> onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings) noexcept { return {}; }
        virtual expected<void, std::string> onContactSolve(CharacterController& character, PhysicsBody& body, const Contact& contact, glm::vec3& characterVelocity) noexcept { return {}; }
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypeCharacterDelegate : public ICharacterDelegate
    {
    public:
        entt::id_type getCharacterDelegateType() const noexcept override
        {
            return entt::type_hash<T>::value();
        }
    };

    class CharacterControllerImpl;

    class DARMOK_EXPORT CharacterController final
    {
    public:
        using ShapeDefinition = protobuf::PhysicsShape;
        using Definition = protobuf::CharacterController;
        using Delegate = ICharacterDelegate;

        CharacterController() noexcept;
        CharacterController(const Definition& def) noexcept;
        CharacterController(const PhysicsShape& shape) noexcept;
        ~CharacterController() noexcept;
        CharacterController(CharacterController&& other) noexcept;
        CharacterController& operator=(CharacterController&& other) noexcept;

        CharacterControllerImpl& getImpl() noexcept;
        const CharacterControllerImpl& getImpl() const noexcept;

        bool isGrounded() const noexcept;
        GroundState getGroundState() const noexcept;

        CharacterController& setLinearVelocity(const glm::vec3& velocity) noexcept;
        glm::vec3 getLinearVelocity() const noexcept;

        CharacterController& setPosition(const glm::vec3& pos) noexcept;
        glm::vec3 getPosition() const noexcept;

        CharacterController& setRotation(const glm::quat& rot) noexcept;
        glm::quat getRotation() const noexcept;

        glm::vec3 getGroundNormal() const noexcept;
        glm::vec3 getGroundPosition() const noexcept;
        glm::vec3 getGroundVelocity() const noexcept;
        PhysicsShape getShape() const noexcept;

        CharacterController& setDelegate(Delegate& dlg) noexcept;
        CharacterController& setDelegate(std::unique_ptr<Delegate>&& dlg) noexcept;
        OptionalRef<Delegate> getDelegate() const noexcept;

        static std::string getGroundStateName(GroundState state) noexcept;
		static Definition createDefinition() noexcept;

    private:
        std::unique_ptr<CharacterControllerImpl> _impl;
    };
}