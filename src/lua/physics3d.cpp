#include "physics3d.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/physics3d.hpp>
#include <darmok/character.hpp>

namespace darmok::physics3d
{
    LuaPhysicsSystem::LuaPhysicsSystem(PhysicsSystem& system) noexcept
        : _system(system)
    {
    }

    void LuaPhysicsSystem::shutdown() noexcept
    {
        _system.removeListener(*this);
        _system.removeUpdater(*this);
    }

    LuaPhysicsSystem& LuaPhysicsSystem::registerUpdate(const sol::protected_function& func) noexcept
    {
        if (_updates.empty())
        {
            _system.addUpdater(*this);
        }
        _updates.emplace_back(func);
        return *this;
    }

    bool LuaPhysicsSystem::unregisterUpdate(const sol::protected_function& func) noexcept
    {
        auto itr = std::find(_updates.begin(), _updates.end(), func);
        if (itr == _updates.end())
        {
            return false;
        }
        _updates.erase(itr);
        if (_updates.empty())
        {
            _system.removeUpdater(*this);
        }
        return true;
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addListener(const sol::table& listener) noexcept
    {
        if (_listeners.empty())
        {
            _system.addListener(*this);
        }
        _listeners.emplace_back(listener);
        return *this;
    }

    bool LuaPhysicsSystem::removeListener(const sol::table& listener) noexcept
    {
        auto itr = std::find(_listeners.begin(), _listeners.end(), listener);
        if (itr == _listeners.end())
        {
            return false;
        }
        _listeners.erase(itr);
        if (_listeners.empty())
        {
            _system.removeListener(*this);
        }
        return true;
    }

    PhysicsSystem& LuaPhysicsSystem::getReal() noexcept
    {
        return _system;
    }

    const PhysicsSystem& LuaPhysicsSystem::getReal() const noexcept
    {
        return _system;
    }

    OptionalRef<Transform>::std_t LuaPhysicsSystem::getRootTransform() const noexcept
    {
        return _system.getRootTransform();
    }

    void LuaPhysicsSystem::setRootTransform(OptionalRef<Transform>::std_t root) noexcept
    {
        _system.setRootTransform(root);
    }

    void LuaPhysicsSystem::fixedUpdate(float fixedDeltaTime)
    {
        for (auto& update : _updates)
        {
            if (!update)
            {
                continue;
            }
            auto result = update(fixedDeltaTime);
            if (!result.valid())
            {
                LuaUtils::logError("running fixed update", result);
            }
        }
    }


    OptionalRef<LuaPhysicsBody> LuaPhysicsSystem::getLuaBody(PhysicsBody& body) const noexcept
    {
        auto optScene = _system.getScene();
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

    void LuaPhysicsSystem::onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableListeners(_listeners, "on_collision_enter", "running physics collision enter",
            [&collision, &luaBody1, &luaBody2](auto& func)
            {
                return func(luaBody1, luaBody2, collision);
        });
    }

    void LuaPhysicsSystem::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableListeners(_listeners, "on_collision_stay", "running physics collision stay",
            [&collision, &luaBody1, &luaBody2](auto& func)
            {
                return func(luaBody1, luaBody2, collision);
        });
    }

    void LuaPhysicsSystem::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableListeners(_listeners, "on_collision_exit", "running physics collision exit",
            [&luaBody1, &luaBody2](auto& func)
            {
                return func(luaBody1, luaBody2);
        });
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast1(const Ray& ray) noexcept
    {
        return _system.raycast(ray);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast2(const Ray& ray, float maxDistance) noexcept
    {
        return _system.raycast(ray, maxDistance);
    }

    std::optional<RaycastHit> LuaPhysicsSystem::raycast3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _system.raycast(ray, maxDistance, layerMask);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll1(const Ray& ray) noexcept
    {
        return _system.raycastAll(ray);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll2(const Ray& ray, float maxDistance) noexcept
    {
        return _system.raycastAll(ray, maxDistance);
    }

    std::vector<RaycastHit> LuaPhysicsSystem::raycastAll3(const Ray& ray, float maxDistance, uint16_t layerMask) noexcept
    {
        return _system.raycastAll(ray, maxDistance, layerMask);
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addSceneComponent1(LuaScene& scene) noexcept
    {
        return scene.addSceneComponent<LuaPhysicsSystem, PhysicsSystem>();
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addSceneComponent2(LuaScene& scene, const Config& config) noexcept
    {
        return scene.addSceneComponent<LuaPhysicsSystem, PhysicsSystem>(config);
    }

    LuaPhysicsSystem& LuaPhysicsSystem::addSceneComponent3(LuaScene& scene, const Config& config, bx::AllocatorI& alloc) noexcept
    {
        return scene.addSceneComponent<LuaPhysicsSystem, PhysicsSystem>(config, alloc);
    }

    void LuaPhysicsSystem::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Config>("Physics3dSystemConfig",
            sol::constructors<Config()>(),
            "maxBodies", &Config::maxBodies,
            "numBodyMutexes", &Config::numBodyMutexes,
            "maxBodyPairs", &Config::maxBodyPairs,
            "maxContactConstraints", &Config::maxContactConstraints,
            "fixedDeltaTime", &Config::fixedDeltaTime,
            "collisionSteps", &Config::collisionSteps,
            "gravity", &Config::gravity
        );

        lua.new_usertype<LuaPhysicsSystem>("Physics3dSystem", sol::no_constructor,
            "register_update", &LuaPhysicsSystem::registerUpdate,
            "unregister_update", &LuaPhysicsSystem::unregisterUpdate,
            "add_listener", &LuaPhysicsSystem::addListener,
            "remove_listener", &LuaPhysicsSystem::removeListener,
            "root_transform", sol::property(&LuaPhysicsSystem::getRootTransform, &LuaPhysicsSystem::setRootTransform),
            "raycast", sol::overload(
                &LuaPhysicsSystem::raycast1,
                &LuaPhysicsSystem::raycast2,
                &LuaPhysicsSystem::raycast3
            ),
            "raycast_all", sol::overload(
                &LuaPhysicsSystem::raycastAll1,
                &LuaPhysicsSystem::raycastAll2,
                &LuaPhysicsSystem::raycastAll3
            ),
            "add_scene_component", sol::overload(
                &LuaPhysicsSystem::addSceneComponent1,
                &LuaPhysicsSystem::addSceneComponent2 /*,
                &LuaPhysicsSystem::addSceneComponent3*/
            )
        );
    }

    LuaPhysicsBody::LuaPhysicsBody(PhysicsBody& body) noexcept
        : _body(body)
    {
    }

    LuaPhysicsBody::~LuaPhysicsBody() noexcept
    {
        _body.removeListener(*this);
    }

    const LuaPhysicsBody::Shape& LuaPhysicsBody::getShape() const noexcept
    {
        return _body.getShape();
    }

    LuaPhysicsBody::MotionType LuaPhysicsBody::getMotionType() const noexcept
    {
        return _body.getMotionType();
    }

    void LuaPhysicsBody::setPosition(const VarLuaTable<glm::vec3>& pos) noexcept
    {
        _body.setPosition(LuaGlm::tableGet(pos));
    }

    glm::vec3 LuaPhysicsBody::getPosition() const noexcept
    {
        return _body.getPosition();
    }

    void LuaPhysicsBody::setRotation(const VarLuaTable<glm::quat>& rot) noexcept
    {
        _body.setRotation(LuaGlm::tableGet(rot));
    }

    glm::quat LuaPhysicsBody::getRotation() const noexcept
    {
        return _body.getRotation();
    }

    void LuaPhysicsBody::setLinearVelocity(const VarLuaTable<glm::vec3>& velocity) noexcept
    {
        _body.setLinearVelocity(LuaGlm::tableGet(velocity));
    }

    glm::vec3 LuaPhysicsBody::getLinearVelocity() const noexcept
    {
        return _body.getLinearVelocity();
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
        LuaUtils::callTableListeners(_listeners, "on_collision_enter", "running physics collision enter",
            [&collision, &luaBody1, &luaBody2](auto& func)
            {
                return func(luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsBody::onCollisionStay(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableListeners(_listeners, "on_collision_stay", "running physics collision stay",
            [&collision, &luaBody1, &luaBody2](auto& func)
            {
                return func(luaBody1, luaBody2, collision);
            });
    }

    void LuaPhysicsBody::onCollisionExit(PhysicsBody& body1, PhysicsBody& body2)
    {
        auto& luaBody1 = getLuaBody(body1).value();
        auto& luaBody2 = getLuaBody(body2).value();
        LuaUtils::callTableListeners(_listeners, "on_collision_exit", "running physics collision exit",
            [&luaBody1, &luaBody2](auto& func)
            {
                return func(luaBody1, luaBody2);
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
            "shape", sol::property(&LuaPhysicsBody::getShape),
            "motion_type", sol::property(&LuaPhysicsBody::getMotionType),
            "position", sol::property(&LuaPhysicsBody::getPosition, &LuaPhysicsBody::setPosition),
            "rotation", sol::property(&LuaPhysicsBody::getRotation, &LuaPhysicsBody::setRotation),
            "linear_velocity", sol::property(&LuaPhysicsBody::getLinearVelocity, &LuaPhysicsBody::setLinearVelocity)
        );
    }
}