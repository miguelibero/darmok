#pragma once

#include <darmok/optional_ref.hpp>
#include <memory>
#include <unordered_map>
#include <string>
#include <sol/sol.hpp>
#include "glm.hpp"

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
        static void bind(sol::state_view& lua) noexcept;
    private:

        using Config = SkeletalAnimatorConfig;
        using AnimationMap = std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>>;

        static void setBlendPosition(SkeletalAnimator& animator, const VarLuaVecTable<glm::vec2>& pos) noexcept;

        static SkeletalAnimator& addEntityComponent(LuaEntity& entity, const std::shared_ptr<Skeleton>& skel, const AnimationMap& anims, const Config& config) noexcept;
        static OptionalRef<SkeletalAnimator>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        static std::optional<LuaEntity> getEntity(const SkeletalAnimator& animator, LuaScene& scene) noexcept;
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
		static std::optional<LuaEntity> getEntity(const RenderableSkeleton& skel, LuaScene& scene) noexcept;
    
        static void bind(sol::state_view& lua) noexcept;
    private:
    };

    class SkeletalAnimationSceneComponent;

    class LuaSkeletalAnimationSceneComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static SkeletalAnimationSceneComponent& addSceneComponent(LuaScene& scene) noexcept;
    };

    class SkeletalAnimationRenderComponent;
    class ForwardRenderer;

    class LuaSkeletalAnimationRenderComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static SkeletalAnimationRenderComponent& addRenderComponent(ForwardRenderer& renderer) noexcept;
    };
}