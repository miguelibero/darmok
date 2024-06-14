#include "character_jolt.hpp"
#include "physics3d_jolt.hpp"
#include <darmok/character.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok
{
    CharacterControllerImpl::CharacterControllerImpl(const Config& config) noexcept
        : _config(config)
        , _character(nullptr)
    {
    }

    CharacterControllerImpl::~CharacterControllerImpl() noexcept
    {
        shutdown();
    }

    void CharacterControllerImpl::init(CharacterController& ctrl, Physics3dSystemImpl& system)
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
        _character = nullptr;
        _system.reset();
        _ctrl.reset();
    }

    glm::vec3 CharacterControllerImpl::getPosition() const noexcept
    {
        if (!_character)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_character->GetPosition());
    }

    void CharacterControllerImpl::setPosition(const glm::vec3& pos) noexcept
    {
        if (_character)
        {
            _character->SetPosition(JoltUtils::convert(pos));
        }
    }

    void CharacterControllerImpl::setLinearVelocity(const glm::vec3& velocity)
    {
        if (_character)
        {
            _character->SetLinearVelocity(JoltUtils::convert(velocity));
        }
    }

    glm::vec3 CharacterControllerImpl::getLinearVelocity()
    {
        if (!_character)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_character->GetLinearVelocity());
    }

    void CharacterControllerImpl::addListener(ICharacterListener& listener) noexcept
    {
        JoltUtils::addRefVector(_listeners, listener);
    }

    bool CharacterControllerImpl::removeListener(ICharacterListener& listener) noexcept
    {
        return JoltUtils::removeRefVector(_listeners, listener);;
    }


    void CharacterControllerImpl::OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings)
    {
    }

    void CharacterControllerImpl::onCollisionEnter(RigidBody3d& other, const Physics3dCollision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionEnter(_ctrl.value(), other, collision);
        }
    }

    void CharacterControllerImpl::onCollisionStay(RigidBody3d& other, const Physics3dCollision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionStay(_ctrl.value(), other, collision);
        }
    }

    void CharacterControllerImpl::onCollisionExit(RigidBody3d& other)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionExit(_ctrl.value(), other);
        }
    }

    bool CharacterControllerImpl::tryCreateCharacter(OptionalRef<Transform> trans) noexcept
    {
        if (_character != nullptr)
        {
            return false;
        }
        JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mMaxSlopeAngle = _config.maxSlopeAngle;
        settings->mMaxStrength = _config.maxStrength;
        settings->mShape = JoltUtils::convert(_config.shape);
        settings->mBackFaceMode = (JPH::EBackFaceMode)_config.backFaceMode;
        settings->mCharacterPadding = _config.padding;
        settings->mPenetrationRecoverySpeed = _config.penetrationRecoverySpeed;
        settings->mPredictiveContactDistance = _config.predictiveContactDistance;
        settings->mSupportingVolume = JPH::Plane(JoltUtils::convert(_config.supportingPlane.normal), _config.supportingPlane.constant);

        glm::vec3 pos(0);
        glm::quat rot(1, 0, 0, 0);
        if (trans)
        {
            pos = trans->getWorldPosition();
            rot = trans->getWorldRotation();
        }

        _character = new JPH::CharacterVirtual(settings, JoltUtils::convert(pos), JoltUtils::convert(rot), 0, &_system->getJolt());
        _character->SetListener(this);
        return true;
    }

    void CharacterControllerImpl::update(Entity entity, float deltaTime)
    {
        if (!_system)
        {
            return;
        }
        auto trans = _system->getScene()->getComponent<Transform>(entity);

        tryCreateCharacter(trans);

        JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
        auto& joltSystem = _system->getJolt();
        auto layer = (JPH::ObjectLayer)JoltLayer::Moving;
        auto gravity = -_character->GetUp() * joltSystem.GetGravity().Length();
        _character->ExtendedUpdate(deltaTime, gravity, updateSettings,
            joltSystem.GetDefaultBroadPhaseLayerFilter(layer),
            joltSystem.GetDefaultLayerFilter(layer),
            {}, {}, _system->getTempAllocator()
        );

        if (trans)
        {
            auto mat = JoltUtils::convert(_character->GetWorldTransform());
            auto parent = trans->getParent();
            if (parent)
            {
                mat = parent->getWorldInverse() * mat;
            }
            trans->setLocalMatrix(mat);
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

    CharacterController& CharacterController::setLinearVelocity(const glm::vec3& velocity)
    {
        _impl->setLinearVelocity(velocity);
        return *this;
    }

    glm::vec3 CharacterController::getLinearVelocity()
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

    CharacterController& CharacterController::addListener(ICharacterListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool CharacterController::removeListener(ICharacterListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }
}