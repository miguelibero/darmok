#include "character_jolt.hpp"
#include "physics3d_jolt.hpp"
#include <darmok/character.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok::physics3d
{
    CharacterImpl::CharacterImpl(const Config& config)
        : _config(config)
    {
    }

    CharacterImpl::~CharacterImpl()
    {
        shutdown();
    }

    void CharacterImpl::init(Character& character, PhysicsSystemImpl& system)
    {

    }

    void CharacterImpl::shutdown()
    {

    }

    void CharacterImpl::update(Entity entity, float deltaTime)
    {

    }

    void CharacterImpl::setLinearVelocity(const glm::vec3& velocity)
    {
        if (_jolt)
        {
            _jolt->SetLinearVelocity(JoltUtils::convert(velocity));
        }
    }

    glm::vec3 CharacterImpl::getLinearVelocity()
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetLinearVelocity());
    }

    void CharacterImpl::setPosition(const glm::vec3& pos) noexcept
    {
        if (_jolt)
        {
            _jolt->SetPosition(JoltUtils::convert(pos));
        }
    }

    glm::vec3 CharacterImpl::getPosition() const noexcept
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetPosition());
    }

    Character::Character(const Config& config)
        : _impl(std::make_unique<CharacterImpl>(config))
    {
    }

    Character::Character(const Shape& shape)
        : Character(Config{ shape })
    {
    }

    Character::~Character()
    {
        // empty on purpose
    }

    CharacterImpl& Character::getImpl() noexcept
    {
        return *_impl;
    }

    const CharacterImpl& Character::getImpl() const noexcept
    {
        return *_impl;
    }

    Character& Character::setLinearVelocity(const glm::vec3& velocity)
    {
        _impl->setLinearVelocity(velocity);
        return *this;
    }

    glm::vec3 Character::getLinearVelocity()
    {
        return _impl->getLinearVelocity();
    }

    Character& Character::setPosition(const glm::vec3& pos) noexcept
    {
        _impl->setPosition(pos);
        return *this;
    }

    glm::vec3 Character::getPosition() const noexcept
    {
        return _impl->getPosition();
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

    void CharacterControllerImpl::init(CharacterController& ctrl, PhysicsSystemImpl& system)
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
        if (_jolt)
        {
            _jolt->SetPosition(JoltUtils::convert(pos));
        }
    }

    void CharacterControllerImpl::setLinearVelocity(const glm::vec3& velocity)
    {
        if (_jolt)
        {
            _jolt->SetLinearVelocity(JoltUtils::convert(velocity));
            _jolt->UpdateGroundVelocity();
        }
    }

    glm::vec3 CharacterControllerImpl::getLinearVelocity()
    {
        if (!_jolt)
        {
            return glm::vec3(0);
        }
        return JoltUtils::convert(_jolt->GetLinearVelocity());
    }

    void CharacterControllerImpl::addListener(ICharacterControllerListener& listener) noexcept
    {
        JoltUtils::addRefVector(_listeners, listener);
    }

    bool CharacterControllerImpl::removeListener(ICharacterControllerListener& listener) noexcept
    {
        return JoltUtils::removeRefVector(_listeners, listener);;
    }

    void CharacterControllerImpl::OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings)
    {
        auto itr = _collisions.find(bodyID2);
        if (itr == _collisions.end())
        {
            itr = _collisions.emplace(bodyID2, Collision{}).first;
        }
        auto& collision = itr->second;
        collision.normal = JoltUtils::convert(contactNormal);
        collision.contacts.push_back(JoltUtils::convert(contactPosition));
    }

    void CharacterControllerImpl::onCollisionEnter(RigidBody& other, const Collision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionEnter(_ctrl.value(), other, collision);
        }
    }

    void CharacterControllerImpl::onCollisionStay(RigidBody& other, const Collision& collision)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionStay(_ctrl.value(), other, collision);
        }
    }

    void CharacterControllerImpl::onCollisionExit(RigidBody& other)
    {
        for (auto& listener : _listeners)
        {
            listener->onCollisionExit(_ctrl.value(), other);
        }
    }

    bool CharacterControllerImpl::tryCreateCharacter(OptionalRef<Transform> trans) noexcept
    {
        if (_jolt != nullptr)
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

        _jolt = new JPH::CharacterVirtual(settings, JoltUtils::convert(pos), JoltUtils::convert(rot), 0, &_system->getJolt());
        _jolt->SetListener(this);
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

        CollisionMap oldCollisions = _collisions;
        _collisions.clear();

        JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
        auto& joltSystem = _system->getJolt();
        auto layer = (JPH::ObjectLayer)JoltLayer::Moving;
        auto gravity = -_jolt->GetUp() * joltSystem.GetGravity().Length();
        _jolt->ExtendedUpdate(deltaTime, gravity, updateSettings,
            joltSystem.GetDefaultBroadPhaseLayerFilter(layer),
            joltSystem.GetDefaultLayerFilter(layer),
            {}, {}, _system->getTempAllocator()
        );

        if (trans)
        {
            auto mat = JoltUtils::convert(_jolt->GetWorldTransform());
            auto parent = trans->getParent();
            if (parent)
            {
                mat = parent->getWorldInverse() * mat;
            }
            trans->setLocalMatrix(mat);
        }
        notifyCollisionListeners(oldCollisions);
    }

    void CharacterControllerImpl::notifyCollisionListeners(const CollisionMap& oldCollisions)
    {
        for (auto& elm : _collisions)
        {
            auto rigidBody = _system->getRigidBody(elm.first);
            if (!rigidBody)
            {
                continue;
            }
            if (oldCollisions.contains(elm.first))
            {
                onCollisionStay(rigidBody.value(), elm.second);
            }
            else
            {
                onCollisionEnter(rigidBody.value(), elm.second);
            }
        }
        for (auto& elm : oldCollisions)
        {
            auto rigidBody = _system->getRigidBody(elm.first);
            if (!rigidBody)
            {
                continue;
            }
            if (!_collisions.contains(elm.first))
            {
                onCollisionExit(rigidBody.value());
            }
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

    CharacterController& CharacterController::addListener(ICharacterControllerListener& listener) noexcept
    {
        _impl->addListener(listener);
        return *this;
    }

    bool CharacterController::removeListener(ICharacterControllerListener& listener) noexcept
    {
        return _impl->removeListener(listener);
    }
}