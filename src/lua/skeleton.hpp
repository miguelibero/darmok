#pragma once

#include "lua/lua.hpp"
#include "lua/glm.hpp"

#include <memory>
#include <unordered_map>
#include <string>

#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Skeleton;
    class Skinnable;
    class Armature;
    class SkeletalAnimation;
    class SkeletalAnimator;
    class ISkeletalAnimatorState;
    class ISkeletalAnimatorTransition;

    namespace protobuf
    {
        class SkeletalAnimator;
    }

    class LuaEntity;
    class Scene;

    struct LuaSkeleton final
    {
        static void bind(sol::state_view& lua) noexcept;
    };

    struct LuaSkinnable final
    {
        static void bind(sol::state_view& lua) noexcept;
    private:
        static Skinnable& addEntityComponent(LuaEntity& entity, const std::shared_ptr<Armature>& armature) noexcept;
        static OptionalRef<Skinnable>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        static std::optional<LuaEntity> getEntity(const Skinnable& skinnable, const std::shared_ptr<Scene>& scene) noexcept;
    };

    class LuaSkeletalAnimator final
	{
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:

        using Definition = protobuf::SkeletalAnimator;

        static void setBlendPosition(SkeletalAnimator& animator, const VarLuaVecTable<glm::vec2>& pos) noexcept;

        static SkeletalAnimator& addEntityComponent(LuaEntity& entity, const std::shared_ptr<Skeleton>& skel, const sol::table& anims, const Definition& def) noexcept;
        static OptionalRef<SkeletalAnimator>::std_t getEntityComponent(LuaEntity& entity) noexcept;
        static std::optional<LuaEntity> getEntity(const SkeletalAnimator& animator, std::shared_ptr<Scene>& scene) noexcept;
        static bool play1(SkeletalAnimator& animator, const std::string& name) noexcept;
        static bool play2(SkeletalAnimator& animator, const std::string& name, float stateSpeed) noexcept;

        static OptionalRef<const ISkeletalAnimatorState>::std_t getCurrentState(const SkeletalAnimator& animator) noexcept;
        static std::string getCurrentStateName(const SkeletalAnimator& animator) noexcept;
        static OptionalRef<const ISkeletalAnimatorTransition>::std_t getCurrentTransition(const SkeletalAnimator& animator) noexcept;
    };

    class RenderableSkeleton;
    struct Material;
    class Mesh;

    class LuaRenderableSkeleton final
	{
    public:
        static RenderableSkeleton& addEntityComponent1(LuaEntity& entity) noexcept;
        static RenderableSkeleton& addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& mat) noexcept;
		static RenderableSkeleton& addEntityComponent3(LuaEntity& entity, const std::shared_ptr<Mesh>& boneMesh, const std::shared_ptr<Material>& mat) noexcept;
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
        static std::reference_wrapper<SkeletalAnimationSceneComponent> addSceneComponent(Scene& scene) noexcept;
        static OptionalRef<SkeletalAnimationSceneComponent>::std_t getSceneComponent(Scene& scene) noexcept;
    };

    class SkeletalAnimationRenderComponent;
    class Camera;

    class LuaSkeletalAnimationRenderComponent final
    {
    public:
        static void bind(sol::state_view& lua) noexcept;
    private:
        static std::reference_wrapper<SkeletalAnimationRenderComponent> addCameraComponent(Camera& cam) noexcept;
        static OptionalRef<SkeletalAnimationRenderComponent>::std_t getCameraComponent(Camera& cam) noexcept;
    };
}