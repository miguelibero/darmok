#pragma once

#include <darmok/export.h>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/asset_core.hpp>
#include <darmok/glm.hpp>
#include <darmok/easing.hpp>
#include <darmok/protobuf/skeleton.pb.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <bx/bx.h>
#include <nlohmann/json.hpp>


#ifndef DARMOK_SKELETON_MAX_BONES
#define DARMOK_SKELETON_MAX_BONES 64
#endif

namespace darmok
{
    class SkeletonImpl;
    class IComponentLoadContext;

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

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletonLoader : public ILoader<Skeleton>{};
    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimationLoader : public ILoader<SkeletalAnimation>{};

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorState
    {
    public:
        using Definition = protobuf::SkeletalAnimatorState;
        virtual ~ISkeletalAnimatorState() = default;
        virtual float getNormalizedTime() const = 0;
        virtual float getDuration() const = 0;
        virtual std::string_view getName() const = 0;
    };

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorTransition
    {
    public:
        using Definition = protobuf::SkeletalAnimatorTransition;
        virtual ~ISkeletalAnimatorTransition() = default;
        virtual float getNormalizedTime() const = 0;
        virtual float getDuration() const = 0;
        virtual const ISkeletalAnimatorState& getCurrentState() const = 0;
        virtual const ISkeletalAnimatorState& getPreviousState() const = 0;
    };

    using SkeletalAnimationMap = std::unordered_map<std::string, std::shared_ptr<SkeletalAnimation>>;

    class DARMOK_EXPORT BX_NO_VTABLE ISkeletalAnimatorDefinitionLoader : public ILoader<protobuf::SkeletalAnimator>{};

    class IDataLoader;

    class DataSkeletalAnimatorDefinitionLoader final : public ISkeletalAnimatorDefinitionLoader
    {
    public:
        DataSkeletalAnimatorDefinitionLoader(IDataLoader& dataLoader) noexcept;

        Result operator()(std::filesystem::path path) noexcept override;
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

    enum class SkeletalAnimatorPlaybackState
    {
        Stopped,
        Playing,
        Paused
    };

    class DARMOK_EXPORT ConstSkeletalAnimatorTweenDefinitionWrapper
    {
    public:
        using Definition = protobuf::SkeletalAnimatorTween;

        ConstSkeletalAnimatorTweenDefinitionWrapper(const Definition& def) noexcept;

        [[nodiscard]] float calcTween(float position) const noexcept;

    private:
        OptionalRef<const Definition> _def;
    };


    class DARMOK_EXPORT SkeletalAnimatorTweenDefinitionWrapper final : public ConstSkeletalAnimatorTweenDefinitionWrapper
    {
    public:
        SkeletalAnimatorTweenDefinitionWrapper(Definition& def) noexcept;
        expected<void, std::string> read(const nlohmann::json& json) noexcept;
    private:
        OptionalRef<Definition> _def;
    };

    class DARMOK_EXPORT ConstSkeletalAnimatorStateDefinitionWrapper
    {
    public:
        using Definition = protobuf::SkeletalAnimatorState;

        ConstSkeletalAnimatorStateDefinitionWrapper(const Definition& def) noexcept;

        [[nodiscard]] float calcBlendWeight(const glm::vec2& pos, const glm::vec2& animPos) const noexcept;
        [[nodiscard]] std::vector<float> calcBlendWeights(const glm::vec2& pos) const noexcept;

    private:
        OptionalRef<const Definition> _def;
    };

    class DARMOK_EXPORT SkeletalAnimatorStateDefinitionWrapper final : public ConstSkeletalAnimatorStateDefinitionWrapper
    {
    public:
        SkeletalAnimatorStateDefinitionWrapper(Definition& def) noexcept;
        expected<void, std::string> read(const nlohmann::json& json) noexcept;
    private:
        OptionalRef<Definition> _def;

        static std::optional<Definition::BlendType> getBlendType(const std::string& name) noexcept;
    };

    class DARMOK_EXPORT ConstSkeletalAnimatorTransitionDefinitionWrapper
    {
    public:
        using Definition = protobuf::SkeletalAnimatorTransition;

        ConstSkeletalAnimatorTransitionDefinitionWrapper(const Definition& def) noexcept;
    private:
        OptionalRef<const Definition> _def;
    };

    class DARMOK_EXPORT SkeletalAnimatorTransitionDefinitionWrapper final : public ConstSkeletalAnimatorTransitionDefinitionWrapper
    {
    public:
        SkeletalAnimatorTransitionDefinitionWrapper(Definition& def) noexcept;
        expected<void, std::string> read(const nlohmann::json& json) noexcept;
    private:
        OptionalRef<Definition> _def;
    };

    class DARMOK_EXPORT ConstSkeletalAnimatorAnimationDefinitionWrapper
    {
    public:
        using Definition = protobuf::SkeletalAnimatorAnimation;

        ConstSkeletalAnimatorAnimationDefinitionWrapper(const Definition& def) noexcept;
    private:
        OptionalRef<const Definition> _def;
    };

    class DARMOK_EXPORT SkeletalAnimatorAnimationDefinitionWrapper final : public ConstSkeletalAnimatorAnimationDefinitionWrapper
    {
    public:
        SkeletalAnimatorAnimationDefinitionWrapper(Definition& def) noexcept;
        expected<void, std::string> read(const nlohmann::json& json) noexcept;
    private:
        OptionalRef<Definition> _def;
    };

    class DARMOK_EXPORT ConstSkeletalAnimatorDefinitionWrapper
    {
    public:
        using TweenDefinition = protobuf::SkeletalAnimatorTween;
        using AnimationDefinition = protobuf::SkeletalAnimatorAnimation;
        using StateDefinition = protobuf::SkeletalAnimatorState;
        using TransitionDefinition = protobuf::SkeletalAnimatorTransition;
        using BlendType = protobuf::SkeletalAnimatorState::BlendType;
        using Definition = protobuf::SkeletalAnimator;
        using AnimationMap = SkeletalAnimationMap;

        ConstSkeletalAnimatorDefinitionWrapper(const Definition& def) noexcept;

        [[nodiscard]] AnimationMap loadAnimations(ISkeletalAnimationLoader& loader) const noexcept;
        [[nodiscard]] OptionalRef<const StateDefinition> getState(std::string_view name) const noexcept;
        [[nodiscard]] OptionalRef<const TransitionDefinition> getTransition(std::string_view src, std::string_view dst) const noexcept;

    private:
        OptionalRef<const Definition> _def;
    };

	class DARMOK_EXPORT SkeletalAnimatorDefinitionWrapper final : public ConstSkeletalAnimatorDefinitionWrapper
    {
    public:
        SkeletalAnimatorDefinitionWrapper(Definition& def) noexcept;

        [[nodiscard]] expected<void, std::string> read(const nlohmann::json& json) noexcept;
    private:
        OptionalRef<Definition> _def;

        std::optional<StateDefinition::BlendType> getBlendType(const std::string& name) noexcept;
        static std::pair<std::string_view, std::string_view> parseTransitionKey(std::string_view key) noexcept;
    };

    class SkeletalAnimatorImpl;

    class DARMOK_EXPORT SkeletalAnimator final
    {
    public:
        using Definition = protobuf::SkeletalAnimator;
        using StateDefinition = protobuf::SkeletalAnimatorState;
        using AnimationMap = SkeletalAnimationMap;
        using PlaybackState = SkeletalAnimatorPlaybackState;
        SkeletalAnimator() noexcept;
        SkeletalAnimator(std::shared_ptr<Skeleton> skel, AnimationMap anims, Definition def) noexcept;
        ~SkeletalAnimator();

        SkeletalAnimator& addListener(std::unique_ptr<ISkeletalAnimatorListener>&& listener) noexcept;
        SkeletalAnimator& addListener(ISkeletalAnimatorListener& listener) noexcept;
        bool removeListener(const ISkeletalAnimatorListener& listener) noexcept;
        size_t removeListeners(const ISkeletalAnimatorListenerFilter& filter) noexcept;

        SkeletalAnimator& setPlaybackSpeed(float speed) noexcept;
        float getPlaybackSpeed() const noexcept;

        SkeletalAnimator& setBlendPosition(const glm::vec2& value) noexcept;
        const glm::vec2& getBlendPosition() const noexcept;

        const Definition& getDefinition() const noexcept;
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

        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt) noexcept;
		static Definition createDefinition() noexcept;

    private:
        std::unique_ptr<SkeletalAnimatorImpl> _impl;
    };

    class Transform;
    struct Material;
    class IFont;

    class DARMOK_EXPORT RenderableSkeleton final
    {
    public:
        RenderableSkeleton(const std::shared_ptr<Material>& mat = nullptr, const std::shared_ptr<Mesh>& boneMesh = nullptr) noexcept;
        RenderableSkeleton& setFont(const std::shared_ptr<IFont>& font) noexcept;
        void update(Scene& scene, const std::unordered_map<std::string, glm::mat4>& boneMatrixes) noexcept;
    private:
        std::shared_ptr<IFont> _font;
        std::shared_ptr<Material> _material;
        std::shared_ptr<Mesh> _boneMesh;
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

    class SkeletalAnimatorDefinitionFileImporter final : public ProtobufFileImporter<ISkeletalAnimatorDefinitionLoader>
    {
    public:
		SkeletalAnimatorDefinitionFileImporter() noexcept;
    private:
		FileDataLoader _dataLoader;
        DataSkeletalAnimatorDefinitionLoader _loader;
    };

    struct DARMOK_EXPORT ArmatureJoint final
    {
        std::string name;
        glm::mat4 inverseBindPose;
    };

    class DARMOK_EXPORT Armature final
    {
    public:
        using Definition = protobuf::Armature;

        Armature(const Definition& def) noexcept;
        Armature(const std::vector<ArmatureJoint>& joints) noexcept;
        Armature(std::vector<ArmatureJoint>&& joints) noexcept;
        const std::vector<ArmatureJoint>& getJoints() const noexcept;

        static Definition createDefinition() noexcept;

    private:
        std::vector<ArmatureJoint> _joints;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IArmatureDefinitionLoader : public ILoader<Armature::Definition>{};
    class DARMOK_EXPORT BX_NO_VTABLE IArmatureLoader : public ILoader<Armature>{};
    class DARMOK_EXPORT BX_NO_VTABLE IArmatureFromDefinitionLoader : public IFromDefinitionLoader<IArmatureLoader, Armature::Definition>{};
    using ArmatureLoader = FromDefinitionLoader<IArmatureFromDefinitionLoader, IArmatureDefinitionLoader>;

    using DataArmatureDefinitionLoader = DataProtobufLoader<IArmatureDefinitionLoader>;

    class DARMOK_EXPORT Skinnable
    {
    public:
        Skinnable(const std::shared_ptr<Armature>& armature = nullptr) noexcept;
        std::shared_ptr<Armature> getArmature() const noexcept;
        void setArmature(const std::shared_ptr<Armature>& armature) noexcept;

        using Definition = protobuf::Skinnable;
        expected<void, std::string> load(const Definition& def, IComponentLoadContext& ctxt);

        static Definition createDefinition() noexcept;

    private:
        std::shared_ptr<Armature> _armature;
    };
}