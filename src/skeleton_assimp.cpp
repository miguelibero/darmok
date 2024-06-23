#include "skeleton_assimp.hpp"
#include "skeleton_ozz.hpp"
#include <darmok/string.hpp>
#include <darmok/skeleton_assimp.hpp>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/skeleton_builder.h>

namespace darmok
{
    AssimpSkeletonConverter::AssimpSkeletonConverter(const aiScene& scene) noexcept
        : _scene(scene)
    {
    }

    bool AssimpSkeletonConverter::update(RawSkeleton& skel) noexcept
    {
        return false;
    }

    bool AssimpSkeletonConverter::update(const std::string& name, RawAnimation& anim) noexcept
    {
        return false;
    }

    std::vector<std::string> AssimpSkeletonConverter::getAnimationNames()
    {
        std::vector<std::string> names;
        return names;
    }

    std::shared_ptr<Skeleton> AssimpSkeletonConverter::createSkeleton()
    {
        RawSkeleton rawSkel;
        update(rawSkel);
        ozz::animation::offline::SkeletonBuilder builder;
        auto skel = builder(rawSkel);
        return std::make_shared<Skeleton>(std::make_unique<SkeletonImpl>(std::move(*skel)));
    }

    AssimpSkeletonLoaderImpl::AssimpSkeletonLoaderImpl(IDataLoader& dataLoader) noexcept
        : _sceneLoader(dataLoader)
    {
    }

    std::shared_ptr<Skeleton> AssimpSkeletonLoaderImpl::operator()(std::string_view name)
    {
        auto scene = _sceneLoader(name);
        if (!scene)
        {
            return nullptr;
        }
        return AssimpSkeletonConverter(*scene).createSkeleton();
    }

    AssimpSkeletonLoader::AssimpSkeletonLoader(IDataLoader& dataLoader) noexcept
        : _impl(std::make_unique<AssimpSkeletonLoaderImpl>(dataLoader))
    {
    }

    AssimpSkeletonLoader::~AssimpSkeletonLoader() noexcept
    {
        // empty on purpose
    }

    AssimpSkeletonLoader::result_type AssimpSkeletonLoader::operator()(std::string_view name)
    {
        return (*_impl)(name);
    }

    AssimpSkeletonImporterImpl::AssimpSkeletonImporterImpl() noexcept
        : _dataLoader(_fileReader, _allocator)
        , _sceneLoader(_dataLoader)
    {
    }

    size_t AssimpSkeletonImporterImpl::getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs) noexcept
    {
        if (input.config.empty())
        {
            return 0;
        }
        if (!_sceneLoader.supports(input.path.string()))
        {
            return 0;
        }
        auto relPath = std::filesystem::relative(input.path, basePath);
        auto stem = StringUtils::getFileStem(relPath.stem().string());
        outputs.push_back(relPath.parent_path() / (stem + ".ozz"));
        return 1;
    }

    std::ofstream AssimpSkeletonImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    void AssimpSkeletonImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
    }

    std::string AssimpSkeletonImporterImpl::getName() const noexcept
    {
        static const std::string name("skeleton");
        return name;
    }

    AssimpSkeletonImporter::AssimpSkeletonImporter() noexcept
        : _impl(std::make_unique<AssimpSkeletonImporterImpl>())
    {
    }

    AssimpSkeletonImporter::~AssimpSkeletonImporter() noexcept
    {
    }

    size_t AssimpSkeletonImporter::getOutputs(const Input& input, const std::filesystem::path& basePath, std::vector<std::filesystem::path>& outputs)
    {
        return _impl->getOutputs(input, basePath, outputs);
    }

    std::ofstream AssimpSkeletonImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void AssimpSkeletonImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    std::string AssimpSkeletonImporter::getName() const noexcept
    {
        return _impl->getName();
    }
}