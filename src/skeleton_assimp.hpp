#pragma once
#include <optional>
#include <vector>
#include <string>
#include <filesystem>
#include <darmok/data.hpp>
#include <bx/allocator.h>
#include <bx/file.h>
#include "model_assimp.hpp"

namespace ozz::animation::offline
{
    struct RawSkeleton;
    struct RawAnimation;
}

struct aiScene;
struct aiNode;

namespace darmok
{
    class Skeleton;
    class SkeletalAnimation;

    class AssimpSkeletonConverter final
    {
    public:
        using RawSkeleton = ozz::animation::offline::RawSkeleton;
        using RawAnimation = ozz::animation::offline::RawAnimation;
        AssimpSkeletonConverter(const aiScene& scene) noexcept;
        bool update(RawSkeleton& skel) noexcept;
        bool update(const std::string& name, RawAnimation& anim) noexcept;
        std::vector<std::string> getAnimationNames();
        std::shared_ptr<Skeleton> createSkeleton();
    private:
        const aiScene& _scene;

        // void extractSkeleton(const aiNode& node) const;
    };

    class IDataLoader;

    class AssimpSkeletonLoaderImpl final
    {
    public:
        AssimpSkeletonLoaderImpl(IDataLoader& dataLoader) noexcept;
        std::shared_ptr<Skeleton> operator()(std::string_view name);
    private:
        AssimpSceneLoader _sceneLoader;
    };

    struct AssetTypeImporterInput;

    class AssimpSkeletonImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;

        AssimpSkeletonImporterImpl() noexcept;
        size_t getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs) noexcept;
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        std::string getName() const noexcept;
    private:
        bx::DefaultAllocator _allocator;
        bx::FileReader _fileReader;
        FileDataLoader _dataLoader;
        AssimpSceneLoader _sceneLoader;
    };
}