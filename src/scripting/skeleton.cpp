#include "skeleton.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include <darmok/skeleton.hpp>

namespace darmok
{
    LuaSkeleton::LuaSkeleton(const std::shared_ptr<Skeleton>& skel) noexcept
        : _skel(skel)
    {
    }

    std::string LuaSkeleton::to_string() const noexcept
    {
        if (!_skel)
        {
            return "Skeleton(null)";
        }
        return _skel->to_string();
    }

    std::shared_ptr<Skeleton> LuaSkeleton::getReal() const noexcept
    {
        return _skel;
    }

    void LuaSkeleton::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaSkeleton>("Skeleton", sol::no_constructor
        );
        LuaSkeletalAnimation::bind(lua);
    }

    LuaSkeletalAnimation::LuaSkeletalAnimation(const std::shared_ptr<SkeletalAnimation>& anim) noexcept
        : _anim(anim)
    {
    }

    std::string LuaSkeletalAnimation::to_string() const noexcept
    {
        if (!_anim)
        {
            return "SkeletalAnimation(null)";
        }
        return _anim->to_string();
    }

    std::shared_ptr<SkeletalAnimation> LuaSkeletalAnimation::getReal() const noexcept
    {
        return _anim;
    }

    void LuaSkeletalAnimation::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaSkeletalAnimation>("SkeletalAnimation", sol::no_constructor
        );
    }

    LuaSkeletalAnimationController::LuaSkeletalAnimationController(SkeletalAnimationController& ctrl) noexcept
        : _ctrl(ctrl)
    {
    }

    LuaSkeletalAnimationController LuaSkeletalAnimationController::addEntityComponent1(LuaEntity& entity, const LuaSkeleton& skel) noexcept
    {
        return entity.addComponent<SkeletalAnimationController>(skel.getReal());
    }

    LuaSkeletalAnimationController LuaSkeletalAnimationController::addEntityComponent2(LuaEntity& entity, const LuaSkeleton& skel, const std::vector<LuaSkeletalAnimation>& animations) noexcept
    {
        std::vector<std::shared_ptr<SkeletalAnimation>> realAnims;
        realAnims.reserve(animations.size());
        for (auto& anim : animations)
        {
            realAnims.push_back(anim.getReal());
        }
        return entity.addComponent<SkeletalAnimationController>(skel.getReal(), realAnims);
    }

    std::optional<LuaSkeletalAnimationController> LuaSkeletalAnimationController::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<SkeletalAnimationController, LuaSkeletalAnimationController>();
    }

    std::optional<LuaEntity> LuaSkeletalAnimationController::getEntity(LuaScene& scene) noexcept
    {
        if (!_ctrl)
        {
            return std::nullopt;
        }
        return scene.getEntity(_ctrl.value());
    }

    LuaSkeletalAnimationController& LuaSkeletalAnimationController::addAnimation(const LuaSkeletalAnimation& anim) noexcept
    {
        _ctrl->addAnimation(anim.getReal());
        return *this;
    }

    bool LuaSkeletalAnimationController::playAnimation(const std::string& name) noexcept
    {
        return _ctrl->playAnimation(name, true);
    }

    bool LuaSkeletalAnimationController::playAnimationOnce(const std::string& name) noexcept
    {
        return _ctrl->playAnimation(name, false);
    }

    LuaSkeletalAnimationController& LuaSkeletalAnimationController::setPlaybackSpeed(float speed) noexcept
    {
        _ctrl->setPlaybackSpeed(speed);
        return *this;
    }

    float LuaSkeletalAnimationController::getPlaybackSpped() const noexcept
    {
        return _ctrl->getPlaybackSpeed();
    }

    void LuaSkeletalAnimationController::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaSkeletalAnimationController>("SkeletalAnimationController", sol::no_constructor,
            "type_id", &entt::type_hash<SkeletalAnimationController>::value,
            "add_entity_component", sol::overload(
                &LuaSkeletalAnimationController::addEntityComponent1,
                &LuaSkeletalAnimationController::addEntityComponent2
            ),
            "get_entity_component", &LuaSkeletalAnimationController::getEntityComponent,
            "get_entity", &LuaSkeletalAnimationController::getEntity,
            "add_animation", &LuaSkeletalAnimationController::addAnimation,
            "play_animation", &LuaSkeletalAnimationController::playAnimation,
            "play_animation_once", &LuaSkeletalAnimationController::playAnimationOnce,
            "playback_speed", sol::property(&LuaSkeletalAnimationController::getPlaybackSpped, &LuaSkeletalAnimationController::setPlaybackSpeed)
        );
    }

    LuaRenderableSkeleton::LuaRenderableSkeleton(RenderableSkeleton& skel) noexcept
        : _skel(skel)
    {
    }

    LuaRenderableSkeleton LuaRenderableSkeleton::addEntityComponent1(LuaEntity& entity, const LuaMaterial& mat) noexcept
    {
        return entity.addComponent<RenderableSkeleton>(mat.getReal());
    }

    LuaRenderableSkeleton LuaRenderableSkeleton::addEntityComponent2(LuaEntity& entity, const LuaMaterial& mat, const LuaMesh& boneMesh) noexcept
    {
        return entity.addComponent<RenderableSkeleton>(mat.getReal(), boneMesh.getReal());
    }

    std::optional<LuaRenderableSkeleton> LuaRenderableSkeleton::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<RenderableSkeleton, LuaRenderableSkeleton>();
    }

    std::optional<LuaEntity> LuaRenderableSkeleton::getEntity(LuaScene& scene) noexcept
    {
        if (!_skel)
        {
            return std::nullopt;
        }
        return scene.getEntity(_skel.value());
    }

    void LuaRenderableSkeleton::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaRenderableSkeleton>("RenderableSkeleton", sol::no_constructor,
            "type_id", &entt::type_hash<RenderableSkeleton>::value,
            "add_entity_component", sol::overload(
                &LuaRenderableSkeleton::addEntityComponent1,
                &LuaRenderableSkeleton::addEntityComponent2
            ),
            "get_entity_component", &LuaRenderableSkeleton::getEntityComponent,
            "get_entity", &LuaRenderableSkeleton::getEntity
        );
    }
}