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

    LuaSkeletalAnimator::LuaSkeletalAnimator(SkeletalAnimator& animator) noexcept
        : _animator(animator)
    {
    }

    LuaSkeletalAnimator LuaSkeletalAnimator::addEntityComponent(LuaEntity& entity, const LuaSkeleton& skel, const Config& config) noexcept
    {
        return entity.addComponent<SkeletalAnimator>(skel.getReal(), config.getReal());
    }

    std::optional<LuaSkeletalAnimator> LuaSkeletalAnimator::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<SkeletalAnimator, LuaSkeletalAnimator>();
    }

    std::optional<LuaEntity> LuaSkeletalAnimator::getEntity(LuaScene& scene) noexcept
    {
        if (!_animator)
        {
            return std::nullopt;
        }
        return scene.getEntity(_animator.value());
    }

    bool LuaSkeletalAnimator::play1(const std::string& name) noexcept
    {
        return _animator->play(name);
    }

    bool LuaSkeletalAnimator::play2(const std::string& name, float normalizedTime) noexcept
    {
        return _animator->play(name, normalizedTime);
    }

    LuaSkeletalAnimator& LuaSkeletalAnimator::setPlaybackSpeed(float speed) noexcept
    {
        _animator->setPlaybackSpeed(speed);
        return *this;
    }

    float LuaSkeletalAnimator::getPlaybackSpeed() const noexcept
    {
        return _animator->getPlaybackSpeed();
    }

    void LuaSkeletalAnimator::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaSkeletalAnimator>("SkeletalAnimator", sol::no_constructor,
            "type_id", &entt::type_hash<SkeletalAnimator>::value,
            "add_entity_component", sol::overload(
                &LuaSkeletalAnimator::addEntityComponent
            ),
            "get_entity_component", &LuaSkeletalAnimator::getEntityComponent,
            "get_entity", &LuaSkeletalAnimator::getEntity,
            "play", sol::overload(&LuaSkeletalAnimator::play1, &LuaSkeletalAnimator::play2),
            "playback_speed", sol::property(&LuaSkeletalAnimator::getPlaybackSpeed, &LuaSkeletalAnimator::setPlaybackSpeed)
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