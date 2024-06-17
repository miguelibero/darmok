#pragma once

#include <darmok/export.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/camera.hpp>
#include <darmok/render.hpp>
#include <bx/bx.h>
#include <darmok/glm.hpp>
#include <nlohmann/json.hpp>
#include <tweeny/tweeny.h>

#ifndef DARMOK_SKELETON_MAX_BONES
#define DARMOK_SKELETON_MAX_BONES 64
#endif

namespace darmok
{
    class SkeletonImpl;

    class DARMOK_EXPORT Skeleton final
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

    class DARMOK_EXPORT SkeletalAnimation final
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

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletonLoader
    {
    public:
        using result_type = std::shared_ptr<Skeleton>;

        virtual ~ISkeletonLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimationLoader
    {
    public:
        using result_type = std::shared_ptr<SkeletalAnimation>;
        virtual ~ISkeletalAnimationLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    struct DARMOK_EXPORT SkeletalAnimatorAnimationConfig final
    {
        std::shared_ptr<SkeletalAnimation> animation = nullptr;
        glm::vec2 blendPosition = {};
        float speed = 1;

        void readJson(const nlohmann::json& json, ISkeletalAnimationLoader& loader);
    };

    enum class SkeletalAnimatorBlendType
    {
        Cartesian,
        Directional
    };

    struct DARMOK_EXPORT SkeletalAnimatorTweenConfig final
    {
        tweeny::easing::enumerated easing = tweeny::easing::enumerated::linear;
        float duration = 0.F;

        tweeny::tween<float> create() const noexcept;
        void readJson(const nlohmann::json& json);
    };

    struct DARMOK_EXPORT SkeletalAnimatorStateConfig final
    {
        using AnimationConfig = SkeletalAnimatorAnimationConfig;
        std::string name;
        std::vector<AnimationConfig> animations;
        SkeletalAnimatorBlendType blendType = SkeletalAnimatorBlendType::Directional;
        float threshold = bx::kFloatSmallest;
        SkeletalAnimatorTweenConfig tween;

        float calcBlendWeight(const glm::vec2& pos, const glm::vec2& animPos);
        std::vector<float> calcBlendWeights(const glm::vec2& pos);

        void readJson(const nlohmann::json& json, ISkeletalAnimationLoader& loader);
        static SkeletalAnimatorBlendType getBlendType(const std::string_view name) noexcept;
    };

    struct DARMOK_EXPORT SkeletalAnimatorTransitionConfig final
    {
        SkeletalAnimatorTweenConfig tween;
        float offset = 0.F;

        static std::pair<std::string, std::string> readJsonKey(std::string_view key);
        void readJson(const nlohmann::json& json);
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorState
    {
    public:
        using Config = SkeletalAnimatorStateConfig;
        virtual ~ISkeletalAnimatorState() = default;
        virtual std::string_view getName() const = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorTransition
    {
    public:
        using Config = SkeletalAnimatorTransitionConfig;
        virtual ~ISkeletalAnimatorTransition() = default;
        virtual float getNormalizedTime() const = 0;
        virtual float getDuration() const = 0;
        virtual const ISkeletalAnimatorState& getCurrentState() const = 0;
        virtual const ISkeletalAnimatorState& getPreviousState() const = 0;
    };

    struct DARMOK_EXPORT SkeletalAnimatorConfig final
    {
        using StateConfig = SkeletalAnimatorStateConfig;

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

        using TransitionConfig = SkeletalAnimatorTransitionConfig;

        void readJson(const nlohmann::json& json, ISkeletalAnimationLoader& loader);

        SkeletalAnimatorConfig& addState(const StateConfig& config) noexcept;
        SkeletalAnimatorConfig& addState(const std::shared_ptr<SkeletalAnimation>& animation, std::string_view name = "") noexcept;
        SkeletalAnimatorConfig& addTransition(std::string_view src, std::string_view dst, const TransitionConfig& config) noexcept;

        std::optional<const StateConfig> getState(std::string_view name) const noexcept;
        std::optional<const TransitionConfig> getTransition(std::string_view src, std::string_view dst) const noexcept;

    private:
        std::unordered_map<std::string, StateConfig> _states;
        std::unordered_map<TransitionKey, TransitionConfig, TransitionKeyHash> _transitions;

    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorConfigLoader
    {
    public:
        using result_type = SkeletalAnimatorConfig;
        virtual ~ISkeletalAnimatorConfigLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    class IDataLoader;

    class DARMOK_EXPORT JsonSkeletalAnimatorConfigLoader final : public ISkeletalAnimatorConfigLoader
    {
    public:
        JsonSkeletalAnimatorConfigLoader(IDataLoader& dataLoader, ISkeletalAnimationLoader& animLoader) noexcept;
        SkeletalAnimatorConfig operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
        ISkeletalAnimationLoader& _animLoader;
    };

    class SkeletalAnimator;

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorListener
    {
    public:
        virtual ~ISkeletalAnimatorListener() = default;
        virtual void onAnimatorStateFinished(SkeletalAnimator& animator, std::string_view state) {};
        virtual void onAnimatorStateStarted(SkeletalAnimator& animator, std::string_view state) {};
        virtual void onAnimatorTransitionFinished(SkeletalAnimator& animator) {};
        virtual void onAnimatorTransitionStarted(SkeletalAnimator& animator) {};
    };

    class SkeletalAnimatorImpl;

    class DARMOK_EXPORT SkeletalAnimator final
    {
    public:
        using Config = SkeletalAnimatorConfig;
        SkeletalAnimator(const std::shared_ptr<Skeleton>& skel, const Config&) noexcept;
        ~SkeletalAnimator();

        SkeletalAnimator& addListener(ISkeletalAnimatorListener& listener) noexcept;
        bool removeListener(ISkeletalAnimatorListener& listener) noexcept;

        SkeletalAnimator& setPlaybackSpeed(float speed) noexcept;
        float getPlaybackSpeed() const noexcept;
        SkeletalAnimator& setBlendPosition(const glm::vec2& value) noexcept;

        const Config& getConfig() const noexcept;
        OptionalRef<const ISkeletalAnimatorState> getCurrentState() const noexcept;
        OptionalRef<const ISkeletalAnimatorTransition> getCurrentTransition() const noexcept;

        bool play(std::string_view name) noexcept;
        
        glm::mat4 getJointModelMatrix(const std::string& node) const noexcept;
        std::vector<glm::mat4> getBoneModelMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;

        void update(float deltaTime);
    private:
        std::unique_ptr<SkeletalAnimatorImpl> _impl;
    };

    class Transform;
    class Material;

    class DARMOK_EXPORT RenderableSkeleton final
    {
    public:
        RenderableSkeleton(const std::shared_ptr<Material>& mat, const std::shared_ptr<IMesh>& boneMesh = nullptr) noexcept;
        void update(Scene& scene, const std::vector<glm::mat4>& boneMatrixes) noexcept;
    private:
        std::shared_ptr<Material> _material;
        std::shared_ptr<IMesh> _boneMesh;
        std::vector<OptionalRef<Transform>> _boneTransforms;
    };

    class DARMOK_EXPORT SkeletalAnimationUpdater final : public ISceneLogicUpdater
    {
    public:
        void init(Scene& scene, App& app) noexcept override;
        void update(float deltaTime) noexcept override;
    private:
        OptionalRef<Scene> _scene;
    };

    struct DARMOK_EXPORT ArmatureJoint final
    {
        std::string name;
        glm::mat4 inverseBindPose;
    };

    class DARMOK_EXPORT Armature final
    {
    public:
        Armature(const std::vector<ArmatureJoint>& joints) noexcept;
        Armature(std::vector<ArmatureJoint>&& joints) noexcept;
        const std::vector<ArmatureJoint>& getJoints() const noexcept;

    private:
        std::vector<ArmatureJoint> _joints;
    };

    class DARMOK_EXPORT Skinnable
    {
    public:
        Skinnable(const std::shared_ptr<Armature>& armature = nullptr) noexcept;
        std::shared_ptr<Armature> getArmature() const noexcept;
        void setArmature(const std::shared_ptr<Armature>& armature) noexcept;
    private:
        std::shared_ptr<Armature> _armature;
    };

    class DARMOK_EXPORT SkeletalAnimationComponent final : public ICameraComponent
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