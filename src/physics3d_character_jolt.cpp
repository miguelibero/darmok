#include "detail/physics3d_character_jolt.hpp"
#include "detail/physics3d_jolt.hpp"
#include <darmok/physics3d_character.hpp>
#include <darmok/transform.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/string.hpp>
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

    expected<void, std::string> CharacterControllerImpl::init(Entity entity, CharacterController& ctrl, PhysicsSystem& system) noexcept
    {
        if (_system)
        {
            shutdown();
        }
        _system = system;
        _ctrl = ctrl;
        return doLoad(entity);
    }

    expected<void, std::string> CharacterControllerImpl::load(const Definition& def, Entity entity) noexcept
    {
        _def = def;
        return doLoad(entity);
    }

    expected<void, std::string> CharacterControllerImpl::doLoad(Entity entity) noexcept
    {
        _ctrl.reset();
        if (entity != entt::null)
        {
            return update(entity, 0.F);
        }
        return {};
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

    PhysicsShape CharacterControllerImpl::getShape() const noexcept
    {
        return darmok::convert<PhysicsShape>(_def.shape());
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
        auto result = _delegate->onAdjustBodyVelocity(_ctrl.value(), body.value(), linv, angv);
        if (!result)
        {
            _pendingErrors.push_back("onAdjustBodyVelocity: " + result.error());
        }
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
        auto result = _delegate->onContactValidate(_ctrl.value(), body.value());
        if (!result)
        {
            _pendingErrors.push_back("onContactValidate: " + result.error());
            return true;
        }
        return result.value();
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
        auto result = _delegate->onContactAdded(_ctrl.value(), body.value(), contact, darmokSettings);
        if (!result)
        {
            _pendingErrors.push_back("onContactAdded: " + result.error());
        }
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
        auto result = _delegate->onContactSolve(_ctrl.value(), body.value(), contact, charVel);
        if (!result)
        {
            _pendingErrors.push_back("onContactSolve: " + result.error());
        }
        newCharacterVelocity = JoltUtils::convert(charVel);
    }

    expected<void, std::string> CharacterControllerImpl::tryCreateCharacter(Transform& trans) noexcept
    {
        if (_jolt)
        {
            return {};
        }
        if (!_system)
        {
            return unexpected<std::string>{"uninitialized"};
        }
        auto joltSystem = getSystemImpl().getJolt();
        if (!joltSystem)
        {
            return unexpected<std::string>{"missing jolt system"};
        }
        auto transResult = getSystemImpl().tryLoadTransform(trans);
        if (!transResult)
        {
            return unexpected{ std::move(transResult).error() };
        }
        auto joltTrans = std::move(transResult).value();
        auto convertResult = JoltUtils::convert(convert<PhysicsShape>(_def.shape()), joltTrans.scale);
        if (!convertResult)
        {
            return unexpected{ std::move(convertResult).error() };
        }

        const JPH::Ref<JPH::CharacterVirtualSettings> settings = new JPH::CharacterVirtualSettings();
        settings->mMaxSlopeAngle = _def.max_slope_angle();
		settings->mMaxStrength = _def.max_strength();
        settings->mShape = convertResult.value();
        settings->mBackFaceMode = (JPH::EBackFaceMode)_def.back_face_mode();
        settings->mCharacterPadding = _def.padding();
        settings->mPenetrationRecoverySpeed = _def.penetration_recovery_speed();
        settings->mPredictiveContactDistance = _def.predictive_contact_distance();
        settings->mSupportingVolume = JoltUtils::convert(convert<Plane>(_def.supporting_plane()));

        auto userData = (uint64_t)_ctrl.ptr();
        _jolt = new JPH::CharacterVirtual(settings, joltTrans.position, joltTrans.rotation, userData, joltSystem.ptr());
        _jolt->SetListener(this);
        return {};
    }

    expected<void, std::string> CharacterControllerImpl::update(Entity entity, float deltaTime) noexcept
    {
        if (!_pendingErrors.empty())
        {
            auto err = StringUtils::joinErrors(_pendingErrors);
            _pendingErrors.clear();
            return unexpected{ std::move(err) };
        }
        if (!_system)
        {
            return unexpected<std::string>{"missing system"};
        }
        auto& scene = *_system->getScene();
        auto& trans = scene.getOrAddComponent<Transform>(entity);

        auto createResult = tryCreateCharacter(trans);
        if (!createResult)
        {
            return createResult;
        }
        if (!_jolt)
        {
            return unexpected<std::string>{"missing jolt controller"};
        }

        auto joltSystem = getSystemImpl().getJolt();
        if (!joltSystem)
        {
            return unexpected<std::string>{"missing jolt system"};
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

        auto layer = _def.layer();

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

        return {};
    }

    CharacterController::CharacterController() noexcept
        : _impl{ std::make_unique<CharacterControllerImpl>(createDefinition()) }
    {
    }

    CharacterController::CharacterController(const Definition& def) noexcept
        : _impl{ std::make_unique<CharacterControllerImpl>(def) }
    {        
    }

    CharacterController::CharacterController(const PhysicsShape& shape) noexcept
    {
        auto def = createDefinition();
        *def.mutable_shape() = convert<protobuf::PhysicsShape>(shape);
        _impl = std::make_unique<CharacterControllerImpl>(def);
    }

    CharacterController::~CharacterController() = default;
    CharacterController::CharacterController(CharacterController&& other) noexcept = default;
    CharacterController& CharacterController::operator=(CharacterController&& other) noexcept = default;

    expected<void, std::string> CharacterController::load(const Definition& def, IComponentLoadContext& context) noexcept
    {
        auto entity = context.getScene().getEntity(*this);
        return _impl->load(def, entity);
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

    CharacterController::Definition CharacterController::createDefinition() noexcept
    {
        Definition def;
        *def.mutable_shape() = PhysicsBody::createCharacterShapeDefinition();
        *def.mutable_supporting_plane() = PhysicsBody::createSupportingPlaneDefinition();
        def.mutable_up()->set_y(1.0F);
        def.set_max_slope_angle(glm::radians(50.F));

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

    PhysicsShape CharacterController::getShape() const noexcept
    {
        return _impl->getShape();
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