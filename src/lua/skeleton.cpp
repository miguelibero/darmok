#include "skeleton.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "asset.hpp"
#include "render.hpp"
#include <darmok/skeleton.hpp>
#include <darmok/asset.hpp>

namespace darmok
{
    void LuaSkeleton::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Skeleton>("Skeleton", sol::no_constructor);
        lua.new_usertype<SkeletalAnimation>("SkeletalAnimation", sol::no_constructor,
            "name", sol::property(&SkeletalAnimation::getName),
            "duration", sol::property(&SkeletalAnimation::getDuration)
        );
    }

    LuaSkeletalAnimator::LuaSkeletalAnimator(SkeletalAnimator& animator) noexcept
        : _animator(animator)
    {
    }

    LuaSkeletalAnimator LuaSkeletalAnimator::addEntityComponent(LuaEntity& entity, const std::shared_ptr<Skeleton>& skel, const AnimationMap& anims, const Config& config) noexcept
    {
        return entity.addComponent<SkeletalAnimator>(skel, anims, config);
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

    bool LuaSkeletalAnimator::play(const std::string& name) noexcept
    {
        return _animator->play(name);
    }

    void LuaSkeletalAnimator::setPlaybackSpeed(float speed) noexcept
    {
        _animator->setPlaybackSpeed(speed);
    }

    float LuaSkeletalAnimator::getPlaybackSpeed() const noexcept
    {
        return _animator->getPlaybackSpeed();
    }

    void LuaSkeletalAnimator::setBlendPosition(const VarLuaVecTable<glm::vec2>& pos) noexcept
    {
        _animator->setBlendPosition(LuaGlm::tableGet(pos));
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
            "play", sol::overload(&LuaSkeletalAnimator::play),
            "playback_speed", sol::property(&LuaSkeletalAnimator::getPlaybackSpeed, &LuaSkeletalAnimator::setPlaybackSpeed),
            "blend_position", sol::property(&LuaSkeletalAnimator::setBlendPosition)
        );
    }

    LuaRenderableSkeleton::LuaRenderableSkeleton(RenderableSkeleton& skel) noexcept
        : _skel(skel)
    {
    }

    LuaRenderableSkeleton LuaRenderableSkeleton::addEntityComponent1(LuaEntity& entity) noexcept
    {
        return entity.addComponent<RenderableSkeleton>();
    }

    LuaRenderableSkeleton LuaRenderableSkeleton::addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& mat) noexcept
    {
        return entity.addComponent<RenderableSkeleton>(mat);
    }

    LuaRenderableSkeleton LuaRenderableSkeleton::addEntityComponent3(LuaEntity& entity, const std::shared_ptr<Material>& mat, const std::shared_ptr<IMesh>& boneMesh) noexcept
    {
        return entity.addComponent<RenderableSkeleton>(mat, boneMesh);
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
                &LuaRenderableSkeleton::addEntityComponent2,
                &LuaRenderableSkeleton::addEntityComponent3
            ),
            "get_entity_component", &LuaRenderableSkeleton::getEntityComponent,
            "get_entity", &LuaRenderableSkeleton::getEntity
        );
    }

    LuaSkeletalAnimationSceneComponent::LuaSkeletalAnimationSceneComponent(SkeletalAnimationSceneComponent& comp) noexcept
        : _comp(comp)
    {
    }

    LuaSkeletalAnimationSceneComponent LuaSkeletalAnimationSceneComponent::addSceneComponent(LuaScene& scene) noexcept
    {
        return scene.getReal()->addSceneComponent<SkeletalAnimationSceneComponent>();
    }

    void LuaSkeletalAnimationSceneComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaSkeletalAnimationSceneComponent>("SkeletalAnimationSceneComponent", sol::no_constructor,
            "add_scene_component", &LuaSkeletalAnimationSceneComponent::addSceneComponent
        );
    }

    LuaSkeletalAnimationRenderComponent::LuaSkeletalAnimationRenderComponent(SkeletalAnimationRenderComponent& comp) noexcept
        : _comp(comp)
    {
    }

    LuaSkeletalAnimationRenderComponent LuaSkeletalAnimationRenderComponent::addRenderComponent(LuaRenderer& renderer) noexcept
    {
        return renderer.getReal().addComponent<SkeletalAnimationRenderComponent>();
    }

    void LuaSkeletalAnimationRenderComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<LuaSkeletalAnimationRenderComponent>("SkeletalAnimationRenderComponent", sol::no_constructor,
            "add_render_component", &LuaSkeletalAnimationRenderComponent::addRenderComponent
        );
    }
}