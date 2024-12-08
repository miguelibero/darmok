#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/glm.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/easing.hpp>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <bx/bx.h>
#include <nlohmann/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

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
        std::string toString() const noexcept;
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
        std::string toString() const noexcept;
        std::string_view getName() const noexcept;
        float getDuration() const noexcept;
    private:
        std::unique_ptr<SkeletalAnimationImpl> _impl;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletonLoader : public ILoader<Skeleton>
    {
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimationLoader : public ILoader<SkeletalAnimation>
    {
    };

    struct DARMOK_EXPORT SkeletalAnimatorAnimationConfig final
    {
        std::string animation;
        glm::vec2 blendPosition = {};
        float speed = 1.F;
        bool loop = true;

        void readJson(const nlohmann::json& json);

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(animation),
                CEREAL_NVP(blendPosition),
                CEREAL_NVP(speed)
            );
        }
    };

    enum class SkeletalAnimatorBlendType
    {
        Cartesian,
        Directional
    };

    struct DARMOK_EXPORT SkeletalAnimatorTweenConfig final
    {
        EasingType easing = EasingType::Linear;
        float duration = 0.F;

        float operator()(float position) const noexcept;
        void readJson(const nlohmann::json& json);

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(easing),
                CEREAL_NVP(duration)
            );
        }
    };

    struct DARMOK_EXPORT SkeletalAnimatorStateConfig final
    {
        using AnimationConfig = SkeletalAnimatorAnimationConfig;
        std::string name;
        std::vector<AnimationConfig> animations;
        SkeletalAnimatorBlendType blendType = SkeletalAnimatorBlendType::Directional;
        float threshold = bx::kFloatSmallest;
        SkeletalAnimatorTweenConfig tween;
        std::string nextState;
        float speed = 1.F;

        float calcBlendWeight(const glm::vec2& pos, const glm::vec2& animPos);
        std::vector<float> calcBlendWeights(const glm::vec2& pos);

        void readJson(const nlohmann::json& json);
        static SkeletalAnimatorBlendType getBlendType(const std::string_view name) noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(name),
                CEREAL_NVP(animations),
                CEREAL_NVP(blendType),
                CEREAL_NVP(threshold),
                CEREAL_NVP(tween)
            );
        }
    };

    struct DARMOK_EXPORT SkeletalAnimatorTransitionConfig final
    {
        SkeletalAnimatorTweenConfig tween;
        float offset = 0.F;

        static std::pair<std::string, std::string> readJsonKey(std::string_view key);
        void readJson(const nlohmann::json& json);

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(tween), CEREAL_NVP(offset));
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorState
    {
    public:
        using Config = SkeletalAnimatorStateConfig;
        virtual ~ISkeletalAnimatorState() = default;
        virtual float getNormalizedTime() const = 0;
        virtual float getDuration() const = 0;
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

    using SkeletalAnimationMap = std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>>;

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

        void readJson(const nlohmann::json& json);

        using AnimationMap = SkeletalAnimationMap;
        AnimationMap loadAnimations(ISkeletalAnimationLoader& loader) const;

        SkeletalAnimatorConfig& addState(const StateConfig& config) noexcept;
        SkeletalAnimatorConfig& addState(std::string_view animation, std::string_view name = "") noexcept;
        SkeletalAnimatorConfig& addTransition(std::string_view src, std::string_view dst, const TransitionConfig& config) noexcept;

        std::optional<const StateConfig> getState(std::string_view name) const noexcept;
        std::optional<const TransitionConfig> getTransition(std::string_view src, std::string_view dst) const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("states", _states),
                CEREAL_NVP_("transitions", _transitions),
                CEREAL_NVP_("animationPattern", _animationPattern)
            );
        }

    private:
        std::string _animationPattern;
        std::unordered_map<std::string, StateConfig> _states;
        std::unordered_map<TransitionKey, TransitionConfig, TransitionKeyHash> _transitions;

        std::string getAnimationName(std::string_view key) const noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorConfigLoader : public ILoader<SkeletalAnimatorConfig>
    {
    };

    class IDataLoader;

    class DARMOK_EXPORT SkeletalAnimatorConfigLoader final : public ISkeletalAnimatorConfigLoader
    {
    public:
        SkeletalAnimatorConfigLoader(IDataLoader& dataLoader) noexcept;
        std::shared_ptr<SkeletalAnimatorConfig> operator()(const std::filesystem::path& path) override;
    private:
        IDataLoader& _dataLoader;
    };

    class SkeletalAnimator;

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorListener
    {
    public:
        virtual ~ISkeletalAnimatorListener() = default;
        virtual entt::id_type getSkeletalAnimatorListenerType() const noexcept { return 0; };
        virtual void onAnimatorDestroyed(SkeletalAnimator& animator) {};
        virtual void onAnimatorStateLooped(SkeletalAnimator& animator, std::string_view state) {};
        virtual void onAnimatorStateFinished(SkeletalAnimator& animator, std::string_view state) {};
        virtual void onAnimatorStateStarted(SkeletalAnimator& animator, std::string_view state) {};
        virtual void onAnimatorTransitionFinished(SkeletalAnimator& animator) {};
        virtual void onAnimatorTransitionStarted(SkeletalAnimator& animator) {};
    };

    template<typename T>
    class DARMOK_EXPORT BX_NO_VTABLE ITypeSkeletalAnimatorListener : public ISkeletalAnimatorListener
    {
    public:
        entt::id_type getSkeletalAnimatorListenerType() const noexcept override
        {
            return entt::type_hash<T>::value();
        }
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorListenerFilter
    {
    public:
        virtual ~ISkeletalAnimatorListenerFilter() = default;
        virtual bool operator()(const ISkeletalAnimatorListener& listener) const = 0;
    };

    class SkeletalAnimatorImpl;

    enum class SkeletalAnimatorPlaybackState
    {
        Stopped,
        Playing,
        Paused
    };

    class DARMOK_EXPORT SkeletalAnimator final
    {
    public:
        using Config = SkeletalAnimatorConfig;
        using AnimationMap = SkeletalAnimationMap;
        using PlaybackState = SkeletalAnimatorPlaybackState;
        SkeletalAnimator(const std::shared_ptr<Skeleton>& skel, const AnimationMap& anims, const Config& config) noexcept;
        ~SkeletalAnimator();

        SkeletalAnimator& addListener(std::unique_ptr<ISkeletalAnimatorListener>&& listener) noexcept;
        SkeletalAnimator& addListener(ISkeletalAnimatorListener& listener) noexcept;
        bool removeListener(const ISkeletalAnimatorListener& listener) noexcept;
        size_t removeListeners(const ISkeletalAnimatorListenerFilter& filter) noexcept;

        SkeletalAnimator& setPlaybackSpeed(float speed) noexcept;
        float getPlaybackSpeed() const noexcept;

        SkeletalAnimator& setBlendPosition(const glm::vec2& value) noexcept;
        const glm::vec2& getBlendPosition() const noexcept;

        const Config& getConfig() const noexcept;
        OptionalRef<const ISkeletalAnimatorState> getCurrentState() const noexcept;
        OptionalRef<const ISkeletalAnimatorTransition> getCurrentTransition() const noexcept;
        float getStateDuration(const std::string& name) const noexcept;

        bool play(std::string_view name, float speedFactor = 1.F) noexcept;
        void stop() noexcept;
        void pause() noexcept;
        PlaybackState getPlaybackState() noexcept;
        
        glm::mat4 getJointModelMatrix(const std::string& node) const noexcept;
        std::unordered_map<std::string, glm::mat4> getBoneModelMatrixes(const glm::vec3& dir = {1, 0, 0}) const noexcept;

        void update(float deltaTime);
    private:
        std::unique_ptr<SkeletalAnimatorImpl> _impl;
    };

    class Transform;
    class Material;
    class IFont;

    class DARMOK_EXPORT RenderableSkeleton final
    {
    public:
        RenderableSkeleton(const std::shared_ptr<Material>& mat = nullptr, const std::shared_ptr<IMesh>& boneMesh = nullptr) noexcept;
        RenderableSkeleton& setFont(const std::shared_ptr<IFont>& font) noexcept;
        void update(Scene& scene, const std::unordered_map<std::string, glm::mat4>& boneMatrixes) noexcept;
    private:
        std::shared_ptr<IFont> _font;
        std::shared_ptr<Material> _material;
        std::shared_ptr<IMesh> _boneMesh;
        std::unordered_map<std::string, OptionalRef<Transform>> _boneTransforms;
    };

    class DARMOK_EXPORT SkeletalAnimationSceneComponent final : public ITypeSceneComponent<SkeletalAnimationSceneComponent>
    {
    public:
        void init(Scene& scene, App& app) noexcept override;
        void update(float deltaTime) noexcept override;
    private:
        OptionalRef<Scene> _scene;
    };

    class DARMOK_EXPORT SkeletalAnimationRenderComponent final : public ITypeCameraComponent<SkeletalAnimationRenderComponent>
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
        void shutdown() noexcept override;
    private:
        bgfx::UniformHandle _skinningUniform;
        std::vector<glm::mat4> _skinning;
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        OptionalRef<SkeletalAnimator> getAnimator(Entity entity) const noexcept;
    };

    class DARMOK_EXPORT SkeletalAnimatorConfigImporter final : public IAssetTypeImporter
    {
    public:
        SkeletalAnimatorConfig read(const std::filesystem::path& path) const;
        std::vector<std::filesystem::path> getOutputs(const Input& input) noexcept override;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath) override;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out) override;
        const std::string& getName() const noexcept override;
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
}

namespace std
{
    template<class Archive>
    static void serialize(Archive& archive, darmok::SkeletalAnimatorConfig::TransitionKey& key)
    {
        archive(cereal::make_map_item(key.first, key.second));
    }
}