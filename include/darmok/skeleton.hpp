#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>
#include <darmok/render.hpp>
#include <bx/bx.h>
#include <glm/glm.hpp>

#ifndef DARMOK_SKELETON_MAX_BONES
#define DARMOK_SKELETON_MAX_BONES 64
#endif

namespace darmok
{
    class SkeletonImpl;

    class Skeleton final
    {
    public:
        Skeleton(std::unique_ptr<SkeletonImpl>&& impl) noexcept;
        ~Skeleton();
        std::string to_string() const noexcept;
        SkeletonImpl& getImpl();
        const SkeletonImpl& getImpl() const;
    private:
        std::unique_ptr<SkeletonImpl> _impl;
    };

    class SkeletalAnimationImpl;

    class SkeletalAnimation final
    {
    public:
        SkeletalAnimation(std::unique_ptr<SkeletalAnimationImpl>&& impl) noexcept;
        ~SkeletalAnimation();
        SkeletalAnimationImpl& getImpl();
        const SkeletalAnimationImpl& getImpl() const;
        std::string to_string() const noexcept;
        std::string_view getName() const noexcept;
        float getDuration() const noexcept;
    private:
        std::unique_ptr<SkeletalAnimationImpl> _impl;
    };

    class BX_NO_VTABLE ISkeletonLoader
    {
    public:
        using result_type = std::shared_ptr<Skeleton>;

        DLLEXPORT virtual ~ISkeletonLoader() = default;
        DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
    };

    class BX_NO_VTABLE ISkeletalAnimationLoader
    {
    public:
        using result_type = std::shared_ptr<SkeletalAnimation>;
        DLLEXPORT virtual ~ISkeletalAnimationLoader() = default;
        DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
    };

    class EmptySkeletonLoader : public ISkeletonLoader
    {
    public:
        DLLEXPORT std::shared_ptr<Skeleton> operator()(std::string_view name) override;
    };

    class EmptySkeletalAnimationLoader : public ISkeletalAnimationLoader
    {
    public:
        DLLEXPORT std::shared_ptr<SkeletalAnimation> operator()(std::string_view name) override;
    };

    template<typename T>
    struct SkeletalBlendElement final
    {
        std::shared_ptr<SkeletalAnimation> motion;
        T threshold;
        float speed;
    };
    using SkeletalBlendElement1D = SkeletalBlendElement<float>;
    using SkeletalBlendElement2D = SkeletalBlendElement<glm::vec2>;

    struct SkeletalBlendState1D final
    {
        std::string name;
        std::vector<SkeletalBlendElement1D> elements;
    };

    struct SkeletalBlendState2D final
    {
        std::string name;
        std::vector<SkeletalBlendElement2D> elements;
    };

    class SkeletalAnimatorImpl;
    class Transform;
    class Material;

    struct SkeletalAnimationStateConfig final
    {
        std::shared_ptr<SkeletalAnimation> motion;
        std::string name;
        float speed = 1.F;
    };

    struct SkeletalAnimationTransitionConfig final
    {
        float exitTime = 1.F;
        float duration = 0.F;
        float offset = 0.F;
    };

    class BX_NO_VTABLE ISkeletalAnimatorState
    {
    public:
        using Config = SkeletalAnimationStateConfig;
        virtual ~ISkeletalAnimatorState() = default;
        virtual float getNormalizedTime() const = 0;
        virtual void setNormalizedTime(float normalizedTime) = 0;
        virtual float getDuration() const = 0;
        virtual std::string_view getName() const = 0;
    };

    class BX_NO_VTABLE ISkeletalAnimatorTransition
    {
    public:
        using Config = SkeletalAnimationTransitionConfig;
        virtual ~ISkeletalAnimatorTransition() = default;
        virtual float getNormalizedTime() const = 0;
        virtual void setNormalizedTime(float normalizedTime) = 0;
        virtual float getDuration() const = 0;
        virtual ISkeletalAnimatorState& getCurrentState() = 0;
        virtual ISkeletalAnimatorState& getPreviousState() = 0;
    };

    struct SkeletalAnimatorTransitionKey final
    {
        std::string src;
        std::string dst;
    };

    struct SkeletalAnimatorConfig final
    {
        using StateConfig = SkeletalAnimationStateConfig;

        using TransitionKey = std::pair<std::string, std::string>;

        struct TransitionKeyHash
        {
            std::size_t operator()(const TransitionKey& key) const noexcept
            {
                auto h1 = std::hash<std::string>{}(key.first);
                auto h2 = std::hash<std::string>{}(key.second);
                return h1 ^ (h2 << 1);
            }
        };

        using TransitionConfig = SkeletalAnimationTransitionConfig;

        SkeletalAnimatorConfig& addState(const StateConfig& config) noexcept;
        SkeletalAnimatorConfig& addState(const std::shared_ptr<SkeletalAnimation>& animation, std::string_view name = "") noexcept;
        SkeletalAnimatorConfig& addTransition(std::string_view src, std::string_view dst, const TransitionConfig& config) noexcept;

        std::optional<const StateConfig> getState(std::string_view name) const noexcept;
        std::optional<const TransitionConfig> getTransition(std::string_view src, std::string_view dst) const noexcept;

    private:
        std::unordered_map<std::string, StateConfig> _states;
        std::unordered_map<TransitionKey, TransitionConfig, TransitionKeyHash> _transitions;
    };

    class SkeletalAnimator;

    class BX_NO_VTABLE ISkeletalAnimatorListener
    {
    public:
        virtual ~ISkeletalAnimatorListener() = default;
        virtual void onAnimatorStateFinished(SkeletalAnimator& animator, ISkeletalAnimatorState& state) {};
        virtual void onAnimatorStateLooped(SkeletalAnimator& animator, ISkeletalAnimatorState& state) {};
        virtual void onAnimatorStateStarted(SkeletalAnimator& animator, ISkeletalAnimatorState& state) {};
        virtual void onAnimatorTransitionFinished(SkeletalAnimator& animator, ISkeletalAnimatorTransition& trans) {};
        virtual void onAnimatorTransitionStarted(SkeletalAnimator& animator, ISkeletalAnimatorTransition& trans) {};
    };

    class SkeletalAnimator final
    {
    public:
        using Config = SkeletalAnimatorConfig;
        DLLEXPORT SkeletalAnimator(const std::shared_ptr<Skeleton>& skel, const Config&) noexcept;
        DLLEXPORT ~SkeletalAnimator();

        DLLEXPORT SkeletalAnimator& addListener(ISkeletalAnimatorListener& listener) noexcept;
        DLLEXPORT bool removeListener(ISkeletalAnimatorListener& listener) noexcept;

        DLLEXPORT SkeletalAnimator& setPlaybackSpeed(float speed) noexcept;
        DLLEXPORT float getPlaybackSpeed() const noexcept;

        DLLEXPORT const Config& getConfig() const noexcept;
        DLLEXPORT OptionalRef<ISkeletalAnimatorState> getCurrentState() noexcept;
        DLLEXPORT OptionalRef<ISkeletalAnimatorTransition> getCurrentTransition() noexcept;

        DLLEXPORT bool play(std::string_view name) noexcept;
        
        DLLEXPORT glm::mat4 getModelMatrix(const std::string& joint) const noexcept;
        DLLEXPORT std::vector<glm::mat4> getBoneMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;

        void update(float deltaTime);
    private:
        std::unique_ptr<SkeletalAnimatorImpl> _impl;
    };

    class RenderableSkeleton final
    {
    public:
        DLLEXPORT RenderableSkeleton(const std::shared_ptr<Material>& mat, const std::shared_ptr<IMesh>& boneMesh = nullptr) noexcept;
        DLLEXPORT ~RenderableSkeleton() noexcept;

        void init(Scene& scene) noexcept;
        void update(float deltaTime) noexcept;
        void shutdown() noexcept;
    private:
        OptionalRef<Scene> _scene;
        std::shared_ptr<Material> _material;
        std::shared_ptr<IMesh> _boneMesh;
        OptionalRef<SkeletalAnimator> _animator;
        std::vector<OptionalRef<Transform>> _bones;

        void fixBoneMesh() noexcept;
        void destroyBones() noexcept;
        void createBones() noexcept;
    };

    class SkeletalAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        void init(Scene& scene, App& app) noexcept override;
        void update(float deltaTime) noexcept override;
        void shutdown() noexcept override;
    private:
        OptionalRef<Scene> _scene;

        void onSkeletonConstructed(EntityRegistry& registry, Entity entity) noexcept;
    };


    struct ArmatureBone final
    {
        std::string joint;
        glm::mat4 inverseBindPose;
    };

    class Armature final
    {
    public:
        DLLEXPORT Armature(const std::vector<ArmatureBone>& bones) noexcept;
        DLLEXPORT Armature(std::vector<ArmatureBone>&& bones) noexcept;
        DLLEXPORT const std::vector<ArmatureBone>& getBones() const noexcept;

    private:
        std::vector<ArmatureBone> _bones;
    };

    class Skinnable
    {
    public:
        DLLEXPORT Skinnable(const std::shared_ptr<Armature>& armature = nullptr) noexcept;
        const std::shared_ptr<Armature>& getArmature() const noexcept;
        void setArmature(const std::shared_ptr<Armature>& armature) noexcept;
    private:
        std::shared_ptr<Armature> _armature;
    };

    class SkeletalAnimationCameraComponent final : public ICameraComponent
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) noexcept override;
        void shutdown() noexcept override;
    private:
        bgfx::UniformHandle _skinningUniform;
        std::vector<glm::mat4> _skinning;
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        OptionalRef<SkeletalAnimator> getAnimator(Entity entity) const noexcept;
    };
}