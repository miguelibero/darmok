#include "skeleton.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "asset.hpp"
#include "render_scene.hpp"
#include "camera.hpp"
#include <darmok/skeleton.hpp>
#include <darmok/asset.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
    void LuaSkeleton::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<Skeleton>("Skeleton", sol::no_constructor,
            sol::meta_function::to_string, &Skeleton::toString
        );
        lua.new_usertype<SkeletalAnimation>("SkeletalAnimation", sol::no_constructor,
            "name", sol::property(&SkeletalAnimation::getName),
            "duration", sol::property(&SkeletalAnimation::getDuration),
            sol::meta_function::to_string, &SkeletalAnimation::toString
        );
    }

    SkeletalAnimator& LuaSkeletalAnimator::addEntityComponent(LuaEntity& entity, const std::shared_ptr<Skeleton>& skel, const AnimationMap& anims, const Definition& def) noexcept
    {
        return entity.addComponent<SkeletalAnimator>(skel, anims, def);
    }

    OptionalRef<SkeletalAnimator>::std_t LuaSkeletalAnimator::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<SkeletalAnimator>();
    }

    std::optional<LuaEntity> LuaSkeletalAnimator::getEntity(const SkeletalAnimator& animator, std::shared_ptr<Scene>& scene) noexcept
    {
        return LuaScene::getEntity(scene, animator);
    }

    bool LuaSkeletalAnimator::play1(SkeletalAnimator& animator, const std::string& name) noexcept
    {
        return animator.play(name);
    }

    bool LuaSkeletalAnimator::play2(SkeletalAnimator& animator, const std::string& name, float stateSpeed) noexcept
    {
        return animator.play(name, stateSpeed);
    }

    OptionalRef<const ISkeletalAnimatorState>::std_t LuaSkeletalAnimator::getCurrentState(const SkeletalAnimator& animator) noexcept
    {
        return animator.getCurrentState();
    }

    std::string LuaSkeletalAnimator::getCurrentStateName(const SkeletalAnimator& animator) noexcept
    {
        if (auto state = animator.getCurrentState())
        {
            return std::string(state->getName());
        }
        return {};
    }

    OptionalRef<const ISkeletalAnimatorTransition>::std_t LuaSkeletalAnimator::getCurrentTransition(const SkeletalAnimator& animator) noexcept
    {
        return animator.getCurrentTransition();
    }

    void LuaSkeletalAnimator::setBlendPosition(SkeletalAnimator& animator, const VarLuaVecTable<glm::vec2>& pos) noexcept
    {
        animator.setBlendPosition(LuaGlm::tableGet(pos));
    }

    void LuaSkeletalAnimator::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<SkeletalAnimator>("SkeletalAnimator", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<SkeletalAnimator>::value),
            "add_entity_component", sol::overload(
                &LuaSkeletalAnimator::addEntityComponent
            ),
            "get_entity_component", &LuaSkeletalAnimator::getEntityComponent,
            "get_entity", &LuaSkeletalAnimator::getEntity,
            "play", sol::overload(
                &LuaSkeletalAnimator::play1,
                &LuaSkeletalAnimator::play2
            ),
            "stop", &SkeletalAnimator::stop,
            "pause", &SkeletalAnimator::pause,
            "playback_state", sol::property(&SkeletalAnimator::getPlaybackState),
            "playback_speed", sol::property(&SkeletalAnimator::getPlaybackSpeed, &SkeletalAnimator::setPlaybackSpeed),
            "blend_position", sol::property(&LuaSkeletalAnimator::setBlendPosition),
            "current_state", sol::property(&LuaSkeletalAnimator::getCurrentState),
            "current_state_name", sol::property(&LuaSkeletalAnimator::getCurrentStateName),
            "current_transition", sol::property(&LuaSkeletalAnimator::getCurrentTransition),
            "get_state_duration", &SkeletalAnimator::getStateDuration
        );
    }

    RenderableSkeleton& LuaRenderableSkeleton::addEntityComponent1(LuaEntity& entity) noexcept
    {
        return entity.addComponent<RenderableSkeleton>();
    }

    RenderableSkeleton& LuaRenderableSkeleton::addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& mat) noexcept
    {
        return entity.addComponent<RenderableSkeleton>(mat);
    }

    RenderableSkeleton& LuaRenderableSkeleton::addEntityComponent3(LuaEntity& entity, const std::shared_ptr<Material>& mat, const std::shared_ptr<Mesh>& boneMesh) noexcept
    {
        return entity.addComponent<RenderableSkeleton>(mat, boneMesh);
    }

    OptionalRef<RenderableSkeleton>::std_t LuaRenderableSkeleton::getEntityComponent(LuaEntity& entity) noexcept
    {
        return entity.getComponent<RenderableSkeleton>();
    }

    std::optional<LuaEntity> LuaRenderableSkeleton::getEntity(const RenderableSkeleton& skel, std::shared_ptr<Scene>& scene) noexcept
    {
        return LuaScene::getEntity(scene, skel);
    }

    void LuaRenderableSkeleton::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<RenderableSkeleton>("RenderableSkeleton", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<RenderableSkeleton>::value),
            "add_entity_component", sol::overload(
                &LuaRenderableSkeleton::addEntityComponent1,
                &LuaRenderableSkeleton::addEntityComponent2,
                &LuaRenderableSkeleton::addEntityComponent3
            ),
            "get_entity_component", &LuaRenderableSkeleton::getEntityComponent,
            "get_entity", &LuaRenderableSkeleton::getEntity
        );
    }

    SkeletalAnimationSceneComponent& LuaSkeletalAnimationSceneComponent::addSceneComponent(Scene& scene) noexcept
    {
        return scene.addSceneComponent<SkeletalAnimationSceneComponent>();
    }

    OptionalRef<SkeletalAnimationSceneComponent>::std_t LuaSkeletalAnimationSceneComponent::getSceneComponent(Scene& scene) noexcept
    {
        return scene.getSceneComponent<SkeletalAnimationSceneComponent>();
    }

    void LuaSkeletalAnimationSceneComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<SkeletalAnimationSceneComponent>("SkeletalAnimationSceneComponent", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<SkeletalAnimationRenderComponent>::value),
            "add_scene_component", &LuaSkeletalAnimationSceneComponent::addSceneComponent,
            "get_scene_component", &LuaSkeletalAnimationSceneComponent::getSceneComponent
        );
    }

    SkeletalAnimationRenderComponent& LuaSkeletalAnimationRenderComponent::addCameraComponent(Camera& cam) noexcept
    {
        return cam.addComponent<SkeletalAnimationRenderComponent>();
    }

    OptionalRef<SkeletalAnimationRenderComponent>::std_t LuaSkeletalAnimationRenderComponent::getCameraComponent(Camera& cam) noexcept
    {
        return cam.getComponent<SkeletalAnimationRenderComponent>();
    }

    void LuaSkeletalAnimationRenderComponent::bind(sol::state_view& lua) noexcept
    {
        lua.new_usertype<SkeletalAnimationRenderComponent>("SkeletalAnimationRenderComponent", sol::no_constructor,
            "type_id", sol::property(&entt::type_hash<SkeletalAnimationRenderComponent>::value),
            "add_camera_component", &LuaSkeletalAnimationRenderComponent::addCameraComponent,
            "get_camera_component", &LuaSkeletalAnimationRenderComponent::getCameraComponent
        );
    }
}