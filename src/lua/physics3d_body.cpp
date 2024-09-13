#include "physics3d.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/physics3d.hpp>
#include <darmok/character.hpp>

namespace darmok::physics3d
{
    LuaPhysicsBody::LuaPhysicsBody(PhysicsBody& body) noexcept
        : _body(body)
    {
    }

    LuaPhysicsBody::~LuaPhysicsBody() noexcept
    {
        _body.removeListener(*this);
    }

    bool LuaPhysicsBody::isGrounded() const noexcept
    {
        return _body.isGrounded();
    }

    GroundState LuaPhysicsBody::getGroundState() const noexcept
    {
        return _body.getGroundState();
    }

    const LuaPhysicsBody::Shape& LuaPhysicsBody::getShape() const noexcept
    {
        return _body.getShape();
    }

    LuaPhysicsBody::MotionType LuaPhysicsBody::getMotionType() const noexcept
    {
        return _body.getMotionType();
    }

    void LuaPhysicsBody::setPosition(const VarLuaTable<glm::vec3>& pos)
    {
        _body.setPosition(LuaGlm::tableGet(pos));
    }

    glm::vec3 LuaPhysicsBody::getPosition() const
    {
        return _body.getPosition();
    }

    void LuaPhysicsBody::setRotation(const VarLuaTable<glm::quat>& rot)
    {
        _body.setRotation(LuaGlm::tableGet(rot));
    }

    glm::quat LuaPhysicsBody::getRotation() const
    {
        return _body.getRotation();
    }

    void LuaPhysicsBody::setLinearVelocity(const VarLuaTable<glm::vec3>& velocity)
    {
        _body.setLinearVelocity(LuaGlm::tableGet(velocity));
    }

    glm::vec3 LuaPhysicsBody::getLinearVelocity() const
    {
        return _body.getLinearVelocity();
    }

    bool LuaPhysicsBody::getActive() const
    {
        return _body.isActive();
    }

    LuaPhysicsBody& LuaPhysicsBody::activate()
    {
        _body.activate();
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::deactivate()
    {
        _body.deactivate();
        return *this;
    }

    bool LuaPhysicsBody::getEnabled() const
    {
        return _body.isEnabled();
    }

    LuaPhysicsBody& LuaPhysicsBody::setEnabled(bool enabled)
    {
        _body.setEnabled(enabled);
        return *this;
    }

    std::string LuaPhysicsBody::toString() const noexcept
    {
        return _body.toString();
    }

    LuaPhysicsBody& LuaPhysicsBody::addTorque(const VarLuaTable<glm::vec3>& torque) noexcept
    {
        _body.addTorque(LuaGlm::tableGet(torque));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::addForce(const VarLuaTable<glm::vec3>& force) noexcept
    {
        _body.addForce(LuaGlm::tableGet(force));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::addImpulse(const VarLuaTable<glm::vec3>& impulse) noexcept
    {
        _body.addImpulse(LuaGlm::tableGet(impulse));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::move1(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot) noexcept
    {
        _body.move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::move2(const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot, float deltaTime) noexcept
    {
        _body.move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot), deltaTime);
        return *this;
    }


    LuaPhysicsBody& LuaPhysicsBody::movePosition1(const VarLuaTable<glm::vec3>& pos) noexcept
    {
        _body.movePosition(LuaGlm::tableGet(pos));
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::movePosition2(const VarLuaTable<glm::vec3>& pos, float deltaTime) noexcept
    {
        _body.movePosition(LuaGlm::tableGet(pos), deltaTime);
        return *this;
    }

    LuaPhysicsBody& LuaPhysicsBody::addListener(const sol::table& listener) noexcept
    {
        if (_listeners.empty())
        {
            _body.addListener(*this);
        }
        _listeners.emplace_back(listener);
        return *this;
    }

    bool LuaPhysicsBody::removeListener(const sol::table& listener) noexcept
    {
        auto itr = std::find(_listeners.begin(), _listeners.end(), listener);
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr);
        if (_listeners.empty())
        {
            _body.removeListener(*this);
        }
        return true;
    }

    OptionalRef<LuaPhysicsBody> LuaPhysicsBody::getLuaBody(PhysicsBody& body) const noexcept
    {
        if (!_body.getSystem())
        {
            return nullptr;
        }
        auto optScene = _body.getSystem()->getScene();
        if (!optScene)
        {
            return nullptr;
        }
        auto& scene = optScene.value();
        auto entity = scene.getEntity(body);
        if (entity == entt::null)
        {
            return nullptr;
        }
        return scene.getComponent<LuaPhysicsBody>(entity);
    }

    void LuaPhysicsBody::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableDelegates(_listeners, "on_collision_enter", "running physics collision enter",
            [&collision, &luaBody1, &luaBody2](auto& func, auto& self)
            {
                return func(self, luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsBody::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableDelegates(_listeners, "on_collision_stay", "running physics collision stay",
            [&collision, &luaBody1, &luaBody2](auto& func, auto& self)
            {
                return func(self, luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsBody::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableDelegates(_listeners, "on_collision_exit", "running physics collision exit",
            [&luaBody1, &luaBody2](auto& func, auto& self)
            {
                return func(self, luaBody1, luaBody2);
            });
    }

    LuaPhysicsBody& LuaPhysicsBody::addEntityComponent1(LuaEntity& entity, const Shape& shape) noexcept
    {
        return entity.addWrapperComponent<LuaPhysicsBody, PhysicsBody>(shape);
    }

    LuaPhysicsBody& LuaPhysicsBody::addEntityComponent2(LuaEntity& entity, const Shape& shape, MotionType motion) noexcept
    {
        return entity.addWrapperComponent<LuaPhysicsBody, PhysicsBody>(shape, motion);
    }

    LuaPhysicsBody& LuaPhysicsBody::addEntityComponent3(LuaEntity& entity, const Shape& shape, MotionType motion, bool trigger) noexcept
    {
        Config config;
        config.motion = motion;
        config.trigger = trigger;
        return entity.addWrapperComponent<LuaPhysicsBody, PhysicsBody>(config);
    }

    LuaPhysicsBody& LuaPhysicsBody::addEntityComponent4(LuaEntity& entity, const Config& config) noexcept
    {
        return entity.addWrapperComponent<LuaPhysicsBody, PhysicsBody>(config);
    }

    LuaPhysicsBody& LuaPhysicsBody::addEntityComponent5(LuaEntity& entity, const CharacterConfig& config) noexcept
    {
        return entity.addWrapperComponent<LuaPhysicsBody, PhysicsBody>(config);
    }

    OptionalRef<LuaPhysicsBody>::std_t LuaPhysicsBody::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<LuaPhysicsBody>();
    }

    std::optional<LuaEntity> LuaPhysicsBody::getEntity(LuaScene& scene) noexcept
    {
        return scene.getEntity(_body);
    }
    
    void LuaPhysicsBody::bind(sol::state_view& lua) noexcept
    {
        lua.new_enum<MotionType>("Physics3dMotionType", {
            { "Static", MotionType::Static },
            { "Dynamic", MotionType::Dynamic },
            { "Kinematic", MotionType::Kinematic },
        });
        lua.new_enum<GroundState>("Physics3dGroundState", {
            { "Grounded", GroundState::Grounded },
            { "GroundedSteep", GroundState::GroundedSteep },
            { "NotSupported", GroundState::NotSupported },
            { "Air", GroundState::Air },
        });
        lua.new_usertype<PhysicsBodyConfig>("Physics3dBodyConfig", sol::default_constructor,
            "shape", &PhysicsBodyConfig::shape,
            "motion", &PhysicsBodyConfig::motion,
            "mass", &PhysicsBodyConfig::mass,
            "inertiaFactor", &PhysicsBodyConfig::inertiaFactor,
            "friction", &PhysicsBodyConfig::friction,
            "gravity_factor", &PhysicsBodyConfig::gravityFactor,
            "layer", &PhysicsBodyConfig::layer,
            "trigger", &PhysicsBodyConfig::trigger
        );
        lua.new_usertype<CharacterConfig>("Physics3dCharacterConfig", sol::default_constructor,
            "shape", &CharacterConfig::shape,
            "up", &CharacterConfig::up,
            "supportingPlane", &CharacterConfig::supportingPlane,
            "maxSlopeAngle", &CharacterConfig::maxSlopeAngle,
            "layer", &CharacterConfig::layer,
            "mass", &CharacterConfig::mass,
            "friction", &CharacterConfig::friction,
            "gravityFactor", &CharacterConfig::gravityFactor
        );
        lua.new_usertype<LuaPhysicsBody>("Physics3dBody", sol::no_constructor,
            "type_id", &entt::type_hash<PhysicsBody>::value,
            sol::meta_function::to_string, &LuaPhysicsBody::toString,
            "add_entity_component", sol::overload(
                &LuaPhysicsBody::addEntityComponent1,
                &LuaPhysicsBody::addEntityComponent2,
                &LuaPhysicsBody::addEntityComponent3,
                &LuaPhysicsBody::addEntityComponent4,
                &LuaPhysicsBody::addEntityComponent5
            ),
            "get_entity_component", &LuaPhysicsBody::getEntityComponent,
            "get_entity", &LuaPhysicsBody::getEntity,
            "add_listener", &LuaPhysicsBody::addListener,
            "remove_listener", &LuaPhysicsBody::removeListener,
            "grounded", sol::property(&LuaPhysicsBody::isGrounded),
            "ground_state", sol::property(&LuaPhysicsBody::getGroundState),
            "shape", sol::property(&LuaPhysicsBody::getShape),
            "motion_type", sol::property(&LuaPhysicsBody::getMotionType),
            "position", sol::property(&LuaPhysicsBody::getPosition, &LuaPhysicsBody::setPosition),
            "rotation", sol::property(&LuaPhysicsBody::getRotation, &LuaPhysicsBody::setRotation),
            "linear_velocity", sol::property(&LuaPhysicsBody::getLinearVelocity, &LuaPhysicsBody::setLinearVelocity),
            "active", sol::property(&LuaPhysicsBody::getActive),
            "activate", &LuaPhysicsBody::activate,
            "deactivate", &LuaPhysicsBody::deactivate,
            "enabled", sol::property(&LuaPhysicsBody::getEnabled, &LuaPhysicsBody::setEnabled)
        );
    }
}