#include "skeleton_assimp.hpp"
#include "skeleton_ozz.hpp"
#include <darmok/string.hpp>
#include <darmok/skeleton_assimp.hpp>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/base/io/archive.h>
#include <assimp/vector3.h>
#include <assimp/matrix4x4.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <algorithm>

namespace darmok
{
    struct OzzAssimpUtils final
    {
        static ozz::math::Float3 convert(const aiVector3D& v) noexcept
        {
            return { v.x, v.y, v.z };
        }

        static ozz::math::Transform convert(const aiMatrix4x4& m) noexcept
        {
            aiVector3D s;
            aiQuaternion r;
            aiVector3D t;
            m.Decompose(s, r, t);
            return ozz::math::Transform{
              .translation = convert(t),
              .rotation = convert(r),
              .scale = convert(s),
            };
        }

        static ozz::math::Quaternion convert(const aiQuaternion& q) noexcept
        {
            return { q.x, q.y, q.z, q.w };
        }

        template<typename T>
        static void writeToStream(const T& input, std::ostream& output, size_t bufferSize)
        {
            // TODO: write std::ostream implementation of ozz::io::Stream to avoid memory creation
            ozz::io::MemoryStream mem;
            ozz::io::OArchive archive(&mem);
            archive << input;
            mem.Seek(0, ozz::io::Stream::kSet);
            std::vector<char> buffer(bufferSize);
            while (auto count = mem.Read(&buffer.front(), bufferSize))
            {
                output.write(&buffer.front(), count);
            }
        }
    };


    AssimpSkeletonConverter::AssimpSkeletonConverter(const aiScene& scene) noexcept
        : _scene(scene)
    {
    }

    std::vector<std::string> AssimpSkeletonConverter::getSkeletonNames()
    {
        std::vector<std::string> names;
        for (size_t i = 0; i < _scene.mNumMeshes; i++)
        {
            auto mesh = _scene.mMeshes[i];
            if (!mesh->HasBones())
            {
                names.emplace_back(mesh->mName.C_Str());
            }
        }
        return names;
    }

    bool AssimpSkeletonConverter::update(RawSkeleton& skel) noexcept
    {
        for (size_t i = 0; i < _scene.mNumMeshes ; i++)
        {
            auto mesh = _scene.mMeshes[i];
            if (mesh == nullptr || !mesh->HasBones())
            {
                continue;
            }
            update(*mesh, skel);
            return true;
        }
        return false;
    }

    aiBone* AssimpSkeletonConverter::findRootBone(const aiMesh& mesh, std::vector<aiNode*>& boneNodes) noexcept
    {
        for (size_t i = 0; i < mesh.mNumBones; i++)
        {
            boneNodes.push_back(mesh.mBones[i]->mNode);
        }
        for (size_t i = 0; i < mesh.mNumBones; i++)
        {
            auto bone = mesh.mBones[i];
            if (bone == nullptr)
            {
                continue;
            }
            auto parent = bone->mNode->mParent;
            while (std::find(boneNodes.begin(), boneNodes.end(), parent) == boneNodes.end())
            {
                if (parent == nullptr)
                {
                    return bone;
                }
                parent = parent->mParent;
            }
        }
        return nullptr;
    }

    bool AssimpSkeletonConverter::update(const aiMesh& mesh, RawSkeleton& skel)
    {
        std::vector<aiNode*> boneNodes;
        auto rootBone = findRootBone(mesh, boneNodes);
        if (rootBone == nullptr)
        {
            return false;
        }
        auto& rootNode = *rootBone->mNode;
        auto rootTrans = rootNode.mTransformation;
        auto parent = rootNode.mParent;
        while (parent != nullptr)
        {
            rootTrans = parent->mTransformation * rootTrans;
            parent = parent->mParent;
        }
        auto& rootJoint = skel.roots.emplace_back();
        rootJoint.name = rootNode.mName.C_Str();
        rootJoint.transform = OzzAssimpUtils::convert(rootTrans);
        for (size_t i = 0; i < rootNode.mNumChildren; i++)
        {
            update(*rootNode.mChildren[i], rootJoint, aiMatrix4x4(), boneNodes);
        }
        return true;
    }

    void AssimpSkeletonConverter::update(const aiNode& node, RawSkeleton::Joint& parentJoint, const aiMatrix4x4& parentTrans, const std::vector<aiNode*>& boneNodes)
    {
        RawSkeleton::Joint* joint = &parentJoint;
        auto trans = parentTrans * node.mTransformation;
        if (std::find(boneNodes.begin(), boneNodes.end(), &node) != boneNodes.end())
        {
            *joint = parentJoint.children.emplace_back();
            joint->name = node.mName.C_Str();
            joint->transform = OzzAssimpUtils::convert(trans);
            trans = aiMatrix4x4();
        }
        for (size_t i = 0; i < node.mNumChildren; i++)
        {
            update(*node.mChildren[i], *joint, trans, boneNodes);
        }
    }

    bool AssimpSkeletonConverter::update(const std::string& name, const OzzSkeleton& skel, RawAnimation& anim) noexcept
    {
        for (size_t i = 0; i < _scene.mNumAnimations; i++)
        {
            auto assimpAnim = _scene.mAnimations[i];
            if (assimpAnim == nullptr || name != assimpAnim->mName.C_Str())
            {
                continue;
            }
            update(*assimpAnim, skel, anim);
            return true;
        }
        return false;
    }

    bool AssimpSkeletonConverter::update(const aiAnimation& assimpAnim, const OzzSkeleton& skel, RawAnimation& anim) noexcept
    {
        anim.name = assimpAnim.mName.C_Str();
        anim.duration = float(assimpAnim.mDuration / assimpAnim.mTicksPerSecond);
        anim.tracks.resize(skel.num_joints());

        for (size_t i = 0; i < assimpAnim.mNumChannels; i++)
        {
            auto chan = assimpAnim.mChannels[i];
            /*
            const auto jointIndex =
                ozz::animation::offline::FindJoint(skel, chan->mNodeName.C_Str());
                */
        }

        return false;
    }

    std::vector<std::string> AssimpSkeletonConverter::getAnimationNames()
    {
        std::vector<std::string> names;
        for (size_t i = 0; i < _scene.mNumAnimations; i++)
        {
            auto anim = _scene.mAnimations[i];
            if (anim != nullptr)
            {
                names.push_back(anim->mName.C_Str());
            }
        }
        return names;
    }

    AssimpSkeletonConverter::OzzSkeleton AssimpSkeletonConverter::createSkeleton()
    {
        RawSkeleton rawSkel;
        if (!update(rawSkel))
        {
            throw std::runtime_error("failed to update the raw skeleton");
        }
        if (!rawSkel.Validate())
        {
            throw std::runtime_error("created an invalid raw skeleton");
        }
        ozz::animation::offline::SkeletonBuilder builder;
        return std::move(*builder(rawSkel));
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
            throw new std::runtime_error("could not load assimp scene");
        }
        auto ozz = AssimpSkeletonConverter(*scene).createSkeleton();
        return std::make_shared<Skeleton>(std::make_unique<SkeletonImpl>(std::move(ozz)));
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


    AssimpSkeletonImporterImpl::AssimpSkeletonImporterImpl(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    AssimpSkeletonImporterImpl::OzzSkeleton AssimpSkeletonImporterImpl::read(const std::filesystem::path& path)
    {
        auto scene = _sceneLoader.loadFromFile(path);
        if (!scene)
        {
            throw std::runtime_error("could not load assimp scene");
        }
        return AssimpSkeletonConverter(*scene).createSkeleton();
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
        if (input.config.contains("outputPath"))
        {
            outputs.push_back(input.config["outputPath"]);
            return 1;
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
        auto skel = read(input.path);
        OzzAssimpUtils::writeToStream(skel, out, _bufferSize);
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

    AssimpSkeletalAnimationImporterImpl::AssimpSkeletalAnimationImporterImpl(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    std::vector<std::string> AssimpSkeletalAnimationImporterImpl::getAnimationNames(const Input& input) const
    {
        std::vector<std::string> animNames;
        if (input.config.contains("animationNames"))
        {
            for (auto& elm : input.config["animationNames"])
            {
                animNames.emplace_back(elm);
            }
        }
        else
        {
            auto scene = _sceneLoader.loadFromFile(input.path);
            AssimpSkeletonConverter converter(*scene);
            animNames = converter.getAnimationNames();
        }
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
        std::string outputPath;
        if (input.config.contains("outputPath"))
        {
            outputPath = input.config["outputPath"];
            if (!outputPath.contains("*"))
            {
                outputs.push_back(outputPath);
                return 1;
            }
        }

        auto animNames = getAnimationNames(input);
        for (auto& name : animNames)
        {
            std::filesystem::path animPath;
            if (outputPath.empty())
            {
                animPath = input.getRelativePath().parent_path() / (name + ".ozz");
            }
            else
            {
                auto animPathStr = outputPath;
                StringUtils::replace(animPathStr, "*", name);
                animPath = animPathStr;
            }
            outputs.push_back(animPath);
        }
        return animNames.size();
    }

    std::ofstream AssimpSkeletalAnimationImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    bool AssimpSkeletalAnimationImporterImpl::read(const std::filesystem::path& path, const std::string& animationName, RawAnimation& anim)
    {
        auto scene = _sceneLoader.loadFromFile(path);
        AssimpSkeletonConverter converter(*scene);
        auto skel = converter.createSkeleton();
        converter.update(animationName, skel, anim);
    }

    void AssimpSkeletalAnimationImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        auto animNames = getAnimationNames(input);
        auto& animName = animNames[outputIndex];
        RawAnimation anim;
        read(input.path, animName, anim);
        OzzAssimpUtils::writeToStream(anim, out, _bufferSize);
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

    bool AssimpOzzImporter::Load(const char* filename)
    {
        _path = filename;
        _assimpScene = _sceneLoader.loadFromFile(_path);
        return _assimpScene != nullptr;
    }

    bool AssimpOzzImporter::Import(RawSkeleton* skeleton, const NodeType& types)
    {
        if (!_assimpScene)
        {
            return false;
        }
        AssimpSkeletonConverter converter(*_assimpScene);
        // TODO: take NodeTypes into account?
        return converter.update(*skeleton);
    }

    AssimpOzzImporter::AnimationNames AssimpOzzImporter::GetAnimationNames()
    {
        AnimationNames names;
        if (!_assimpScene)
        {
            return names;
        }
        AssimpSkeletonConverter converter(*_assimpScene);
        for (auto& name : converter.getAnimationNames())
        {
            names.emplace_back(name);
        }
        return names;
    }

    bool AssimpOzzImporter::Import(const char* animationName,
        const OzzSkeleton& skeleton,
        float samplingRate, RawAnimation* animation)
    {
        if (!_assimpScene)
        {
            return false;
        }
        AssimpSkeletonConverter converter(*_assimpScene);
        // TODO: check samplingRate
        return converter.update(animationName, skeleton, *animation);
    }

    AssimpOzzImporter::NodeProperties AssimpOzzImporter::GetNodeProperties(const char* nodeName)
    {
        NodeProperties props;
        return props;
    }

    bool AssimpOzzImporter::Import(const char* animationName, const char* nodeName,
        const char* trackName, NodeProperty::Type trackType,
        float samplingRate, RawFloatTrack* track)
    {
        return false;
    }

    bool AssimpOzzImporter::Import(const char* animationName, const char* nodeName,
        const char* trackName, NodeProperty::Type trackType,
        float samplingRate, RawFloat2Track* track)
    {
        return false;
    }

    bool AssimpOzzImporter::Import(const char* animationName, const char* nodeName,
        const char* trackName, NodeProperty::Type trackType,
        float samplingRate, RawFloat3Track* track)
    {
        return false;
    }

    bool AssimpOzzImporter::Import(const char* animationName, const char* nodeName,
        const char* trackName, NodeProperty::Type trackType,
        float samplingRate, RawFloat4Track* track)
    {
        return false;
    }
}