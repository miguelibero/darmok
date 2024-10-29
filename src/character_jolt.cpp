#include "character_jolt.hpp"
#include "physics3d_jolt.hpp"
#include <darmok/character.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok::physics3d
{
    void CharacterConfig::load(const PhysicsBodyConfig& bodyConfig) noexcept
    {
        shape = bodyConfig.shape;
        if (bodyConfig.mass)
        {
            mass = bodyConfig.mass.value();
        }
        friction = bodyConfig.friction;
        gravityFactor = bodyConfig.gravityFactor;
        layer = bodyConfig.layer;
    }

    CharacterControllerImpl::CharacterControllerImpl(const Config& config) noexcept
        : _config(config)
        , _jolt(nullptr)
    {
    }

    CharacterControllerImpl::~CharacterControllerImpl() noexcept
    {
        shutdown();
    }

    PhysicsSystemImpl& CharacterControllerImpl::getSystemImpl()
    {
        return _system->getImpl();
    }

    void CharacterControllerImpl::init(CharacterController& ctrl, PhysicsSystem& system)
    {
        if (_system)
        {
            shutdown();
        }
        _system = system;
        _ctrl = ctrl;
    }

    void CharacterControllerImpl::shutdown()
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

    void CharacterControllerImpl::setLinearVelocity(const glm::vec3& velocity)
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

    void CharacterControllerImpl::OnAdjustBodyVelocity(const JPH::CharacterVirtual* character, const JPH::Body& body2, JPH::Vec3& linearVelocity, JPH::Vec3& angularVelocity)
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
        auto lv = JoltUtils::convert(linearVelocity);
        auto av = JoltUtils::convert(angularVelocity);
        _delegate->onAdjustBodyVelocity(_ctrl.value(), body.value(), lv, av);
        linearVelocity = JoltUtils::convert(lv);
        angularVelocity = JoltUtils::convert(av);
    }

    bool CharacterControllerImpl::OnContactValidate(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2)
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

    void CharacterControllerImpl::OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings)
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

    void CharacterControllerImpl::OnContactSolve(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::Vec3Arg contactVelocity, const JPH::PhysicsMaterial* contactMaterial, JPH::Vec3Arg characterVelocity, JPH::Vec3& newCharacterVelocity)
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

    bool CharacterControllerImpl::tryCreateCharacter(Transform& trans)
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

        JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mMaxSlopeAngle = _config.maxSlopeAngle;
        settings->mMaxStrength = _config.maxStrength;
        settings->mShape = JoltUtils::convert(_config.shape, joltTrans.scale);
        settings->mBackFaceMode = (JPH::EBackFaceMode)_config.backFaceMode;
        settings->mCharacterPadding = _config.padding;
        settings->mPenetrationRecoverySpeed = _config.penetrationRecoverySpeed;
        settings->mPredictiveContactDistance = _config.predictiveContactDistance;
        settings->mSupportingVolume = JoltUtils::convert(_config.supportingPlane);

        auto userData = (uint64_t)_ctrl.ptr();
        _jolt = new JPH::CharacterVirtual(settings, joltTrans.position, joltTrans.rotation, userData, joltSystem.ptr());
        _jolt->SetListener(this);
        return true;
    }

    void CharacterControllerImpl::update(Entity entity, float deltaTime)
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

        auto gravity = -_jolt->GetUp() * joltSystem->GetGravity().Length();
        _jolt->ExtendedUpdate(deltaTime, gravity, updateSettings,
            joltSystem->GetDefaultBroadPhaseLayerFilter(_config.layer),
            joltSystem->GetDefaultLayerFilter(_config.layer),
            bodyFilter, {}, getSystemImpl().getTempAllocator()
        );

        // if the entity has a rigid body, that component will update the transform
        if (!getPhysicsBody())
        {
            getSystemImpl().updateTransform(trans, _jolt->GetWorldTransform());
        }
    }

    CharacterController::CharacterController(const Config& config)
        : _impl(std::make_unique<CharacterControllerImpl>(config))
    {        
    }

    CharacterController::CharacterController(const Shape& shape)
        : _impl(std::make_unique<CharacterControllerImpl>(Config{ shape }))
    {
    }

    CharacterController::~CharacterController()
    {
        // empty on purpose for the forward declaration of the impl
    }

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

    CharacterController& CharacterController::setLinearVelocity(const glm::vec3& velocity)
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