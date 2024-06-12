#pragma once

#include <darmok/optional_ref.hpp>
#include <memory>
#include <sol/sol.hpp>

namespace darmok
{
    class Skeleton;

    class LuaSkeleton final
    {
    public:
        LuaSkeleton(const std::shared_ptr<Skeleton>& skel) noexcept;
        std::string to_string() const noexcept;
        std::shared_ptr<Skeleton> getReal() const noexcept;
        static void bind(sol::state_view& lua) noexcept;
    private:
        std::shared_ptr<Skeleton> _skel;
    };

    class SkeletalAnimation;

    class LuaSkeletalAnimation final
    {
    public:
        LuaSkeletalAnimation(const std::shared_ptr<SkeletalAnimation>& anim) noexcept;
        std::string to_string() const noexcept;
        std::shared_ptr<SkeletalAnimation> getReal() const noexcept;
        static void bind(sol::state_view& lua) noexcept;
    private:
        std::shared_ptr<SkeletalAnimation> _anim;
    };

    struct SkeletalAnimatorConfig;
    class SkeletalAnimator;
    class LuaEntity;
    class LuaScene;

    class LuaSkeletalAnimator final
	{
    public:
        using Config = SkeletalAnimatorConfig;
        LuaSkeletalAnimator(SkeletalAnimator& animator) noexcept;
        
        static LuaSkeletalAnimator addEntityComponent(LuaEntity& entity, const LuaSkeleton& skel, const Config& config) noexcept;
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
}