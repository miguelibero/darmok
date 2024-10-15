#pragma once

#include "lua.hpp"
#include "glm.hpp"

#include <memory>
#include <unordered_map>
#include <string>

#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Skeleton;
    class SkeletalAnimation;
    struct SkeletalAnimatorConfig;
    class SkeletalAnimator;
    class ISkeletalAnimatorState;
    class ISkeletalAnimatorTransition;

    class LuaEntity;
    class Scene;

    struct LuaSkeleton final
    {
        static void bind(sol::state_view& lua) noexcept;
    };

    class LuaSkeletalAnimator final
	{
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:

        using Config = SkeletalAnimatorConfig;
        using AnimationMap = std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>>;

        static void setBlendPosition(SkeletalAnimator& animator, const VarLuaVecTable<glm::vec2>& pos) noexcept;

        static SkeletalAnimator& addEntityComponent(LuaEntity& entity, const std::shared_ptr<Skeleton>& skel, const AnimationMap& anims, const Config& config) noexcept;
        static OptionalRef<SkeletalAnimator>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        static std::optional<LuaEntity> getEntity(const SkeletalAnimator& animator, std::shared_ptr<Scene>& scene) noexcept;
        static bool play1(SkeletalAnimator& animator, const std::string& name) noexcept;
        static bool play2(SkeletalAnimator& animator, const std::string& name, float stateSpeed) noexcept;

        static OptionalRef<const ISkeletalAnimatorState>::std_t getCurrentState(const SkeletalAnimator& animator) noexcept;
        static std::string getCurrentStateName(const SkeletalAnimator& animator) noexcept;
        static OptionalRef<const ISkeletalAnimatorTransition>::std_t getCurrentTransition(const SkeletalAnimator& animator) noexcept;
    };

    class RenderableSkeleton;
    class Material;
    class IMesh;

    class LuaRenderableSkeleton final
	{
    public:
        static RenderableSkeleton& addEntityComponent1(LuaEntity& entity) noexcept;
        static RenderableSkeleton& addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& mat) noexcept;
		static RenderableSkeleton& addEntityComponent3(LuaEntity& entity, const std::shared_ptr<Material>& mat, const std::shared_ptr<IMesh>& boneMesh) noexcept;
		static OptionalRef<RenderableSkeleton>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const RenderableSkeleton& skel, std::shared_ptr<Scene>& scene) noexcept;
    
        static void bind(sol::state_view& lua) noexcept;
    private:
    };

    class SkeletalAnimationSceneComponent;

    class LuaSkeletalAnimationSceneComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static SkeletalAnimationSceneComponent& addSceneComponent(Scene& scene) noexcept;
        static OptionalRef<SkeletalAnimationSceneComponent>::std_t getSceneComponent(Scene& scene) noexcept;
    };

    class SkeletalAnimationRenderComponent;
    class Camera;

    class LuaSkeletalAnimationRenderComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static SkeletalAnimationRenderComponent& addCameraComponent(Camera& cam) noexcept;
        static OptionalRef<SkeletalAnimationRenderComponent>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}