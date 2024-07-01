#include "character_jolt.hpp"
#include "physics3d_jolt.hpp"
#include <darmok/character.hpp>
#include <Jolt/Physics/PhysicsSystem.h>

namespace darmok::physics3d
{
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

    bool CharacterControllerImpl::isGrounded() const noexcept
    {
        auto state = getGroundState();
        return state == GroundState::Grounded || state == GroundState::GroundedSteep;
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
        return JoltUtils::convertPosition(_jolt->GetPosition(), _config.shape);
    }

    void CharacterControllerImpl::setPosition(const glm::vec3& pos) noexcept
    {
        auto rb = getPhysicsBody();
        if (rb)
        {
            rb->setPosition(pos);
        }
        if (_jolt)
        {
            _jolt->SetPosition(JoltUtils::convertPosition(pos, _config.shape));
        }
    }

    void CharacterControllerImpl::setLinearVelocity(const glm::vec3& velocity)
    {
        auto rb = getPhysicsBody();
        if (rb)
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

    void CharacterControllerImpl::addListener(ICharacterControllerListener& listener) noexcept
    {
        JoltUtils::addRefVector(_listeners, listener);
    }

    bool CharacterControllerImpl::removeListener(ICharacterControllerListener& listener) noexcept
    {
        return JoltUtils::removeRefVector(_listeners, listener);;
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

    void CharacterControllerImpl::OnContactAdded(const JPH::CharacterVirtual* character, const JPH::BodyID& bodyID2, const JPH::SubShapeID& subShapeID2, JPH::RVec3Arg contactPosition, JPH::Vec3Arg contactNormal, JPH::CharacterContactSettings& settings)
    {
        auto body = getPhysicsBody();
        if (body && body->getImpl().getBodyId() == bodyID2)
        {
            return;
        }
        auto itr = _collisions.find(bodyID2);
        if (itr == _collisions.end())
        {
            itr = _collisions.emplace(bodyID2, Collision{}).first;
        }
        auto& collision = itr->second;
        collision.normal = JoltUtils::convert(contactNormal);
        collision.contacts.push_back(JoltUtils::convert(contactPosition));
    }

    bool CharacterControllerImpl::tryCreateCharacter(OptionalRef<Transform> trans) noexcept
    {
        if (_jolt != nullptr)
        {
            return false;
        }
        auto joltSystem = _system->getJolt();
        if (!joltSystem)
        {
            return false;
        }

        JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mMaxSlopeAngle = _config.maxSlopeAngle;
        settings->mMaxStrength = _config.maxStrength;
        settings->mShape = JoltUtils::convert(_config.shape);
        settings->mBackFaceMode = (JPH::EBackFaceMode)_config.backFaceMode;
        settings->mCharacterPadding = _config.padding;
        settings->mShapeOffset = JoltUtils::convert(JoltUtils::getOrigin(_config.shape));
        settings->mPenetrationRecoverySpeed = _config.penetrationRecoverySpeed;
        settings->mPredictiveContactDistance = _config.predictiveContactDistance;
        settings->mSupportingVolume = JPH::Plane(JoltUtils::convert(_config.supportingPlane.normal), _config.supportingPlane.constant);

        auto [pos, rot] = JoltUtils::convert(_config.shape, trans);
        auto userData = (uint64_t)_ctrl.ptr();
        _jolt = new JPH::CharacterVirtual(settings, pos, rot, userData, joltSystem.ptr());
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
        auto trans = scene.getComponent<Transform>(entity);

        tryCreateCharacter(trans);
        if (!_jolt)
        {
            return;
        }

        auto joltSystem = _system->getJolt();
        if (!joltSystem)
        {
            return;
        }

        JPH::IgnoreMultipleBodiesFilter bodyFilter;

        auto rb = getPhysicsBody();
        if (rb)
        {
             auto bodyId = rb->getImpl().getBodyId();
             bodyFilter.IgnoreBody(bodyId);
             auto& iface = joltSystem->GetBodyInterface();
             JPH::Vec3 pos;
             JPH::Quat rot;
             iface.GetPositionAndRotation(bodyId, pos, rot);
             _jolt->SetPosition(pos);
             _jolt->SetRotation(rot);
             _jolt->SetLinearVelocity(iface.GetLinearVelocity(bodyId));
        }

        CollisionMap oldCollisions = _collisions;
        _collisions.clear();
        JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;

        auto gravity = -_jolt->GetUp() * joltSystem->GetGravity().Length();
        _jolt->ExtendedUpdate(deltaTime, gravity, updateSettings,
            joltSystem->GetDefaultBroadPhaseLayerFilter(_config.layer),
            joltSystem->GetDefaultLayerFilter(_config.layer),
            bodyFilter, {}, _system->getTempAllocator()
        );

        notifyCollisionListeners(oldCollisions);

        // if the entity has a rigid body, that component will update the transform
        if (!getPhysicsBody() && trans)
        {
            auto mat = JoltUtils::convert(_jolt->GetWorldTransform(), _config.shape, trans.value());
            trans->setLocalMatrix(mat);
        }
    }

    void CharacterControllerImpl::notifyCollisionListeners(const CollisionMap& oldCollisions)
    {
        if (!_system)
        {
            return;
        }
        for (auto& elm : _collisions)
        {
            auto body = _system->getPhysicsBody(elm.first);
            if (!body)
            {
                continue;
            }
            if (oldCollisions.contains(elm.first))
            {
                onCollisionStay(body.value(), elm.second);
            }
            else
            {
                onCollisionEnter(body.value(), elm.second);
            }
        }
        for (auto& elm : oldCollisions)
        {
            auto body = _system->getPhysicsBody(elm.first);
            if (!body)
            {
                continue;
            }
            if (!_collisions.contains(elm.first))
            {
                onCollisionExit(body.value());
            }
        }
    }

    void CharacterControllerImpl::onCollisionEnter(PhysicsBody& other, const Collision& collision)
    {
        if (!_ctrl)
        {
            return;
        }
        for (auto& listener : _listeners)
        {
            listener->onCollisionEnter(_ctrl.value(), other, collision);
        }
    }

    void CharacterControllerImpl::onCollisionStay(PhysicsBody& other, const Collision& collision)
    {
        if (!_ctrl)
        {
            return;
        }
        for (auto& listener : _listeners)
        {
            listener->onCollisionStay(_ctrl.value(), other, collision);
        }
    }

    void CharacterControllerImpl::onCollisionExit(PhysicsBody& other)
    {
        if (!_ctrl)
        {
            return;
        }
        for (auto& listener : _listeners)
        {
            listener->onCollisionExit(_ctrl.value(), other);
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