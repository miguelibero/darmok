#include "physics3d.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/physics3d.hpp>
#include <darmok/character.hpp>

namespace darmok::physics3d
{
    OptionalRef<PhysicsSystem>::std_t LuaPhysicsBody::getSystem(PhysicsBody& body) noexcept
    {
        return body.getSystem();
    }

    void LuaPhysicsBody::setPosition(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos)
    {
        body.setPosition(LuaGlm::tableGet(pos));
    }

    void LuaPhysicsBody::setRotation(PhysicsBody& body, const VarLuaTable<glm::quat>& rot)
    {
        body.setRotation(LuaGlm::tableGet(rot));
    }

    void LuaPhysicsBody::setLinearVelocity(PhysicsBody& body, const VarLuaTable<glm::vec3>& velocity)
    {
        body.setLinearVelocity(LuaGlm::tableGet(velocity));
    }

    PhysicsBody& LuaPhysicsBody::addTorque(PhysicsBody& body, const VarLuaTable<glm::vec3>& torque) noexcept
    {
        return body.addTorque(LuaGlm::tableGet(torque));
    }

    PhysicsBody& LuaPhysicsBody::addForce(PhysicsBody& body, const VarLuaTable<glm::vec3>& force) noexcept
    {
        return body.addForce(LuaGlm::tableGet(force));
    }

    PhysicsBody& LuaPhysicsBody::addImpulse(PhysicsBody& body, const VarLuaTable<glm::vec3>& impulse) noexcept
    {
        return body.addImpulse(LuaGlm::tableGet(impulse));
    }

    PhysicsBody& LuaPhysicsBody::move1(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot) noexcept
    {
        return body.move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot));
    }

    PhysicsBody& LuaPhysicsBody::move2(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos, const VarLuaTable<glm::quat>& rot, float deltaTime) noexcept
    {
        return body.move(LuaGlm::tableGet(pos), LuaGlm::tableGet(rot), deltaTime);
    }

    PhysicsBody& LuaPhysicsBody::movePosition1(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos) noexcept
    {
        return body.movePosition(LuaGlm::tableGet(pos));
    }

    PhysicsBody& LuaPhysicsBody::movePosition2(PhysicsBody& body, const VarLuaTable<glm::vec3>& pos, float deltaTime) noexcept
    {
        return body.movePosition(LuaGlm::tableGet(pos), deltaTime);
    }

    PhysicsBody& LuaPhysicsBody::addListener(PhysicsBody& body, const sol::table& table) noexcept
    {
        return body.addListener(std::make_unique<LuaCollisionListener>(table));
    }

    bool LuaPhysicsBody::removeListener(PhysicsBody& body, const sol::table& table) noexcept
    {
        return body.removeListeners(LuaCollisionListenerFilter(table)) > 0;
    }

    PhysicsBody& LuaPhysicsBody::addEntityComponent1(LuaEntity& entity, const Shape& shape) noexcept
    {
        return entity.addComponent<PhysicsBody>(shape);
    }

    PhysicsBody& LuaPhysicsBody::addEntityComponent2(LuaEntity& entity, const Shape& shape, MotionType motion) noexcept
    {
        return entity.addComponent<PhysicsBody>(shape, motion);
    }

    PhysicsBody& LuaPhysicsBody::addEntityComponent3(LuaEntity& entity, const Shape& shape, MotionType motion, bool trigger) noexcept
    {
        Config config;
        config.shape = shape;
        config.motion = motion;
        config.trigger = trigger;
        return entity.addComponent<PhysicsBody>(config);
    }

    PhysicsBody& LuaPhysicsBody::addEntityComponent4(LuaEntity& entity, const Config& config) noexcept
    {
        return entity.addComponent<PhysicsBody>(config);
    }

    PhysicsBody& LuaPhysicsBody::addEntityComponent5(LuaEntity& entity, const CharacterConfig& config) noexcept
    {
        return entity.addComponent<PhysicsBody>(config);
    }

    OptionalRef<PhysicsBody>::std_t LuaPhysicsBody::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<PhysicsBody>();
    }

    std::optional<LuaEntity> LuaPhysicsBody::getEntity(const PhysicsBody& body, const std::shared_ptr<Scene>& scene) noexcept
    {
        return LuaScene::getEntity(scene, body);
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
            "inertia_factor", &PhysicsBodyConfig::inertiaFactor,
            "friction", &PhysicsBodyConfig::friction,
            "gravity_factor", &PhysicsBodyConfig::gravityFactor,
            "layer", &PhysicsBodyConfig::layer,
            "trigger", &PhysicsBodyConfig::trigger
        );
        lua.new_usertype<CharacterConfig>("Physics3dCharacterConfig", sol::default_constructor,
            "shape", &CharacterConfig::shape,
            "up", &CharacterConfig::up,
            "supporting_plane", &CharacterConfig::supportingPlane,
            "max_slope_angle", &CharacterConfig::maxSlopeAngle,
            "layer", &CharacterConfig::layer,
            "mass", &CharacterConfig::mass,
            "friction", &CharacterConfig::friction,
            "gravity_factor", &CharacterConfig::gravityFactor
        );
        lua.new_usertype<PhysicsBody>("Physics3dBody", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<PhysicsBody>::value),
            sol::meta_function::to_string, &PhysicsBody::toString,
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
            "grounded", sol::property(&PhysicsBody::isGrounded),
            "ground_state", sol::property(&PhysicsBody::getGroundState),
            "shape", sol::property(&PhysicsBody::getShape),
            "motion_type", sol::property(&PhysicsBody::getMotionType),
            "position", sol::property(&PhysicsBody::getPosition, &LuaPhysicsBody::setPosition),
            "rotation", sol::property(&PhysicsBody::getRotation, &LuaPhysicsBody::setRotation),
            "linear_velocity", sol::property(&PhysicsBody::getLinearVelocity, &LuaPhysicsBody::setLinearVelocity),
            "inverse_mass", sol::property(&PhysicsBody::getInverseMass, &PhysicsBody::setInverseMass),
            "active", sol::property(&PhysicsBody::isActive),
            "system", sol::property(&LuaPhysicsBody::getSystem),
            "activate", &PhysicsBody::activate,
            "deactivate", &PhysicsBody::deactivate,
            "enabled", sol::property(&PhysicsBody::isEnabled, &PhysicsBody::setEnabled),
            "add_torque", &LuaPhysicsBody::addTorque,
            "add_force", &LuaPhysicsBody::addForce,
            "add_impulse", &LuaPhysicsBody::addImpulse,
            "move", sol::overload(&LuaPhysicsBody::move1, &LuaPhysicsBody::move2),
            "move_position", sol::overload(&LuaPhysicsBody::movePosition1, &LuaPhysicsBody::movePosition2)
        );
    }
}