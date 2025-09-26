#pragma once

#include <darmok/data.hpp>
#include <darmok/expected.hpp>
#include "scene_assimp.hpp"

#include <optional>
#include <vector>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <map>

#include <bx/allocator.h>
#include <bx/file.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/tools/import2ozz.h>
#include <ozz/animation/runtime/skeleton.h>
#include <assimp/matrix4x4.h>

namespace ozz::animation::offline
{
    struct RawAnimation;
    struct RawFloatTrack;
    struct RawFloat2Track;
    struct RawFloat3Track;
    struct RawFloat4Track;
}

namespace ozz::animation
{
    class Skeleton;
    class Animation;
}

struct aiScene;
struct aiNode;
struct aiBone;
struct aiMesh;
struct aiAnimation;
struct aiNodeAnim;
template<typename TReal> class aiMatrix4x4t;
typedef float ai_real;
typedef aiMatrix4x4t<ai_real> aiMatrix4x4;

namespace darmok
{
    class Skeleton;
    class SkeletalAnimation;

    class AssimpOzzSkeletonConverter final
    {
    public:
        using Skeleton = ozz::animation::Skeleton;
        using RawSkeleton = ozz::animation::offline::RawSkeleton;
        AssimpOzzSkeletonConverter(const aiScene& scene) noexcept;
        Skeleton createSkeleton();
        bool update(RawSkeleton& skel) noexcept;
        std::vector<std::string> getSkeletonNames();
        AssimpOzzSkeletonConverter& setBoneNames(const std::vector<std::string>& names) noexcept;
        AssimpOzzSkeletonConverter& setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept;
        AssimpOzzSkeletonConverter& setConfig(const nlohmann::json& config) noexcept;
    private:
        const aiScene& _scene;
        std::unordered_map<std::string, std::string> _boneNames;

        using BoneNodes = std::unordered_map<const aiNode*, std::string>;

        aiBone* findRootBone(const aiMesh& mesh, BoneNodes& boneNodes) noexcept;
        bool update(const aiMesh& mesh, RawSkeleton& skel);
        void update(const aiNode& node, RawSkeleton::Joint& parentJoint, const aiMatrix4x4& parentTrans, const BoneNodes& boneNodes);
        OptionalRef<RawSkeleton::Joint> findJoint(RawSkeleton::Joint::Children& joints, const ozz::string& name, bool recursive) noexcept;
    };

    class AssimpOzzAnimationConverter final
    {
    public:

        struct OptimizationSettings final
        {
            struct Element final
            {
                float tolerance = 0.001;
                float distance = 0.1;
            };
            Element defaultElement;
            std::unordered_map<std::string, Element> elements;
        };

        using Animation = ozz::animation::Animation;
        using Skeleton = ozz::animation::Skeleton;
        using RawAnimation = ozz::animation::offline::RawAnimation;
        AssimpOzzAnimationConverter(const aiScene& scene, OptionalRef<std::ostream> log = nullptr) noexcept;
        AssimpOzzAnimationConverter& setBoneNames(const std::vector<std::string>& boneNames) noexcept;
        AssimpOzzAnimationConverter& setSkeleton(const Skeleton& skel) noexcept;
        AssimpOzzAnimationConverter& setOptimization(const OptimizationSettings& settings) noexcept;
        AssimpOzzAnimationConverter& setMinKeyframeDuration(float v) noexcept;

        Animation createAnimation(const std::string& name);
        Animation createAnimation();
        bool update(const std::string& name, RawAnimation& anim);
        std::vector<std::string> getAnimationNames();
    private:
        OptionalRef<std::ostream> _log;
        const aiScene& _scene;
        std::vector<std::string> _boneNames;
        std::optional<OptimizationSettings> _optimization;
        OptionalRef<const Skeleton> _skeleton;
        float _minKeyDuration;

        std::optional<RawAnimation> optimize(const RawAnimation& animation) noexcept;

        void update(const aiAnimation& assimpAnim, RawAnimation& anim);
        void update(const std::string& boneName, float tickPerSecond, const aiNodeAnim& assimpTrack, RawAnimation::JointTrack& track) noexcept;
        bool isBone(const aiNode& node) const noexcept;
        bool logInfo(const aiAnimation& assimpAnim, const std::string& prefix = "") noexcept;
    };


    class AssimpOzzImporter final : ozz::animation::offline::OzzImporter
    {
    public:
        using OzzSkeleton = ozz::animation::Skeleton;
        using RawSkeleton = ozz::animation::offline::RawSkeleton;
        using RawAnimation = ozz::animation::offline::RawAnimation;
        using RawFloatTrack = ozz::animation::offline::RawFloatTrack;
        using RawFloat2Track = ozz::animation::offline::RawFloat2Track;
        using RawFloat3Track = ozz::animation::offline::RawFloat3Track;
        using RawFloat4Track = ozz::animation::offline::RawFloat4Track;

        bool Load(const char* filename) override;
        bool Import(RawSkeleton* skeleton, const NodeType& types) override;
        AnimationNames GetAnimationNames() override;
        bool Import(const char* animationName,
            const OzzSkeleton& skeleton,
            float samplingRate, RawAnimation* animation) override;
        NodeProperties GetNodeProperties(const char* nodeName) override;

        bool Import(const char* animationName, const char* nodeName,
            const char* trackName, NodeProperty::Type trackType,
            float samplingRate, RawFloatTrack* track) override;
        bool Import(const char* animationName, const char* nodeName,
            const char* trackName, NodeProperty::Type trackType,
            float samplingRate, RawFloat2Track* track)  override;
        bool Import(const char* animationName, const char* nodeName,
            const char* trackName, NodeProperty::Type trackType,
            float samplingRate, RawFloat3Track* track)  override;
        bool Import(const char* animationName, const char* nodeName,
            const char* trackName, NodeProperty::Type trackType,
            float samplingRate, RawFloat4Track* track)  override;
    private:
        std::filesystem::path _path;
        std::shared_ptr<aiScene> _assimpScene;
        AssimpLoader _assimpLoader;
    };

    class IDataLoader;

    class AssimpSkeletonLoaderImpl final
    {
    public:
        using Result = expected<std::shared_ptr<Skeleton>, std::string>;
        AssimpSkeletonLoaderImpl(IDataLoader& dataLoader) noexcept;
        Result operator()(const std::filesystem::path& path);
    private:
        IDataLoader& _dataLoader;
        AssimpLoader _assimpLoader;
    };

    class AssimpSkeletalAnimationLoaderImpl final
    {
    public:
		using Result = expected<std::shared_ptr<SkeletalAnimation>, std::string>;
        AssimpSkeletalAnimationLoaderImpl(IDataLoader& dataLoader) noexcept;
        Result operator()(const std::filesystem::path& path);
    private:
        IDataLoader& _dataLoader;
        AssimpLoader _assimpLoader;
    };

    class AssimpSkeletonFileImporterImpl final
    {
    public:
        using Input = FileImportInput;
        using OzzSkeleton = ozz::animation::Skeleton;
        using Effect = FileImportEffect;
        using Config = FileImportConfig;

        AssimpSkeletonFileImporterImpl(size_t bufferSize = 4096) noexcept;        
        const std::string& getName() const noexcept;

        expected<Effect, std::string> prepare(const Input& input) noexcept;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept;

        expected<OzzSkeleton, std::string> read(const std::filesystem::path& path, const nlohmann::json& config) const noexcept;
    private:
        size_t _bufferSize;
        AssimpLoader _assimpLoader;
    };

    class AssimpSkeletalAnimationFileImporterImpl final
    {
    public:
        using Input = FileImportInput;
        using Effect = FileImportEffect;
        using Config = FileImportConfig;
        using OzzAnimation = ozz::animation::Animation;
        using OzzSkeleton = ozz::animation::Skeleton;
        using OptimizationSettings = AssimpOzzAnimationConverter::OptimizationSettings;

        AssimpSkeletalAnimationFileImporterImpl(size_t bufferSize = 4096) noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        const std::string& getName() const noexcept;

        expected<Effect, std::string> prepare(const Input& input) noexcept;
        expected<void, std::string> operator()(const Input& input, Config& config) noexcept;
    private:
        expected<OzzSkeleton, std::string> loadSkeleton(const std::filesystem::path& path, const nlohmann::json& config);
        OptimizationSettings loadOptimizationSettings(const nlohmann::json& config) noexcept;
        std::filesystem::path getOutputPath(const Input& input, const std::string& animName, const std::string& outputPath) noexcept;
        std::filesystem::path getOutputPath(const Input& input, const std::string& animName, const nlohmann::json& animConfig, const std::string& outputPath) noexcept;
        OzzAnimation read(const std::filesystem::path& path, const std::string& animationName);

        Input _currentInput;
        std::optional<OptimizationSettings> _currentOptimization;
        std::optional<OzzSkeleton> _currentSkeleton;
        std::shared_ptr<aiScene> _currentScene;
        std::vector<std::string> _currentAnimationNames;
        std::optional<float> _currentMinKeyframeDuration;
        OptionalRef<std::ostream> _log;

        AssimpLoader _assimpLoader;
        size_t _bufferSize;

        static const std::string _skeletonJsonKey;
        static const std::string _outputPathJsonKey;
        static const std::string _animationsJsonKey;
        static const std::string _useFileMatchAsNameJsonKey;
        static const std::string _optimizationJsonKey;
        static const std::string _optimizationToleranceJsonKey;
        static const std::string _optimizationDistanceJsonKey;
        static const std::string _optimizationJointsJsonKey;
        static const std::string _minKeyframeDurationJsonKey;
    };
}