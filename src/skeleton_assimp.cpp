#include "skeleton_assimp.hpp"
#include "skeleton_ozz.hpp"
#include <darmok/string.hpp>
#include <darmok/skeleton_assimp.hpp>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/base/io/archive.h>

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
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<Skeleton> AssimpSkeletonLoaderImpl::operator()(std::string_view name)
    {
        auto scene = _sceneLoader.loadFromMemory(_dataLoader(name).view(), std::string(name));
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

    void AssimpSkeletonImporterImpl::read(const std::filesystem::path& path, RawSkeleton& skeleton)
    {
        auto scene = _sceneLoader.loadFromFile(path);
        AssimpSkeletonConverter converter(*scene);
        converter.update(skeleton);
    }

    size_t AssimpSkeletonImporterImpl::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) noexcept
    {
        if (input.config.empty())
        {
            return 0;
        }
        if (!_sceneLoader.supports(input.path.string()))
        {
            return 0;
        }
        auto stem = std::string(StringUtils::getFileStem(input.path.stem().string()));
        outputs.push_back(input.getRelativePath().parent_path() / (stem + ".ozz"));
        return 1;
    }

    std::ofstream AssimpSkeletonImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    void AssimpSkeletonImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        RawSkeleton skel;
        read(input.path, skel);
        ozz::io::File file(input.path.string().c_str(), "wb");
        ozz::io::OArchive archive(&file);
        archive << skel;
    }

    const std::string& AssimpSkeletonImporterImpl::getName() const noexcept
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

    size_t AssimpSkeletonImporter::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs)
    {
        return _impl->getOutputs(input, outputs);
    }

    std::ofstream AssimpSkeletonImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void AssimpSkeletonImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        return _impl->writeOutput(input, outputIndex, out);
    }

    const std::string& AssimpSkeletonImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    size_t AssimpSkeletalAnimationImporterImpl::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs) noexcept
    {
        if (input.config.empty())
        {
            return 0;
        }
        if (!_sceneLoader.supports(input.path.string()))
        {
            return 0;
        }
        auto stem = std::string(StringUtils::getFileStem(input.path.stem().string()));
        outputs.push_back(input.getRelativePath().parent_path() / (stem + ".ozz"));
        return 1;
    }

    std::ofstream AssimpSkeletalAnimationImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    void AssimpSkeletalAnimationImporterImpl::read(const std::filesystem::path& path, RawAnimation& anim)
    {
        auto scene = _sceneLoader.loadFromFile(path);
        AssimpSkeletonConverter converter(*scene);

        // TODO
    }

    void AssimpSkeletalAnimationImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        RawAnimation anim;
        read(input.path, anim);
        ozz::io::File file(input.path.string().c_str(), "wb");
        ozz::io::OArchive archive(&file);
        archive << anim;
    }

    const std::string& AssimpSkeletalAnimationImporterImpl::getName() const noexcept
    {
        static const std::string name("skeletal_animation");
        return name;
    }

    AssimpSkeletalAnimationImporter::AssimpSkeletalAnimationImporter() noexcept
        : _impl(std::make_unique<AssimpSkeletalAnimationImporterImpl>())
    {
    }

    AssimpSkeletalAnimationImporter::~AssimpSkeletalAnimationImporter() noexcept
    {
        // empty on purpose
    }

    size_t AssimpSkeletalAnimationImporter::getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs)
    {
        return _impl->getOutputs(input, outputs);
    }

    std::ofstream AssimpSkeletalAnimationImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return _impl->createOutputStream(input, outputIndex, path);
    }

    void AssimpSkeletalAnimationImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        _impl->writeOutput(input, outputIndex, out);
    }

    const std::string& AssimpSkeletalAnimationImporter::getName() const noexcept
    {
        return _impl->getName();
    }
}