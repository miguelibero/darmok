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

    class SkeletalAnimationController;
    class LuaEntity;
    class LuaScene;

    class LuaSkeletalAnimationController final
	{
    public:
        LuaSkeletalAnimationController(SkeletalAnimationController& ctrl) noexcept;
        
        static LuaSkeletalAnimationController addEntityComponent1(LuaEntity& entity, const LuaSkeleton& skel) noexcept;
		static LuaSkeletalAnimationController addEntityComponent2(LuaEntity& entity, const LuaSkeleton& skel, const std::vector<LuaSkeletalAnimation>& animations) noexcept;
		static std::optional<LuaSkeletalAnimationController> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;

        LuaSkeletalAnimationController& addAnimation(const LuaSkeletalAnimation& anim) noexcept;
        bool playAnimationOnce(const std::string& name) noexcept;
        bool playAnimation(const std::string& name) noexcept;
        LuaSkeletalAnimationController& setPlaybackSpeed(float speed) noexcept;
        float getPlaybackSpped() const noexcept;
        
        static void bind(sol::state_view& lua) noexcept;


    private:
        OptionalRef<SkeletalAnimationController> _ctrl;
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