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

namespace ozz::animation::offline
{
    struct RawAnimation;
}

namespace ozz::animation
{
    class Skeleton;
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

    class AssimpSkeletonConverter final
    {
    public:
        using OzzSkeleton = ozz::animation::Skeleton;
        using RawSkeleton = ozz::animation::offline::RawSkeleton;
        using RawAnimation = ozz::animation::offline::RawAnimation;
        AssimpSkeletonConverter(const aiScene& scene) noexcept;
        bool update(RawSkeleton& skel) noexcept;
        bool update(const std::string& name, const OzzSkeleton& skel, RawAnimation& anim) noexcept;
        std::vector<std::string> getSkeletonNames();
        std::vector<std::string> getAnimationNames();
        std::unique_ptr<OzzSkeleton> createSkeleton();
    private:
        const aiScene& _scene;
        aiBone* findRootBone(const aiMesh& mesh, std::vector<aiNode*>& boneNodes) noexcept;
        bool update(const aiAnimation& assimpAnim, const OzzSkeleton& skel, RawAnimation& anim) noexcept;
        bool update(const aiMesh& mesh, RawSkeleton& skel);
        void update(const aiNode& node, RawSkeleton::Joint& parentJoint, const aiMatrix4x4& parentTrans, const std::vector<aiNode*>& boneNodes);

        // void extractSkeleton(const aiNode& node) const;
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

    struct AssetTypeImporterInput;

    class AssimpSkeletonImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        using OzzSkeleton = ozz::animation::Skeleton;

        AssimpSkeletonImporterImpl(size_t bufferSize = 4096) noexcept;
        std::unique_ptr<OzzSkeleton> read(const std::filesystem::path& path);
        size_t getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) noexcept;
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
        using RawAnimation = ozz::animation::offline::RawAnimation;

        AssimpSkeletalAnimationImporterImpl(size_t bufferSize = 4096) noexcept;
        void read(const std::filesystem::path& path, RawAnimation& anim);
        size_t getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) noexcept;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        const std::string& getName() const noexcept;
    private:
        AssimpSceneLoader _sceneLoader;
        size_t _bufferSize;
    };
}