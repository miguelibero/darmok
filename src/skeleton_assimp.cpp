#include "skeleton_assimp.hpp"
#include "skeleton_ozz.hpp"
#include <darmok/string.hpp>
#include <darmok/skeleton_assimp.hpp>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/base/io/archive.h>
#include <assimp/vector3.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <algorithm>
#include <regex>

namespace darmok
{
    struct AssimpOzzUtils final
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

        static aiMatrix4x4 getWorldTransform(const aiNode& node)
        {
            auto parent = node.mParent;
            aiMatrix4x4 trans = node.mTransformation;
            while (parent != nullptr)
            {
                trans = parent->mTransformation * trans;
                parent = parent->mParent;
            }
            return trans;
        }
    };

    AssimpOzzSkeletonConverter::AssimpOzzSkeletonConverter(const aiScene& scene) noexcept
        : _scene(scene)
    {
    }

    AssimpOzzSkeletonConverter& AssimpOzzSkeletonConverter::setBoneNames(const std::vector<std::string>& names) noexcept
    {
        _boneNames.clear();
        for (auto& name : names)
        {
            _boneNames.emplace(name, name);
        }
        return *this;
    }

    AssimpOzzSkeletonConverter& AssimpOzzSkeletonConverter::setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept
    {
        _boneNames = names;
        return *this;
    }

    AssimpOzzSkeletonConverter& AssimpOzzSkeletonConverter::setConfig(const nlohmann::json& config) noexcept
    {
        if (config.contains("bones"))
        {
            auto& bonesConfig = config["bones"];
            if (bonesConfig.is_array())
            {
                std::vector<std::string> boneNames = bonesConfig;
                setBoneNames(boneNames);
            }
            else
            {
                std::unordered_map<std::string, std::string> boneNames = bonesConfig;
                setBoneNames(boneNames);
            }
        }
        return *this;
    }

    std::vector<std::string> AssimpOzzSkeletonConverter::getSkeletonNames()
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

    bool AssimpOzzSkeletonConverter::update(RawSkeleton& skel) noexcept
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

    aiBone* AssimpOzzSkeletonConverter::findRootBone(const aiMesh& mesh, BoneNodes& boneNodes) noexcept
    {
        for (size_t i = 0; i < mesh.mNumBones; i++)
        {
            auto bone = mesh.mBones[i];
            if (!_boneNames.empty())
            {
                auto itr = _boneNames.find(bone->mName.C_Str());
                if (itr != _boneNames.end())
                {
                    boneNodes.emplace(bone->mNode, itr->second);
                }
            }
            else
            {
                boneNodes.emplace(bone->mNode, "");
            }
        }
        for (size_t i = 0; i < mesh.mNumBones; i++)
        {
            auto bone = mesh.mBones[i];
            if (bone == nullptr)
            {
                continue;
            }
            auto parent = bone->mNode->mParent;
            while (boneNodes.find(parent) == boneNodes.end())
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

    bool AssimpOzzSkeletonConverter::update(const aiMesh& mesh, RawSkeleton& skel)
    {
        BoneNodes boneNodes;
        auto rootBone = findRootBone(mesh, boneNodes);
        if (rootBone == nullptr)
        {
            return false;
        }
        auto& rootNode = *rootBone->mNode;
        auto rootTrans = AssimpOzzUtils::getWorldTransform(rootNode);
        auto& rootJoint = skel.roots.emplace_back();
        auto itr = boneNodes.find(&rootNode);
        rootJoint.name = itr == boneNodes.end() || itr->second.empty() ? rootNode.mName.C_Str() : itr->second;
        rootJoint.transform = AssimpOzzUtils::convert(rootTrans);
        for (size_t i = 0; i < rootNode.mNumChildren; i++)
        {
            update(*rootNode.mChildren[i], rootJoint, aiMatrix4x4(), boneNodes);
        }
        return true;
    }

    void AssimpOzzSkeletonConverter::update(const aiNode& node, RawSkeleton::Joint& parentJoint, const aiMatrix4x4& parentTrans, const BoneNodes& boneNodes)
    {
        RawSkeleton::Joint* joint = &parentJoint;
        auto trans = parentTrans * node.mTransformation;
        auto itr = boneNodes.find(&node);
        if (itr != boneNodes.end())
        {
            joint = &parentJoint.children.emplace_back();
            joint->name = itr->second.empty() ? node.mName.C_Str() : itr->second;
            joint->transform = AssimpOzzUtils::convert(trans);
            trans = aiMatrix4x4();
        }
        for (size_t i = 0; i < node.mNumChildren; i++)
        {
            update(*node.mChildren[i], *joint, trans, boneNodes);
        }
    }

    AssimpOzzSkeletonConverter::Skeleton AssimpOzzSkeletonConverter::createSkeleton()
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

    AssimpOzzAnimationConverter::AssimpOzzAnimationConverter(const aiScene& scene, OptionalRef<std::ostream> log) noexcept
        : _scene(scene)
        , _log(log)
    {
    }

    AssimpOzzAnimationConverter& AssimpOzzAnimationConverter::setBoneNames(const std::vector<std::string>& boneNames) noexcept
    {
        _boneNames = boneNames;
        return *this;
    }

    AssimpOzzAnimationConverter::Animation AssimpOzzAnimationConverter::createAnimation(const std::string& name)
    {
        RawAnimation rawAnim;
        if (!update(name, rawAnim))
        {
            throw std::runtime_error("failed to update the raw animation");
        }
        if (!rawAnim.Validate())
        {
            throw std::runtime_error("created an invalid raw skeleton");
        }
        ozz::animation::offline::AnimationBuilder builder;
        return std::move(*builder(rawAnim));
    }

    AssimpOzzAnimationConverter::Animation AssimpOzzAnimationConverter::createAnimation()
    {
        auto names = getAnimationNames();
        if (names.empty())
        {
            throw std::runtime_error("no animations found");
        }
        return createAnimation(names.front());
    }

    bool AssimpOzzAnimationConverter::update(const std::string& name, RawAnimation& anim)
    {
        for (size_t i = 0; i < _scene.mNumAnimations; i++)
        {
            auto assimpAnim = _scene.mAnimations[i];
            if (assimpAnim == nullptr || name != assimpAnim->mName.C_Str())
            {
                continue;
            }
            update(*assimpAnim, anim);
            return true;
        }
        return false;
    }

    bool AssimpOzzAnimationConverter::logInfo(const aiAnimation& assimpAnim, const std::string& prefix) noexcept
    {
        if (!_log)
        {
            return false;
        }
        std::vector<std::string> animChannels;
        for (size_t i = 0; i < assimpAnim.mNumChannels; i++)
        {
            animChannels.emplace_back(assimpAnim.mChannels[i]->mNodeName.C_Str());
        }
        auto& log = *_log;
        if (!prefix.empty())
        {
            log << prefix << std::endl;
        }
        log << "skeleton bone names: " << StringUtils::join(", ", _boneNames) << std::endl;
        log << "animation channels: " << StringUtils::join(", ", animChannels) << std::endl;
        return true;
    }

    bool AssimpOzzAnimationConverter::isBone(const aiNode& node) const noexcept
    {
        std::string name = node.mName.C_Str();
        return std::find(_boneNames.begin(), _boneNames.end(), name) != _boneNames.end();
    }

    void AssimpOzzAnimationConverter::update(const aiAnimation& assimpAnim, RawAnimation& anim)
    {
        anim.name = assimpAnim.mName.C_Str();
        anim.duration = float(assimpAnim.mDuration / assimpAnim.mTicksPerSecond);
        anim.tracks.resize(_boneNames.size());

        for (size_t i = 0; i < _boneNames.size(); i++)
        {
            auto& boneName = _boneNames[i];
            size_t j = 0;
            bool found = false;
            for (; j < assimpAnim.mNumChannels; j++)
            {
                if (boneName == assimpAnim.mChannels[j]->mNodeName.C_Str())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::string err = "could not find skeleton bone \"" + boneName
                    + "\" in animation \"" + assimpAnim.mName.C_Str() + "\"";
                logInfo(assimpAnim, err);
                // throw std::runtime_error(err);
                continue;
            }


            aiVector3D basePos;
            aiVector3D baseScale(1);
            aiQuaternion baseRot;

            auto node = _scene.mRootNode->FindNode(boneName.c_str());
            aiMatrix4x4 baseTrans;
            if (node)
            {
                auto parent = node->mParent;
                while (parent != nullptr && !isBone(*parent))
                {
                    baseTrans = parent->mTransformation * baseTrans;
                    parent = parent->mParent;
                }
                if (parent == nullptr)
                {
                    baseTrans = aiMatrix4x4();
                }
            }
            baseTrans.Decompose(baseScale, baseRot, basePos);

            auto& track = anim.tracks[i];
            auto& chan = *assimpAnim.mChannels[j];

            track.translations.resize(chan.mNumPositionKeys);
            for (size_t j = 0; j < chan.mNumPositionKeys; j++)
            {
                auto& key = chan.mPositionKeys[j];
                auto& elm = track.translations[j];
                auto val = baseTrans * key.mValue;
                elm.value = AssimpOzzUtils::convert(val);
                elm.time = float(key.mTime / assimpAnim.mTicksPerSecond);
            }
            track.rotations.resize(chan.mNumRotationKeys);
            for (size_t j = 0; j < chan.mNumRotationKeys; j++)
            {
                auto& key = chan.mRotationKeys[j];
                auto& elm = track.rotations[j];
                auto val = AssimpOzzUtils::convert(baseRot * key.mValue);

                // temp fix for https://github.com/assimp/assimp/issues/4714
                // https://github.com/assimp/assimp/discussions/4966
                // if (boneName == "B-thigh.L")
                // {
                //     std::cout << "rot " << val.x << " " << val.y << " " << val.z << " " << val.w << std::endl;
                // }

                elm.value = val;
                elm.time = float(key.mTime / assimpAnim.mTicksPerSecond);

            }
            track.scales.resize(chan.mNumScalingKeys);
            for (size_t j = 0; j < chan.mNumScalingKeys; j++)
            {
                auto& key = chan.mScalingKeys[j];
                auto& elm = track.scales[j];
                auto val = baseScale.SymMul(key.mValue);

                elm.value = AssimpOzzUtils::convert(val);
                elm.time = float(key.mTime / assimpAnim.mTicksPerSecond);
            }
        }
    }

    std::vector<std::string> AssimpOzzAnimationConverter::getAnimationNames()
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

    AssimpSkeletonLoaderImpl::AssimpSkeletonLoaderImpl(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<Skeleton> AssimpSkeletonLoaderImpl::operator()(std::string_view name)
    {
        AssimpSceneLoadConfig loadConfig;
        loadConfig.leftHanded = false;
        loadConfig.populateArmature = true;
        auto scene = _sceneLoader.loadFromMemory(_dataLoader(name), std::string(name), loadConfig);
        if (!scene)
        {
            throw new std::runtime_error("could not load assimp scene");
        }
        auto ozz = AssimpOzzSkeletonConverter(*scene).createSkeleton();
        return std::make_shared<Skeleton>(std::make_unique<SkeletonImpl>(std::move(ozz)));
    }

    AssimpSkeletalAnimationLoaderImpl::AssimpSkeletalAnimationLoaderImpl(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<SkeletalAnimation> AssimpSkeletalAnimationLoaderImpl::operator()(std::string_view name)
    {
        AssimpSceneLoadConfig loadConfig;
        loadConfig.leftHanded = false;
        loadConfig.populateArmature = false;
        auto scene = _sceneLoader.loadFromMemory(_dataLoader(name), std::string(name), loadConfig);
        if (!scene)
        {
            throw new std::runtime_error("could not load assimp scene");
        }

        auto ozz = AssimpOzzAnimationConverter(*scene).createAnimation();
        return std::make_shared<SkeletalAnimation>(std::make_unique<SkeletalAnimationImpl>(std::move(ozz)));
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

    AssimpSkeletonImporterImpl::OzzSkeleton AssimpSkeletonImporterImpl::read(const std::filesystem::path& path, const nlohmann::json& config)
    {
        AssimpSceneLoadConfig loadConfig;
        loadConfig.leftHanded = false;
        loadConfig.populateArmature = true;
        auto scene = _sceneLoader.loadFromFile(path, loadConfig);
        if (!scene)
        {
            throw std::runtime_error("could not load assimp scene");
        }
        AssimpOzzSkeletonConverter converter(*scene);
        converter.setConfig(config);
        return converter.createSkeleton();
    }

    std::vector<std::filesystem::path> AssimpSkeletonImporterImpl::getOutputs(const Input& input) noexcept
    {
        std::vector<std::filesystem::path> outputs;
        if (input.config.is_null())
        {
            return outputs;
        }
        if (!_sceneLoader.supports(input.path.string()))
        {
            return outputs;
        }
        std::filesystem::path outputPath;
        if (input.config.contains("outputPath"))
        {
            outputPath = input.config["outputPath"].get<std::string>();
        }
        else
        {
            auto stem = std::string(StringUtils::getFileStem(input.path.stem().string()));
            outputPath = stem + ".ozz";
        }
        auto basePath = input.getRelativePath().parent_path();
        outputs.push_back(basePath  / outputPath);
        return outputs;
    }

    std::ofstream AssimpSkeletonImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    void AssimpSkeletonImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        auto skel = read(input.path, input.config);
        AssimpOzzUtils::writeToStream(skel, out, _bufferSize);
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
        // empty on purpose
    }

    std::vector<std::filesystem::path> AssimpSkeletonImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
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

    void AssimpSkeletalAnimationImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _log = log;
    }

    std::filesystem::path AssimpSkeletalAnimationImporterImpl::getOutputPath(const Input& input, const std::string& animName, const std::string& outputPath) noexcept
    {
        static const std::string matchIndexKey = "useFileMatchAsName";
        auto fanimName = animName;

        std::optional<size_t> matchIndex;
        if(input.config.contains(matchIndexKey))
        {
            matchIndex = input.config[matchIndexKey].get<size_t>();
        }
        else if (input.dirConfig.contains(matchIndexKey))
        {
            matchIndex = input.dirConfig[matchIndexKey].get<size_t>();
        }
        if (matchIndex && input.pathMatches.size() > matchIndex.value())
        {
            fanimName = input.pathMatches[matchIndex.value()];
        }
        std::string animOutputPath = fanimName + ".ozz";
        if (!outputPath.empty())
        {
            animOutputPath = outputPath;
            StringUtils::replace(animOutputPath, "*", fanimName);
        }
        return input.getRelativePath().parent_path() / animOutputPath;
    }

    std::filesystem::path AssimpSkeletalAnimationImporterImpl::getOutputPath(const Input& input, const std::string& animName, const nlohmann::json& animConfig, const std::string& outputPath) noexcept
    {
        std::string fixedOutputPath = outputPath;
        if (animConfig.contains("outputPath"))
        {
            fixedOutputPath = animConfig["outputPath"];
        }
        return getOutputPath(input, animName, fixedOutputPath);
    }

    void AssimpSkeletalAnimationImporterImpl::loadSkeleton(const std::filesystem::path& path, const nlohmann::json& config)
    {
        AssimpSceneLoadConfig loadConfig;
        loadConfig.leftHanded = false;
        loadConfig.populateArmature = true;
        auto skelScene = _sceneLoader.loadFromFile(path, loadConfig);
        AssimpOzzSkeletonConverter converter(*skelScene);
        converter.setConfig(config);
        auto skel = converter.createSkeleton();

        _currentSkeletonJoints.resize(skel.num_joints());
        for (int i = 0; i < skel.num_joints(); ++i)
        {
            _currentSkeletonJoints[i] = skel.joint_names()[i];
        }
    }

    bool AssimpSkeletalAnimationImporterImpl::startImport(const Input& input, bool dry)
    {
        if (input.config.is_null())
        {
            return false;
        }
        if (!_sceneLoader.supports(input.path.string()))
        {
            return false;
        }
        _currentInput = input;
        AssimpSceneLoadConfig loadConfig;
        loadConfig.leftHanded = false;
        loadConfig.populateArmature = false;
        _currentScene = _sceneLoader.loadFromFile(input.path, loadConfig);
        if (_currentScene == nullptr)
        {
            throw std::runtime_error("could not load assimp scene");
        }

        if (!dry)
        {
            std::filesystem::path skelPath = input.path;
            if (input.config.contains("skeleton"))
            {
                skelPath = input.path.parent_path() / input.config["skeleton"];
            }
            else if (input.dirConfig.contains("skeleton"))
            {
                skelPath = input.path.parent_path() / input.dirConfig["skeleton"];
            }
            loadSkeleton(skelPath, input.config);
        }
        return true;
    }

    std::vector<std::filesystem::path> AssimpSkeletalAnimationImporterImpl::getOutputs(const Input& input)
    {
        std::vector<std::filesystem::path> outputs;

        std::string outputPath;
        if (input.config.contains("outputPath"))
        {
            outputPath = input.config["outputPath"];
        }

        AssimpOzzAnimationConverter converter(*_currentScene);
        auto allAnimationNames = converter.getAnimationNames();
        if (input.config.contains("animations"))
        {
            for (auto& [animName, animConfig] : input.config["animations"].items())
            {
                if (StringUtils::containsGlobPattern(animName))
                {
                    auto regex = std::regex(StringUtils::globToRegex(animName));
                    for (auto& animName : allAnimationNames)
                    {
                        if (std::regex_match(animName, regex))
                        {
                            _currentAnimationNames.emplace_back(animName);
                            outputs.push_back(getOutputPath(input, animName, animConfig, outputPath));
                        }
                    }
                }
                else
                {
                    _currentAnimationNames.emplace_back(animName);
                    outputs.push_back(getOutputPath(input, animName, animConfig, outputPath));
                }
            }
        }
        else
        {
            _currentAnimationNames = allAnimationNames;
            for (auto& animName : _currentAnimationNames)
            {
                outputs.push_back(getOutputPath(input, animName, outputPath));
            }
        }

        return outputs;
    }

    void AssimpSkeletalAnimationImporterImpl::endImport(const Input& input) noexcept
    {
        _currentInput = Input{};
        _currentScene.reset();
        _currentSkeletonJoints.clear();
        _currentAnimationNames.clear();
    }

    std::ofstream AssimpSkeletalAnimationImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
    {
        return std::ofstream(path, std::ios::binary);
    }

    AssimpSkeletalAnimationImporterImpl::OzzAnimation AssimpSkeletalAnimationImporterImpl::read(const std::filesystem::path& path, const std::string& animationName)
    {
        AssimpOzzAnimationConverter converter(*_currentScene, _log);
        converter.setBoneNames(_currentSkeletonJoints);
        return converter.createAnimation(animationName);
    }

    void AssimpSkeletalAnimationImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        auto& animName = _currentAnimationNames[outputIndex];
        auto anim = read(input.path, animName);
        AssimpOzzUtils::writeToStream(anim, out, _bufferSize);
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

    void AssimpSkeletalAnimationImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _impl->setLogOutput(log);
    }

    bool AssimpSkeletalAnimationImporter::startImport(const Input& input, bool dry)
    {
        return _impl->startImport(input, dry);
    }

    void AssimpSkeletalAnimationImporter::endImport(const Input& input)
    {
        return _impl->endImport(input);
    }

    std::vector<std::filesystem::path> AssimpSkeletalAnimationImporter::getOutputs(const Input& input)
    {
        return _impl->getOutputs(input);
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
        AssimpSceneLoadConfig loadConfig;
        loadConfig.leftHanded = false;
        loadConfig.populateArmature = true;
        _assimpScene = _sceneLoader.loadFromFile(_path, loadConfig);
        return _assimpScene != nullptr;
    }

    bool AssimpOzzImporter::Import(RawSkeleton* skeleton, const NodeType& types)
    {
        if (!_assimpScene)
        {
            return false;
        }
        AssimpOzzSkeletonConverter converter(*_assimpScene);
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
        AssimpOzzAnimationConverter converter(*_assimpScene);
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
        AssimpOzzAnimationConverter converter(*_assimpScene);
        // TODO: check samplingRate
        return converter.update(animationName, *animation);
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