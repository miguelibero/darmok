#pragma once

#include <darmok/optional_ref.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <sol/sol.hpp>

namespace darmok
{
    class Skeleton;
    class SkeletalAnimation;
    struct SkeletalAnimatorConfig;
    class SkeletalAnimator;

    class LuaEntity;
    class LuaScene;

    struct LuaSkeleton final
    {
        static void bind(sol::state_view& lua) noexcept;
    };

    class LuaSkeletalAnimator final
	{
    public:
        using Config = SkeletalAnimatorConfig;
        using AnimationMap = std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>>;
        
        LuaSkeletalAnimator(SkeletalAnimator& animator) noexcept;

        static LuaSkeletalAnimator addEntityComponent(LuaEntity& entity, const std::shared_ptr<Skeleton>& skel, const AnimationMap& anims, const Config& config) noexcept;
		static std::optional<LuaSkeletalAnimator> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;

        bool play(const std::string& name) noexcept;
        LuaSkeletalAnimator& setPlaybackSpeed(float speed) noexcept;
        float getPlaybackSpeed() const noexcept;
        
        static void bind(sol::state_view& lua) noexcept;


    private:
        OptionalRef<SkeletalAnimator> _animator;
    };

    class RenderableSkeleton;
    class LuaMaterial;
    class LuaMesh;

    class LuaRenderableSkeleton final
	{
    public:
        LuaRenderableSkeleton(RenderableSkeleton& skel) noexcept;

        static LuaRenderableSkeleton addEntityComponent1(LuaEntity& entity, const LuaMaterial& mat) noexcept;
		static LuaRenderableSkeleton addEntityComponent2(LuaEntity& entity, const LuaMaterial& mat, const LuaMesh& boneMesh) noexcept;
		static std::optional<LuaRenderableSkeleton> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
    
        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<RenderableSkeleton> _skel;
    };

    class SkeletalAnimationSceneComponent;

    class LuaSkeletalAnimationSceneComponent final
    {
    public:
        LuaSkeletalAnimationSceneComponent(SkeletalAnimationSceneComponent& comp) noexcept;
        static LuaSkeletalAnimationSceneComponent addSceneComponent(LuaScene& scene) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<SkeletalAnimationSceneComponent> _comp;
    };

    class SkeletalAnimationCameraComponent;
    class LuaCamera;

    class LuaSkeletalAnimationCameraComponent final
    {
    public:
        LuaSkeletalAnimationCameraComponent(SkeletalAnimationCameraComponent& comp) noexcept;
        static LuaSkeletalAnimationCameraComponent addCameraComponent(LuaCamera& cam) noexcept;

        static void bind(sol::state_view& lua) noexcept;
    private:
        OptionalRef<SkeletalAnimationCameraComponent> _comp;
    };
}