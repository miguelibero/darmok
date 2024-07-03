#pragma once
#include <optional>
#include <vector>
#include <string>
#include <filesystem>
#include <darmok/data.hpp>
#include <bx/allocator.h>
#include <bx/file.h>
#include "model_assimp.hpp"
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/tools/import2ozz.h>
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
    private:
        const aiScene& _scene;
        aiBone* findRootBone(const aiMesh& mesh, std::vector<aiNode*>& boneNodes) noexcept;
        bool update(const aiMesh& mesh, RawSkeleton& skel);
        void update(const aiNode& node, RawSkeleton::Joint& parentJoint, const aiMatrix4x4& parentTrans, const std::vector<aiNode*>& boneNodes);
    };

    class AssimpOzzAnimationConverter final
    {
    public:
        using Animation = ozz::animation::Animation;
        using RawAnimation = ozz::animation::offline::RawAnimation;
        AssimpOzzAnimationConverter(const aiScene& scene, OptionalRef<std::ostream> log = nullptr) noexcept;
        AssimpOzzAnimationConverter& setJointNames(const std::vector<std::string> jointNames) noexcept;
        Animation createAnimation(const std::string& name);
        Animation createAnimation();
        bool update(const std::string& name, RawAnimation& anim);
        std::vector<std::string> getAnimationNames();
    private:
        OptionalRef<std::ostream> _log;
        const aiScene& _scene;
        std::vector<std::string> _jointNames;
        void update(const aiAnimation& assimpAnim, RawAnimation& anim);
        bool isJoint(const aiNode& node) const noexcept;
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
        AssimpSceneLoader _sceneLoader;
    };

    class IDataLoader;

    class AssimpSkeletonLoaderImpl final
    {
    public:
        AssimpSkeletonLoaderImpl(IDataLoader& dataLoader) noexcept;
        std::shared_ptr<Skeleton> operator()(std::string_view name);
    private:
        IDataLoader& _dataLoader;
        AssimpSceneLoader _sceneLoader;
    };

    class AssimpSkeletalAnimationLoaderImpl final
    {
    public:
        AssimpSkeletalAnimationLoaderImpl(IDataLoader& dataLoader) noexcept;
        std::shared_ptr<SkeletalAnimation> operator()(std::string_view name);
    private:
        IDataLoader& _dataLoader;
        AssimpSceneLoader _sceneLoader;
    };

    struct AssetTypeImporterInput;

    class AssimpSkeletonImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        using OzzSkeleton = ozz::animation::Skeleton;

        AssimpSkeletonImporterImpl(size_t bufferSize = 4096) noexcept;
        OzzSkeleton read(const std::filesystem::path& path);
        std::vector<std::filesystem::path> getOutputs(const Input& input) noexcept;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;
    private:
        size_t _bufferSize;
        AssimpSceneLoader _sceneLoader;

    };

    class AssimpSkeletalAnimationImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        using OzzAnimation = ozz::animation::Animation;

        AssimpSkeletalAnimationImporterImpl(size_t bufferSize = 4096) noexcept;
        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        OzzAnimation read(const std::filesystem::path& path, const std::string& animationName);
        bool startImport(const Input& input, bool dry);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        void endImport(const Input& input) noexcept;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;
    private:
        void loadSkeleton(const std::filesystem::path& path);
        std::filesystem::path getOutputPath(const Input& input, const std::string& animName, const std::string& outputPath) noexcept;
        std::filesystem::path getOutputPath(const Input& input, const std::string& animName, const nlohmann::json& animConfig, const std::string& outputPath) noexcept;
        Input _currentInput;
        std::vector<std::string> _currentSkeletonJoints;
        std::shared_ptr<aiScene> _currentScene;
        std::vector<std::string> _currentAnimationNames;
        OptionalRef<std::ostream> _log;

        AssimpSceneLoader _sceneLoader;
        size_t _bufferSize;
    };
}