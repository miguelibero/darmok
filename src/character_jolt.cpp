#include "detail/character_jolt.hpp"
#include "detail/physics3d_jolt.hpp"
#include <darmok/character.hpp>
#include <darmok/transform.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/shape_serialize.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok::physics3d
{
    CharacterControllerImpl::CharacterControllerImpl(const Definition& def) noexcept
        : _def{ def }
        , _jolt{ nullptr }
    {
    }

    CharacterControllerImpl::~CharacterControllerImpl() noexcept
    {
        shutdown();
    }

    PhysicsSystemImpl& CharacterControllerImpl::getSystemImpl() noexcept
    {
        return _system->getImpl();
    }

    void CharacterControllerImpl::init(CharacterController& ctrl, PhysicsSystem& system) noexcept
    {
        if (_system)
        {
            shutdown();
        }
        _system = system;
        _ctrl = ctrl;
    }

    void CharacterControllerImpl::shutdown() noexcept
    {
        _jolt = nullptr;
        _system.reset();
        _ctrl.reset();
    }

    bool CharacterControllerImpl::isGrounded() const noexcept
    {
        auto state = getGroundState();
        return state == GroundState::Grounded;
    }

    GroundState CharacterControllerImpl::getGroundState() const noexcept
    {
        if (!_jolt)
        {
            return GroundState::NotSupported;
        }
        return (GroundState)_jolt->GetGroundState();
    }

    glm::vec3 CharacterControllerImpl::getPosition() const noexcept
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetPosition());
    }

    void CharacterControllerImpl::setPosition(const glm::vec3& pos) noexcept
    {
        auto rb = getPhysicsBody();
        if (rb && rb->isEnabled())
        {
            rb->setPosition(pos);
        }
        if (_jolt)
        {
            _jolt->SetPosition(JoltUtils::convert(pos));
        }
    }

    void CharacterControllerImpl::setLinearVelocity(const glm::vec3& velocity) noexcept
    {
        auto rb = getPhysicsBody();
        if (rb && rb->isEnabled())
        {
            rb->setLinearVelocity(velocity);
        }
        if (_jolt)
        {
            _jolt->SetLinearVelocity(JoltUtils::convert(velocity));
        }
    }

    glm::vec3 CharacterControllerImpl::getLinearVelocity() const noexcept
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetLinearVelocity());
    }

    glm::quat CharacterControllerImpl::getRotation() const noexcept
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetRotation());
    }

    void CharacterControllerImpl::setRotation(const glm::quat& rot) noexcept
    {
        auto rb = getPhysicsBody();
        if (rb && rb->isEnabled())
        {
            rb->setRotation(rot);
        }
        if (_jolt)
        {
            _jolt->SetRotation(JoltUtils::convert(rot));
        }
    }

    glm::vec3 CharacterControllerImpl::getGroundVelocity() const noexcept
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetGroundVelocity());
    }

    glm::vec3 CharacterControllerImpl::getGroundNormal() const noexcept
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetGroundNormal());
    }

    glm::vec3 CharacterControllerImpl::getGroundPosition() const noexcept
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetGroundPosition());
    }

    void CharacterControllerImpl::setDelegate(Delegate& dlg) noexcept
    {
        _delegate = dlg;
        _delegatePointer.reset();
    }

    void CharacterControllerImpl::setDelegate(std::unique_ptr<Delegate>&& dlg) noexcept
    {
        _delegate = *dlg;
        _delegatePointer = std::move(dlg);
    }

    OptionalRef<CharacterControllerImpl::Delegate> CharacterControllerImpl::getDelegate() const noexcept
    {
        return _delegate;
    }

    OptionalRef<PhysicsBody> CharacterControllerImpl::getPhysicsBody() const noexcept
    {
        if (!_ctrl || !_system)
        {
            return nullptr;
        }
        auto scene = _system->getScene();
        if (!scene)
        {
            return nullptr;
        }
        auto entity = scene->getEntity(_ctrl.value());
        return scene->getComponent<PhysicsBody>(entity);
    }

    void CharacterControllerImpl::OnAdjustBodyVelocity(const JPH::CharacterVirtual* character, const JPH::Body& body2, JPH::Vec3& linearVelocity, JPH::Vec3& angularVelocity) noexcept
    {
        if (!_delegate || !_ctrl || !_system)
        {
            return;
        }

        auto body = getSystemImpl().getPhysicsBody(body2);
        if (!body)
        {
            return;
        }
        auto linv = JoltUtils::convert(linearVelocity);
        auto angv = JoltUtils::convert(angularVelocity);
        _delegate->onAdjustBodyVelocity(_ctrl.value(), body.value(), linv, angv);
        linearVelocity = JoltUtils::convert(linv);
        angularVelocity = JoltUtils::convert(angv);
    }

    bool CharacterControllerImpl::OnContactValidate(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2) noexcept
    {
        if (!_delegate || !_ctrl || !_system)
        {
            return true;
        }
        auto body = getSystemImpl().getPhysicsBody(bodyID2);
        if (!body)
        {
            return true;
        }
        return _delegate->onContactValidate(_ctrl.value(), body.value());
    }

    void CharacterControllerImpl::OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings) noexcept
    {
        if (!_delegate || !_ctrl || !_system)
        {
            return;
        }
        auto body = getSystemImpl().getPhysicsBody(bodyID2);
        if (!body)
        {
            return;
        }
        Contact contact{ JoltUtils::convert(contactPosition), JoltUtils::convert(contactNormal) };
        CharacterContactSettings darmokSettings
        {
            settings.mCanPushCharacter, settings.mCanReceiveImpulses
        };
        _delegate->onContactAdded(_ctrl.value(), body.value(), contact, darmokSettings);
        settings.mCanPushCharacter = darmokSettings.canPushCharacter;
        settings.mCanReceiveImpulses = darmokSettings.canReceiveImpulses;
    }

    void CharacterControllerImpl::OnContactSolve(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::Vec3Arg contactVelocity, const JPH::PhysicsMaterial* contactMaterial, JPH::Vec3Arg characterVelocity, JPH::Vec3& newCharacterVelocity) noexcept
    {
        if (!_delegate || !_ctrl || !_system)
        {
            return;
        }
        auto body = getSystemImpl().getPhysicsBody(bodyID2);
        if (!body)
        {
            return;
        }
        Contact contact{ JoltUtils::convert(contactPosition), JoltUtils::convert(contactNormal), JoltUtils::convert(contactVelocity) };
        glm::vec3 charVel = JoltUtils::convert(characterVelocity);
        _delegate->onContactSolve(_ctrl.value(), body.value(), contact, charVel);
        newCharacterVelocity = JoltUtils::convert(charVel);
    }

    bool CharacterControllerImpl::tryCreateCharacter(Transform& trans) noexcept
    {
        if (_jolt || !_system)
        {
            return false;
        }
        auto joltSystem = getSystemImpl().getJolt();
        if (!joltSystem)
        {
            return false;
        }
        auto joltTrans = getSystemImpl().loadTransform(trans);

        const JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mMaxSlopeAngle = _def.base().max_slope_angle();
		settings->mMaxStrength = _def.max_strength();
        settings->mShape = JoltUtils::convert(_def.base().shape(), joltTrans.scale);
        settings->mBackFaceMode = (JPH::EBackFaceMode)_def.back_face_mode();
        settings->mCharacterPadding = _def.padding();
        settings->mPenetrationRecoverySpeed = _def.penetration_recovery_speed();
        settings->mPredictiveContactDistance = _def.predictive_contact_distance();
        settings->mSupportingVolume = JoltUtils::convert(darmok::protobuf::convert(_def.base().supporting_plane()));

        auto userData = (uint64_t)_ctrl.ptr();
        _jolt = new JPH::CharacterVirtual(settings, joltTrans.position, joltTrans.rotation, userData, joltSystem.ptr());
        _jolt->SetListener(this);
        return true;
    }

    void CharacterControllerImpl::update(Entity entity, float deltaTime) noexcept
    {
        if (!_system)
        {
            return;
        }
        auto& scene = *_system->getScene();
        auto& trans = scene.getOrAddComponent<Transform>(entity);

        tryCreateCharacter(trans);
        if (!_jolt)
        {
            return;
        }

        auto joltSystem = getSystemImpl().getJolt();
        if (!joltSystem)
        {
            return;
        }

        JPH::IgnoreMultipleBodiesFilter bodyFilter;

        auto rb = getPhysicsBody();
        if (rb)
        {
             auto& bodyId = rb->getImpl().getBodyId();
             bodyFilter.IgnoreBody(bodyId);
             auto& iface = joltSystem->GetBodyInterface();
             JPH::Vec3 pos;
             JPH::Quat rot;
             iface.GetPositionAndRotation(bodyId, pos, rot);
             _jolt->SetPosition(pos);
             _jolt->SetRotation(rot);
             _jolt->SetLinearVelocity(iface.GetLinearVelocity(bodyId));
        }

        JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;

        auto layer = _def.base().layer();

        auto gravity = -_jolt->GetUp() * joltSystem->GetGravity().Length();
        _jolt->ExtendedUpdate(deltaTime, gravity, updateSettings,
            joltSystem->GetDefaultBroadPhaseLayerFilter(layer),
            joltSystem->GetDefaultLayerFilter(layer),
            bodyFilter, {}, getSystemImpl().getTempAllocator()
        );

        // if the entity has a rigid body, that component will update the transform
        if (!getPhysicsBody())
        {
            getSystemImpl().updateTransform(trans, _jolt->GetWorldTransform());
        }
    }

    CharacterController::CharacterController(const Definition& def) noexcept
        : _impl{ std::make_unique<CharacterControllerImpl>(def) }
    {        
    }

    CharacterController::CharacterController(const Shape& shape) noexcept
    {
        auto def = createDefinition();
        PhysicsShapeDefinitionWrapper{ *def.mutable_base()->mutable_shape() }.setShape(shape);
        _impl = std::make_unique<CharacterControllerImpl>(def);
    }

    CharacterController::~CharacterController() = default;

    CharacterControllerImpl& CharacterController::getImpl() noexcept
    {
        return *_impl;
    }

    const CharacterControllerImpl& CharacterController::getImpl() const noexcept
    {
        return *_impl;
    }

    bool CharacterController::isGrounded() const noexcept
    {
        return _impl->isGrounded();
    }

    GroundState CharacterController::getGroundState() const noexcept
    {
        return _impl->getGroundState();
    }

    std::string CharacterController::getGroundStateName(GroundState state) noexcept
    {
        return JPH::CharacterBase::sToString((JPH::CharacterBase::EGroundState)state);
    }

    CharacterController::Definition CharacterController::createDefinition() noexcept
    {
        Definition def;
		*def.mutable_base() = PhysicsBody::createBaseCharacterDefinition();
        def.set_max_strength(100.0f);
        def.set_back_face_mode(Definition::CollideWithBackFaces);
        def.set_padding(0.02f);
        def.set_penetration_recovery_speed(1.0f);
        def.set_predictive_contact_distance(0.1f);
		return def;
    }

    CharacterController& CharacterController::setLinearVelocity(const glm::vec3& velocity) noexcept
    {
        _impl->setLinearVelocity(velocity);
        return *this;
    }

    glm::vec3 CharacterController::getLinearVelocity() const noexcept
    {
        return _impl->getLinearVelocity();
    }

    CharacterController& CharacterController::setPosition(const glm::vec3& pos) noexcept
    {
        _impl->setPosition(pos);
        return *this;
    }

    glm::vec3 CharacterController::getPosition() const noexcept
    {
        return _impl->getPosition();
    }

    CharacterController& CharacterController::setRotation(const glm::quat& rot) noexcept
    {
        _impl->setRotation(rot);
        return *this;
    }

    glm::quat CharacterController::getRotation() const noexcept
    {
        return _impl->getRotation();
    }

    glm::vec3 CharacterController::getGroundNormal() const noexcept
    {
        return _impl->getGroundNormal();
    }

    glm::vec3 CharacterController::getGroundPosition() const noexcept
    {
        return _impl->getGroundPosition();
    }

    glm::vec3 CharacterController::getGroundVelocity() const noexcept
    {
        return _impl->getGroundVelocity();
    }

    CharacterController& CharacterController::setDelegate(Delegate& dlg) noexcept
    {
        _impl->setDelegate(dlg);
        return *this;
    }

    CharacterController& CharacterController::setDelegate(std::unique_ptr<Delegate>&& dlg) noexcept
    {
        _impl->setDelegate(std::move(dlg));
        return *this;
    }

    OptionalRef<CharacterControllerImpl::Delegate> CharacterController::getDelegate() const noexcept
    {
        return _impl->getDelegate();
    }
}