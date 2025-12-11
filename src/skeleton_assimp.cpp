#include "detail/skeleton_assimp.hpp"
#include "detail/skeleton_ozz.hpp"
#include <darmok/string.hpp>
#include <darmok/stream.hpp>
#include <darmok/skeleton_assimp.hpp>
#include <darmok/glm_serialize.hpp>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/animation_optimizer.h>
#include <ozz/animation/offline/track_optimizer.h>
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
            ozz::io::OArchive archive{ &mem };
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
        for (size_t i = 0; i < _scene.mNumMeshes; ++i)
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
        bool found = false;
        for (size_t i = 0; i < _scene.mNumMeshes ; ++i)
        {
            auto mesh = _scene.mMeshes[i];
            if (mesh == nullptr || !mesh->HasBones())
            {
                continue;
            }
            if (update(*mesh, skel))
            {
                found = true;
            }
        }
        return found;
    }

    aiBone* AssimpOzzSkeletonConverter::findRootBone(const aiMesh& mesh, BoneNodes& boneNodes) noexcept
    {
        for (size_t i = 0; i < mesh.mNumBones; ++i)
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
        for (size_t i = 0; i < mesh.mNumBones; ++i)
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

    OptionalRef<AssimpOzzSkeletonConverter::RawSkeleton::Joint> AssimpOzzSkeletonConverter::findJoint(RawSkeleton::Joint::Children& joints, const ozz::string& name, bool recursive) noexcept
    {
        for (auto& joint : joints)
        {
            if (joint.name == name)
            {
                return joint;
            }
            if (recursive)
            {
                if (auto child = findJoint(joint.children, name, recursive))
                {
                    return child;
                }
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
        auto itr = boneNodes.find(&rootNode);
        bool isRoot = itr == boneNodes.end() || itr->second.empty();
        const ozz::string name = isRoot ? rootNode.mName.C_Str() : itr->second.c_str();

        auto rootJoint = findJoint(skel.roots, name, true);
        if (!rootJoint)
        {
            rootJoint = skel.roots.emplace_back();
            rootJoint->name = name;
            auto rootTrans = AssimpOzzUtils::getWorldTransform(rootNode);
            rootJoint->transform = AssimpOzzUtils::convert(rootTrans);
        }

        for (size_t i = 0; i < rootNode.mNumChildren; ++i)
        {
            update(*rootNode.mChildren[i], rootJoint.value(), aiMatrix4x4(), boneNodes);
        }
        return true;
    }

    void AssimpOzzSkeletonConverter::update(const aiNode& node, RawSkeleton::Joint& parentJoint, const aiMatrix4x4& parentTrans, const BoneNodes& boneNodes)
    {
        OptionalRef<RawSkeleton::Joint> joint = &parentJoint;
        auto trans = parentTrans * node.mTransformation;
        auto itr = boneNodes.find(&node);
        if (itr != boneNodes.end())
        {
            const ozz::string name = itr->second.empty() ? node.mName.C_Str() : itr->second.c_str();
            joint = findJoint(parentJoint.children, name, false);
            if (!joint)
            {
                joint = parentJoint.children.emplace_back();
                joint->name = name;
                joint->transform = AssimpOzzUtils::convert(trans);
            }
            trans = aiMatrix4x4();
        }
        for (size_t i = 0; i < node.mNumChildren; ++i)
        {
            update(*node.mChildren[i], joint.value(), trans, boneNodes);
        }
    }

    expected<AssimpOzzSkeletonConverter::Skeleton, std::string> AssimpOzzSkeletonConverter::createSkeleton() noexcept
    {
        RawSkeleton rawSkel;
        if (!update(rawSkel))
        {
            return unexpected<std::string>{ "failed to update the raw skeleton" };
        }
        if (!rawSkel.Validate())
        {
            return unexpected<std::string>{ "created an invalid raw skeleton" };
        }
        ozz::animation::offline::SkeletonBuilder builder;
        return std::move(*builder(rawSkel));
    }

    AssimpOzzAnimationConverter::AssimpOzzAnimationConverter(const aiScene& scene, OptionalRef<std::ostream> log) noexcept
        : _scene(scene)
        , _log(log)
        , _minKeyDuration(0.F)
    {
    }

    AssimpOzzAnimationConverter& AssimpOzzAnimationConverter::setBoneNames(const std::vector<std::string>& boneNames) noexcept
    {
        _boneNames = boneNames;
        return *this;
    }

    AssimpOzzAnimationConverter& AssimpOzzAnimationConverter::setSkeleton(const Skeleton& skel) noexcept
    {
        _skeleton = skel;

        if (_skeleton)
        {
            _boneNames.resize(_skeleton->num_joints());
            for (int i = 0; i < _boneNames.size(); ++i)
            {
                _boneNames[i] = _skeleton->joint_names()[i];
            }
        }

        return *this;
    }

    AssimpOzzAnimationConverter& AssimpOzzAnimationConverter::setMinKeyframeDuration(float v) noexcept
    {
        _minKeyDuration = v;
        return *this;
    }

    AssimpOzzAnimationConverter& AssimpOzzAnimationConverter::setOptimization(const OptimizationSettings& settings) noexcept
    {
        _optimization = settings;
        return *this;
    }

    std::optional<AssimpOzzAnimationConverter::RawAnimation> AssimpOzzAnimationConverter::optimize(const RawAnimation& animation) noexcept
    {
        if (!_optimization || !_skeleton)
        {
            return std::nullopt;
        }
        RawAnimation optimizedAnim;

        ozz::animation::offline::AnimationOptimizer jointOptimizer;

        auto& opt = _optimization.value();
        jointOptimizer.setting.distance = opt.defaultElement.distance;
        jointOptimizer.setting.tolerance = opt.defaultElement.tolerance;

        auto jointNames = _skeleton->joint_names();
        for (auto& [name, elm] : opt.elements)
        {
            for (int i = 0; i < jointNames.size(); ++i)
            {
                if (jointNames[i] == name)
                {
                    jointOptimizer.joints_setting_override[i] =
                        ozz::animation::offline::AnimationOptimizer::Setting(elm.tolerance, elm.distance);
                    break;
                }
            }
        }

        if (jointOptimizer(animation, _skeleton.value(), &optimizedAnim))
        {
            return optimizedAnim;
        }

        return std::nullopt;
    }

    expected<AssimpOzzAnimationConverter::Animation, std::string> AssimpOzzAnimationConverter::createAnimation(const std::string& name) noexcept
    {
        RawAnimation rawAnim;
        if (!update(name, rawAnim))
        {
            return unexpected<std::string>{ "failed to update the raw animation" };
        }
        if (!rawAnim.Validate())
        {
            return unexpected<std::string>{ "created an invalid raw skeleton" };
        }

        if (auto optimizedAnim = optimize(rawAnim))
        {
            rawAnim = optimizedAnim.value();
        }

        ozz::animation::offline::AnimationBuilder builder;
        return std::move(*builder(rawAnim));
    }

    expected<AssimpOzzAnimationConverter::Animation, std::string> AssimpOzzAnimationConverter::createAnimation() noexcept
    {
        auto names = getAnimationNames();
        if (names.empty())
        {
            return unexpected<std::string>{ "no animations found" };
        }

        return createAnimation(names.front());
    }

    expected<void, std::string> AssimpOzzAnimationConverter::update(const std::string& name, RawAnimation& anim) noexcept
    {
        for (size_t i = 0; i < _scene.mNumAnimations; ++i)
        {
            auto assimpAnim = _scene.mAnimations[i];
            if (assimpAnim == nullptr || name != assimpAnim->mName.C_Str())
            {
                continue;
            }
            auto result = update(*assimpAnim, anim);
            if (!result)
            {
                return result;
            }
        }
        return {};
    }

    bool AssimpOzzAnimationConverter::logInfo(const aiAnimation& assimpAnim, const std::string& prefix) noexcept
    {
        if (!_log)
        {
            return false;
        }
        std::vector<std::string> animChannels;
        for (size_t i = 0; i < assimpAnim.mNumChannels; ++i)
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
        const std::string name = node.mName.C_Str();
        return std::find(_boneNames.begin(), _boneNames.end(), name) != _boneNames.end();
    }

    void AssimpOzzAnimationConverter::update(const std::string& boneName, float tickPerSecond, const aiNodeAnim& assimpTrack, RawAnimation::JointTrack& track) noexcept
    {
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

        aiVector3D basePos;
        aiVector3D baseScale(1);
        aiQuaternion baseRot;
        baseTrans.Decompose(baseScale, baseRot, basePos);

        track.translations.reserve(assimpTrack.mNumPositionKeys);
        float lastTime = 0.F;
        // temp fix for https://github.com/assimp/assimp/issues/4714
        // seems like assimp is adding additional keyframes in the rotation
        auto checkMinDuration = [this, &lastTime](float time)
        {
            if (_minKeyDuration > 0.F && time - lastTime < _minKeyDuration)
            {
                return false;
            }
            lastTime = time;
            return true;
        };

        for (size_t i = 0; i < assimpTrack.mNumPositionKeys; ++i)
        {
            auto& key = assimpTrack.mPositionKeys[i];
            auto time = float(key.mTime / tickPerSecond);
            if (i > 0 && !checkMinDuration(time))
            {
                continue;
            }

            auto& elm = track.translations.emplace_back();
            auto val = baseTrans * key.mValue;
            elm.value = AssimpOzzUtils::convert(val);
            elm.time = time;
        }
        track.rotations.reserve(assimpTrack.mNumRotationKeys);
        lastTime = 0.F;
        for (size_t i = 0; i < assimpTrack.mNumRotationKeys; ++i)
        {
            auto& key = assimpTrack.mRotationKeys[i];
            auto time = float(key.mTime / tickPerSecond);
            if (i > 0 && !checkMinDuration(time))
            {
                continue;
            }

            auto& elm = track.rotations.emplace_back();
            auto val = AssimpOzzUtils::convert(baseRot * key.mValue);

            elm.value = val;
            elm.time = time;
        }

        track.scales.reserve(assimpTrack.mNumScalingKeys);
        for (size_t i = 0; i < assimpTrack.mNumScalingKeys; ++i)
        {
            auto& key = assimpTrack.mScalingKeys[i];
            auto time = float(key.mTime / tickPerSecond);
            if (i > 0 && !checkMinDuration(time))
            {
                continue;
            }

            auto& elm = track.scales.emplace_back();
            auto val = baseScale.SymMul(key.mValue);

            elm.value = AssimpOzzUtils::convert(val);
            elm.time = time;
        }
    }

    expected<void, std::string> AssimpOzzAnimationConverter::update(const aiAnimation& assimpAnim, RawAnimation& anim) noexcept
    {
        anim.name = assimpAnim.mName.C_Str();
        anim.duration = float(assimpAnim.mDuration / assimpAnim.mTicksPerSecond);
        anim.tracks.resize(_boneNames.size());

        std::vector<std::string> errors;

        for (size_t i = 0; i < _boneNames.size(); ++i)
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
                errors.push_back(std::move(err));
                continue;
            }

            auto& track = anim.tracks[i];
            auto& chan = *assimpAnim.mChannels[j];
            auto tps = assimpAnim.mTicksPerSecond;
            if (tps == 0.F)
            {
                tps = 1.F;
            }
            update(boneName, tps, chan, track);
        }
        return StringUtils::joinExpectedErrors(errors);
    }

    std::vector<std::string> AssimpOzzAnimationConverter::getAnimationNames() noexcept
    {
        std::vector<std::string> names;
        for (size_t i = 0; i < _scene.mNumAnimations; ++i)
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

    static AssimpLoader::Config createAssimpSkeletonLoadConfig() noexcept
    {
        return {
            .leftHanded = false
        };
    }

    AssimpSkeletonLoaderImpl::Result AssimpSkeletonLoaderImpl::operator()(const std::filesystem::path& path)
    {
        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
			return unexpected<std::string>{dataResult.error()};
        }
        auto assimpConfig = createAssimpSkeletonLoadConfig();
        assimpConfig.populateArmature = true;
        assimpConfig.setPath(path);
        auto assimpResult = _assimpLoader.loadFromMemory(dataResult.value(), assimpConfig);
        if (!assimpResult)
        {
            return unexpected<std::string>{"could not load assimp scene"};
        }
        auto ozzResult = AssimpOzzSkeletonConverter(*assimpResult.value()).createSkeleton();
        if (!ozzResult)
        {
            return unexpected{ std::move(ozzResult).error() };
        }
        auto impl = std::make_unique<SkeletonImpl>(std::move(ozzResult).value());
        return std::make_shared<Skeleton>(std::move(impl));
    }

    AssimpSkeletalAnimationLoaderImpl::AssimpSkeletalAnimationLoaderImpl(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    AssimpSkeletalAnimationLoaderImpl::Result AssimpSkeletalAnimationLoaderImpl::operator()(const std::filesystem::path& path)
    {
        auto assimpConfig = createAssimpSkeletonLoadConfig();
        assimpConfig.setPath(path);

        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
			return unexpected<std::string>{dataResult.error()};
        }
        auto sceneResult = _assimpLoader.loadFromMemory(dataResult.value(), assimpConfig);
        if (!sceneResult)
        {
            return unexpected<std::string>{"could not load assimp scene"};
        }

        auto ozzResult = AssimpOzzAnimationConverter(*sceneResult.value()).createAnimation();
        if (!ozzResult)
        {
            return unexpected{ std::move(ozzResult).error() };
        }
        auto impl = std::make_unique<SkeletalAnimationImpl>(std::move(ozzResult).value());
        return std::make_shared<SkeletalAnimation>(std::move(impl));
    }

    AssimpSkeletonLoader::AssimpSkeletonLoader(IDataLoader& dataLoader) noexcept
        : _impl(std::make_unique<AssimpSkeletonLoaderImpl>(dataLoader))
    {
    }

    AssimpSkeletonLoader::~AssimpSkeletonLoader() noexcept = default;

    AssimpSkeletonLoader::Result AssimpSkeletonLoader::operator()(std::filesystem::path path) noexcept
    {
        return (*_impl)(path);
    }

    AssimpSkeletonFileImporterImpl::AssimpSkeletonFileImporterImpl(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    expected<AssimpSkeletonFileImporterImpl::OzzSkeleton, std::string> AssimpSkeletonFileImporterImpl::read(const std::filesystem::path& path, const nlohmann::json& config) const noexcept
    {
        auto assimpConfig = createAssimpSkeletonLoadConfig();
        assimpConfig.populateArmature = true;
        auto sceneResult = _assimpLoader.loadFromFile(path, assimpConfig);
        if (!sceneResult)
        {
            return unexpected<std::string>{"could not load assimp scene"};
        }
        AssimpOzzSkeletonConverter converter(*sceneResult.value());
        converter.setConfig(config);
        return converter.createSkeleton();
    }

    expected<AssimpSkeletonFileImporterImpl::Effect, std::string> AssimpSkeletonFileImporterImpl::prepare(const Input& input) noexcept
    {
        Effect effect;
        if (input.config.is_null())
        {
            return effect;
        }
        if (!_assimpLoader.supports(input.path.string()))
        {
            return effect;
        }

        auto outputPath = input.getOutputPath(".ozz");
        effect.outputs.emplace_back(outputPath, true);
        return effect;
    }

    expected<void, std::string> AssimpSkeletonFileImporterImpl::operator()(const Input& input, Config& config) noexcept
    {
        auto result = read(input.path, input.config);
        if (!result)
        {
            return unexpected{ "failed to read skeleton: " + result.error() };
        }
        for (auto& out : config.outputStreams)
        {
            if (out)
            {
                AssimpOzzUtils::writeToStream(result.value(), *out, _bufferSize);
            }
        }

        return {};
    }

    const std::string& AssimpSkeletonFileImporterImpl::getName() const noexcept
    {
        static const std::string name("skeleton");
        return name;
    }

    AssimpSkeletonFileImporter::AssimpSkeletonFileImporter() noexcept
        : _impl(std::make_unique<AssimpSkeletonFileImporterImpl>())
    {
    }

    AssimpSkeletonFileImporter::~AssimpSkeletonFileImporter() noexcept = default;

    const std::string& AssimpSkeletonFileImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    expected<AssimpSkeletonFileImporter::Effect, std::string> AssimpSkeletonFileImporter::prepare(const Input& input) noexcept
    {
        return _impl->prepare(input);
    }

    expected<void, std::string> AssimpSkeletonFileImporter::operator()(const Input& input, Config& config) noexcept
    {
        return (*_impl)(input, config);
    }

    AssimpSkeletalAnimationFileImporterImpl::AssimpSkeletalAnimationFileImporterImpl(size_t bufferSize) noexcept
        : _bufferSize(bufferSize)
    {
    }

    void AssimpSkeletalAnimationFileImporterImpl::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _log = log;
    }

    std::filesystem::path AssimpSkeletalAnimationFileImporterImpl::getOutputPath(const Input& input, const std::string& animName, const std::string& outputPath) noexcept
    {
        auto fanimName = animName;

        std::optional<size_t> matchIndex;
        if(input.config.contains(_useFileMatchAsNameJsonKey))
        {
            matchIndex = input.config[_useFileMatchAsNameJsonKey].get<size_t>();
        }
        else if (input.dirConfig.contains(_useFileMatchAsNameJsonKey))
        {
            matchIndex = input.dirConfig[_useFileMatchAsNameJsonKey].get<size_t>();
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

    std::filesystem::path AssimpSkeletalAnimationFileImporterImpl::getOutputPath(const Input& input, const std::string& animName, const nlohmann::json& animConfig, const std::string& outputPath) noexcept
    {
        std::string fixedOutputPath = outputPath;
        if (animConfig.contains("outputPath"))
        {
            fixedOutputPath = animConfig["outputPath"];
        }
        return getOutputPath(input, animName, fixedOutputPath);
    }

    expected<AssimpSkeletalAnimationFileImporterImpl::OzzSkeleton, std::string> AssimpSkeletalAnimationFileImporterImpl::loadSkeleton(const std::filesystem::path& path, const nlohmann::json& config)
    {
        auto loadConfig = createAssimpSkeletonLoadConfig();
        loadConfig.populateArmature = true;
        auto assimpResult = _assimpLoader.loadFromFile(path, loadConfig);
		if (!assimpResult)
		{
			return unexpected<std::string>{assimpResult.error()};
		}
        AssimpOzzSkeletonConverter converter(*assimpResult.value());
        converter.setConfig(config);
        return converter.createSkeleton();
    }

    const std::string AssimpSkeletalAnimationFileImporterImpl::_skeletonJsonKey = "skeleton";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_outputPathJsonKey = "outputPath";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_animationsJsonKey = "animations";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_useFileMatchAsNameJsonKey = "useFileMatchAsName";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_optimizationJsonKey = "optimization";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_optimizationToleranceJsonKey = "tolerance";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_optimizationDistanceJsonKey = "distance";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_optimizationJointsJsonKey = "joints";
    const std::string AssimpSkeletalAnimationFileImporterImpl::_minKeyframeDurationJsonKey = "minKeyframeDuration";

    expected<AssimpSkeletalAnimationFileImporterImpl::Effect, std::string> AssimpSkeletalAnimationFileImporterImpl::prepare(const Input& input) noexcept
    {
        Effect effect;
        if (input.config.is_null())
        {
            return effect;
        }
        if (!_assimpLoader.supports(input.path.string()))
        {
            return effect;
        }
        _currentInput = input;
        auto assimpConfig = createAssimpSkeletonLoadConfig();
        auto assimpResult = _assimpLoader.loadFromFile(input.path, assimpConfig);
        if (!assimpResult)
        {
            return unexpected{ assimpResult.error() };
        }
        _currentScene = assimpResult.value();

        std::filesystem::path skelPath = input.path;
        if (input.config.contains(_skeletonJsonKey))
        {
            skelPath = input.path.parent_path() / input.config[_skeletonJsonKey];
        }
        else if (input.dirConfig.contains(_skeletonJsonKey))
        {
            skelPath = input.path.parent_path() / input.dirConfig[_skeletonJsonKey];
        }
        auto skelResult = loadSkeleton(skelPath, input.config);
        if (!skelResult)
        {
            return unexpected{ skelResult.error() };
        }
        _currentSkeleton = std::move(skelResult).value();

        nlohmann::json optJson;
        if (input.dirConfig.contains(_optimizationJsonKey))
        {
            optJson = input.dirConfig[_optimizationJsonKey];
        }
        if (input.config.contains(_optimizationJsonKey))
        {
            optJson.update(input.config[_optimizationJsonKey]);
        }
        if (!optJson.empty())
        {
            _currentOptimization = loadOptimizationSettings(optJson);
        }
        if (input.config.contains(_minKeyframeDurationJsonKey))
        {
            _currentMinKeyframeDuration = input.config[_minKeyframeDurationJsonKey];
        }
        else if (input.dirConfig.contains(_minKeyframeDurationJsonKey))
        {
            _currentMinKeyframeDuration = input.dirConfig[_minKeyframeDurationJsonKey];
        }

        std::string outputPath;
        if (input.config.contains(_outputPathJsonKey))
        {
            outputPath = input.config[_outputPathJsonKey];
        }

        AssimpOzzAnimationConverter converter(*_currentScene);
        auto allAnimationNames = converter.getAnimationNames();
        if (input.config.contains(_animationsJsonKey))
        {
            for (const auto& [animName, animConfig] : input.config[_animationsJsonKey].items())
            {
                if (StringUtils::containsGlobPattern(animName))
                {
                    auto regex = std::regex{ StringUtils::globToRegex(animName) };
                    for (auto& animName : allAnimationNames)
                    {
                        if (std::regex_match(animName, regex))
                        {
                            _currentAnimationNames.emplace_back(animName);
                            effect.outputs.emplace_back(getOutputPath(input, animName, animConfig, outputPath), true);
                        }
                    }
                }
                else
                {
                    _currentAnimationNames.emplace_back(animName);
                    effect.outputs.emplace_back(getOutputPath(input, animName, animConfig, outputPath), true);
                }
            }
        }
        else
        {
            _currentAnimationNames = allAnimationNames;
            for (auto& animName : _currentAnimationNames)
            {
                effect.outputs.emplace_back(getOutputPath(input, animName, outputPath), true);
            }
        }

        return effect;
    }

    expected<void, std::string> AssimpSkeletalAnimationFileImporterImpl::operator()(const Input& input, Config& config) noexcept
    {
        nlohmann::json optimize;
        auto itr = input.dirConfig.find(_optimizationJsonKey);
        if (itr != input.dirConfig.end())
        {
            optimize = *itr;
        }
        itr = input.config.find(_optimizationJsonKey);
        if (itr != input.config.end())
        {
            optimize.update(*itr);
        }

        size_t i = 0;
        for (auto& out : config.outputStreams)
        {
            if (out)
            {
                auto& animName = _currentAnimationNames[i];
                auto animResult = readAnimation(input.path, animName);
                if (!animResult)
                {
                    return unexpected<std::string>{ fmt::format("failed to read animation {}: {}", animName, animResult.error()) };
                }
                AssimpOzzUtils::writeToStream(animResult.value(), *out, _bufferSize);
            }
            ++i;
        }

        return {};
    }

    AssimpSkeletalAnimationFileImporterImpl::OptimizationSettings AssimpSkeletalAnimationFileImporterImpl::loadOptimizationSettings(const nlohmann::json& config) noexcept
    {
        auto readElement = [](const nlohmann::json& config)
        {
            OptimizationSettings::Element elm;
            if (config.contains(_optimizationToleranceJsonKey))
            {
                elm.tolerance = config[_optimizationToleranceJsonKey];
            }
            if (config.contains(_optimizationDistanceJsonKey))
            {
                elm.distance = config[_optimizationDistanceJsonKey];
            }
            return elm;
        };

        OptimizationSettings settings;
        settings.defaultElement = readElement(config);
        if (config.contains(_optimizationJointsJsonKey))
        {
            for (const auto& [elmName, elmConfig] : config[_optimizationJointsJsonKey].items())
            {
                settings.elements[elmName] = readElement(elmConfig);
            }
        }
        return settings;
    }

    expected<AssimpSkeletalAnimationFileImporterImpl::OzzAnimation, std::string> AssimpSkeletalAnimationFileImporterImpl::readAnimation(const std::filesystem::path& path, const std::string& animationName) noexcept
    {
        AssimpOzzAnimationConverter converter(*_currentScene, _log);
        if (_currentSkeleton)
        {
            converter.setSkeleton(_currentSkeleton.value());
        }
        if (_currentOptimization)
        {
            converter.setOptimization(_currentOptimization.value());
        }
        if (_currentMinKeyframeDuration)
        {
            converter.setMinKeyframeDuration(_currentMinKeyframeDuration.value());
        }
        return converter.createAnimation(animationName);
    }

    const std::string& AssimpSkeletalAnimationFileImporterImpl::getName() const noexcept
    {
        static const std::string name("skeletal_animation");
        return name;
    }

    AssimpSkeletalAnimationFileImporter::AssimpSkeletalAnimationFileImporter() noexcept
        : _impl(std::make_unique<AssimpSkeletalAnimationFileImporterImpl>())
    {
    }

    AssimpSkeletalAnimationFileImporter::~AssimpSkeletalAnimationFileImporter() noexcept = default;

    void AssimpSkeletalAnimationFileImporter::setLogOutput(OptionalRef<std::ostream> log) noexcept
    {
        _impl->setLogOutput(log);
    }

    const std::string& AssimpSkeletalAnimationFileImporter::getName() const noexcept
    {
        return _impl->getName();
    }

    expected<AssimpSkeletalAnimationFileImporter::Effect, std::string> AssimpSkeletalAnimationFileImporter::prepare(const Input& input) noexcept
    {
        return _impl->prepare(input);
    }

    expected<void, std::string> AssimpSkeletalAnimationFileImporter::operator()(const Input& input, Config& config) noexcept
    {
        return (*_impl)(input, config);
    }

    bool AssimpOzzImporter::Load(const char* filename)
    {
        _path = filename;
        auto assimpConfig = createAssimpSkeletonLoadConfig();
        assimpConfig.populateArmature = true;
        auto assimpResult = _assimpLoader.loadFromFile(_path, assimpConfig);
        if (!assimpResult)
        {
            return false;
        }
		_assimpScene = assimpResult.value();
        return true;
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
        auto result = converter.update(animationName, *animation);
        if (!result)
        {
            StreamUtils::log(result.error(), true);
            return false;
        }
        return true;
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

    AssimpArmatureDefinitionConverter::AssimpArmatureDefinitionConverter(const aiMesh& assimpMesh, Definition& def) noexcept
        : _assimpMesh{ assimpMesh }
        , _def{ def }
    {
    }

    AssimpArmatureDefinitionConverter& AssimpArmatureDefinitionConverter::setBoneNames(const std::vector<std::string>& names) noexcept
    {
        _boneNames.clear();
        for (auto& name : names)
        {
            _boneNames.emplace(name, name);
        }
        return *this;
    }

    AssimpArmatureDefinitionConverter& AssimpArmatureDefinitionConverter::setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept
    {
        _boneNames = names;
        return *this;
    }

    expected<void, std::string> AssimpArmatureDefinitionConverter::operator()() noexcept
    {
        for (size_t i = 0; i < _assimpMesh.mNumBones; ++i)
        {
            auto bone = _assimpMesh.mBones[i];
            auto boneName = AssimpUtils::getString(bone->mName);
            if (!_boneNames.empty())
            {
                auto itr = std::find_if(_boneNames.begin(), _boneNames.end(),
                    [&boneName](auto& elm) { return elm.first == boneName; });
                if (itr == _boneNames.end())
                {
                    continue;
                }
                boneName = itr->second;
            }
            auto& joint = *_def.add_joints();
            joint.set_name(boneName);
            *joint.mutable_inverse_bind_pose() = convert<protobuf::Mat4>(AssimpUtils::convert(bone->mOffsetMatrix));
        }
        return {};
    }
}