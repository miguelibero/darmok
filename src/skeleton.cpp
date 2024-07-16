#include <darmok/skeleton.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/transform.hpp>
#include <darmok/render.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/string.hpp>
#include <darmok/model.hpp>
#include <darmok/text.hpp>
#include <stdexcept>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

// to allow serialization
#include <darmok/math.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>

namespace darmok
{
    SkeletonImpl& Skeleton::getImpl()
    {
        return *_impl;
    }

    const SkeletonImpl& Skeleton::getImpl() const
    {
        return *_impl;
    }

    SkeletalAnimationImpl& SkeletalAnimation::getImpl()
    {
        return *_impl;
    }

    const SkeletalAnimationImpl& SkeletalAnimation::getImpl() const
    {
        return *_impl;
    }
    
    RenderableSkeleton::RenderableSkeleton(const std::shared_ptr<Material>& mat, const std::shared_ptr<IMesh>& boneMesh) noexcept
        : _boneMesh(boneMesh)
        , _material(mat)
    {
        if (!_material)
        {
            _material = std::make_shared<Material>();
            _material->setColor(MaterialColorType::Diffuse, Colors::magenta());
        }
        if (!_boneMesh)
        {
            bgfx::VertexLayout layout;
            if (auto prog = _material->getProgram())
            {
                layout = prog->getVertexLayout();
            }
            else
            {
                layout = MeshData::getDefaultVertexLayout();
            }
            _boneMesh = MeshData(Line(), LineMeshType::Diamond).createMesh(layout);
        }
    }

    RenderableSkeleton& RenderableSkeleton::setFont(const std::shared_ptr<IFont>& font) noexcept
    {
        _font = font;
        return *this;
    }
   
    void RenderableSkeleton::update(Scene& scene, const std::unordered_map<std::string, glm::mat4>& boneMatrixes) noexcept
    {
        for (auto itr = _boneTransforms.begin(); itr != _boneTransforms.end(); )
        {
            if (!boneMatrixes.contains(itr->first))
            {
                itr = _boneTransforms.erase(itr);
            }
            else
            {
                ++itr;
            }
        }
        if (boneMatrixes.empty())
        {
            return;
        }

        auto entity = scene.getEntity(*this);
        if (entity == entt::null)
        {
            return;
        }

        auto& trans = scene.getOrAddComponent<Transform>(entity);

        for (auto& [name, mtx] : boneMatrixes)
        {
            auto itr = _boneTransforms.find(name);
            if (itr != _boneTransforms.end())
            {
                itr->second->setLocalMatrix(mtx);
                continue;
            }

            auto boneEntity = scene.createEntity();
            scene.addComponent<Renderable>(boneEntity, _boneMesh, _material);
            auto& boneTrans = scene.addComponent<Transform>(boneEntity, mtx, trans);
            boneTrans.setName(name);
            _boneTransforms.emplace(name, boneTrans);

            // TODO: add border
            scene.addComponent<Text>(boneEntity, _font, name)
                .setColor(Colors::black())
                .setOrientation(TextOrientation::Center);
        }
    }

    void SkeletalAnimationSceneComponent::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
    }

    void SkeletalAnimationSceneComponent::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto animators = _scene->getRegistry().view<SkeletalAnimator>();
        std::vector<OptionalRef<RenderableSkeleton>> skeletons;
        for (auto [entity, anim] : animators.each())
        {
            anim.update(deltaTime);
            skeletons.clear();
            auto matrixes = anim.getBoneModelMatrixes();
            _scene->getComponentsInChildren<RenderableSkeleton>(entity, skeletons);
            for (auto& skel : skeletons)
            {
                skel->update(_scene.value(), matrixes);
            }
        }
    }

    void SkeletalAnimationCameraComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
        // TODO: maybe we should use u_model[X] but the bgfx API setTransform forces you to pass all at the same time
        _skinningUniform = bgfx::createUniform("u_skinning", bgfx::UniformType::Mat4, DARMOK_SKELETON_MAX_BONES);
    }

    void SkeletalAnimationCameraComponent::shutdown() noexcept
    {
        _scene.reset();
        _cam.reset();
        if (isValid(_skinningUniform))
        {
            bgfx::destroy(_skinningUniform);
            _skinningUniform.idx = bgfx::kInvalidHandle;
        }
    }

    OptionalRef<SkeletalAnimator> SkeletalAnimationCameraComponent::getAnimator(Entity entity) const noexcept
    {
        if (!_scene)
        {
            return nullptr;
        }
        return _scene->getComponentInParent<SkeletalAnimator>(entity);
    }

    void SkeletalAnimationCameraComponent::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) noexcept
    {
        auto animator = getAnimator(entity);
        if (!animator)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        auto skinnable = registry.try_get<Skinnable>(entity);
        if (!skinnable)
        {
            return;
        }

        auto armature = skinnable->getArmature();
        _skinning.clear();
        _skinning.push_back(glm::mat4(1));
        if (armature)
        {
            auto& joints = armature->getJoints();
            _skinning.reserve(joints.size() + 1);
            for (auto& joint : joints)
            {
                auto model = animator->getJointModelMatrix(joint.name);
                _skinning.push_back(model * joint.inverseBindPose);
            }
        }
        encoder.setUniform(_skinningUniform, &_skinning.front(), uint16_t(_skinning.size()));
    }

    tweeny::tween<float> SkeletalAnimatorTweenConfig::create() const noexcept
    {
        return tweeny::from(0.F).to(1.F).via(easing).during(duration * 1000);
    }

    void SkeletalAnimatorTweenConfig::readJson(const nlohmann::json& json)
    {
        if (json.contains("duration"))
        {
            duration = json["duration"];
        }
        if (json.contains("easing"))
        {
            easing = json["easing"];
        }
    }

    void SkeletalAnimatorAnimationConfig::readJson(const nlohmann::json& json)
    {
        if (json.contains("value"))
        {
            auto& v = json["value"];
            if (v.is_number())
            {
                blendPosition = glm::vec2(v);
            }
            if (v.is_array())
            {
                blendPosition = glm::vec2(v[0], v[1]);
            }
        }
        if (json.contains("animation"))
        {
            animation = json["animation"];
        }
    }

    float SkeletalAnimatorStateConfig::calcBlendWeight(const glm::vec2& pos, const glm::vec2& animPos)
    {
        std::vector<std::pair<glm::vec2, glm::vec2>> parts;
        parts.reserve(animations.size());
        if (blendType == SkeletalAnimatorBlendType::Directional)
        {
            auto lenAnim = glm::length(animPos);
            auto len = glm::length(pos);
            for (auto& anim : animations)
            {
                auto lenAnim2 = glm::length(anim.blendPosition);
                auto p = lenAnim + lenAnim2;
                auto a = glm::vec2((len - lenAnim) * p, glm::angle(pos, animPos));
                auto b = glm::vec2((lenAnim2 - lenAnim) * p, glm::angle(anim.blendPosition, animPos));
                parts.emplace_back(a, b);
            }
        }
        else
        {
            auto a = pos - animPos;
            for (auto& anim : animations)
            {
                auto b = anim.blendPosition - animPos;
                parts.emplace_back(a, b);
            }
        }
        
        auto w = bx::kFloatLargest;
        for (auto& [a, b] : parts)
        {
            auto f = 1.F - (glm::dot(a, b) / glm::length2(b));
            if (f < w)
            {
                w = f;
            }
        }

        return w;
    }

    std::vector<float> SkeletalAnimatorStateConfig::calcBlendWeights(const glm::vec2& pos)
    {
        // logic based on the Motion Interpolation explanation here
        // https://runevision.com/thesis/rune_skovbo_johansen_thesis.pdf
        // FreeFormDirectional: Gradient Bands in polar space
        // FreeFormCartesian: Gradiend band
        // direct: distance is weight
        std::vector<float> weights;
        weights.reserve(animations.size());

        for (auto& anim : animations)
        {
            auto w = calcBlendWeight(pos, anim.blendPosition);
            weights.push_back(w);
        }

        return weights;
    }

    SkeletalAnimatorBlendType SkeletalAnimatorStateConfig::getBlendType(const std::string_view name) noexcept
    {
        if (name == "directional")
        {
            return SkeletalAnimatorBlendType::Directional;
        }
        if (name == "cartesian")
        {
            return SkeletalAnimatorBlendType::Cartesian;
        }
        return SkeletalAnimatorBlendType::Cartesian;
    }

    void SkeletalAnimatorStateConfig::readJson(const nlohmann::json& json)
    {
        if (json.contains("elements"))
        {
            for (auto& elm : json["elements"])
            {
                AnimationConfig config;
                config.readJson(elm);
                animations.push_back(config);
            }
        }
        if (json.contains("animation"))
        {
            AnimationConfig config;
            config.readJson(json);
            animations.push_back(config);
        }
        if (json.contains("name"))
        {
            name = json["name"];
        }
        if (json.contains("threshold"))
        {
            threshold = json["threshold"];
        }
        if (json.contains("blend"))
        {
            blendType = getBlendType(json["blend"]);
        }
        if(json.contains("tween"))
        {
            tween.readJson(json["tween"]);
        }
    }

    std::pair<std::string, std::string> SkeletalAnimatorTransitionConfig::readJsonKey(std::string_view key)
    {
        const char sep = '>';
        auto pos = key.find(sep);
        if (pos == std::string::npos)
        {
            return std::pair<std::string, std::string>("", key);
        }
        return std::pair<std::string, std::string>(key.substr(0, pos), key.substr(pos + 1));
    }

    void SkeletalAnimatorTransitionConfig::readJson(const nlohmann::json& json)
    {
        tween.readJson(json);
        if (json.contains("offset"))
        {
            offset = json["offset"];
        }
    }

    std::string SkeletalAnimatorConfig::getAnimationName(std::string_view key) const noexcept
    {
        if (_animationPattern.empty())
        {
            return std::string(key);
        }
        std::string v = _animationPattern;
        StringUtils::replace(v, "*", key);
        return v;
    }

    SkeletalAnimatorConfig::AnimationMap SkeletalAnimatorConfig::loadAnimations(ISkeletalAnimationLoader& loader) const
    {
        AnimationMap anims;
        for (auto& elm : _states)
        {
            for (auto& anim : elm.second.animations)
            {
                anims.emplace(anim.animation, loader(getAnimationName(anim.animation)));
            }
        }
        return anims;
    }

    void SkeletalAnimatorConfig::readJson(const nlohmann::json& json)
    {
        if (json.contains("animationNamePattern"))
        {
            _animationPattern = json["animationNamePattern"];
        }
        if (json.contains("states"))
        {
            for (auto& stateJson : json["states"].items())
            {
                StateConfig config;
                config.name = stateJson.key();
                config.readJson(stateJson.value());
                addState(config);
            }
        }
        if (json.contains("transitions"))
        {
            for (auto& transitionJson : json["transitions"].items())
            {
                TransitionConfig config;
                config.readJson(transitionJson.value());
                auto [src, dst] = TransitionConfig::readJsonKey(transitionJson.key());
                addTransition(src, dst, config);
            }
        }
    }

    SkeletalAnimatorConfig& SkeletalAnimatorConfig::addState(const StateConfig& config) noexcept
    {
        if (config.name.empty())
        {
            StateConfig fixedConfig = config;
            size_t i = 0;
            while (fixedConfig.name.empty() && i < fixedConfig.animations.size())
            {
                auto& anim = fixedConfig.animations[i++].animation;
                fixedConfig.name = anim;
            }
            _states.emplace(fixedConfig.name, fixedConfig);
        }
        else
        {
            _states.emplace(config.name, config);
        }
        
        return *this;
    }

    SkeletalAnimatorConfig& SkeletalAnimatorConfig::addState(std::string_view animation, std::string_view name) noexcept
    {
        return addState(StateConfig{ std::string(name), { { std::string(animation) } } });
    }

    SkeletalAnimatorConfig& SkeletalAnimatorConfig::addTransition(std::string_view src, std::string_view dst, const TransitionConfig& config) noexcept
    {
        TransitionKey key(src, dst);
        _transitions.emplace(key, config);
        return *this;
    }

    std::optional<const SkeletalAnimatorConfig::StateConfig> SkeletalAnimatorConfig::getState(std::string_view name) const noexcept
    {
        auto itr = std::find_if(_states.begin(), _states.end(), [name](auto& elm) { return elm.first == name; });
        if (itr == _states.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    std::optional<const SkeletalAnimatorConfig::TransitionConfig> SkeletalAnimatorConfig::getTransition(std::string_view src, std::string_view dst) const noexcept
    {
        TransitionKey key(src, dst);
        auto itr = _transitions.find(key);
        if (itr == _transitions.end())
        {
            TransitionKey key("", dst);
            itr = _transitions.find(key);
        }
        if (itr == _transitions.end())
        {
            return std::nullopt;
        }
        return itr->second;
    }

    SkeletalAnimatorConfigLoader::SkeletalAnimatorConfigLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    SkeletalAnimatorConfig SkeletalAnimatorConfigLoader::operator()(std::string_view name)
    {
        auto data = _dataLoader(name);
        SkeletalAnimatorConfig config;
        auto ext = StringUtils::getFileExt(name);
        if (ext == ".json")
        {
            config.readJson(nlohmann::json::parse(data.stringView()));
        }
        else
        {
            DataInputStream::read(data, config);
        }
        return config;
    }

    std::vector<std::filesystem::path> SkeletalAnimatorConfigImporter::getOutputs(const Input& input) noexcept
    {
        std::vector<std::filesystem::path> outputs;
        auto ext = StringUtils::getFileExt(input.path.filename().string());
        if (input.config.is_null())
        {
            if (ext != ".animator.json" && ext != ".animator.bin")
            {
                return outputs;
            }
        }
        if (input.config.contains("outputPath"))
        {
            outputs.push_back(input.config["outputPath"]);
            return outputs;
        }

        auto stem = std::string(StringUtils::getFileStem(input.path.filename().string()));
        outputs.push_back(input.path.parent_path() / (stem + ".bin"));
        return outputs;
    }

    std::ofstream SkeletalAnimatorConfigImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& outputPath)
    {
        auto ext = StringUtils::getFileExt(input.path.filename().string());
        if (ext == ".json")
        {
            return std::ofstream(outputPath);
        }
        return std::ofstream(outputPath, std::ios::binary);
    }


    SkeletalAnimatorConfig SkeletalAnimatorConfigImporter::read(const std::filesystem::path& path) const
    {
        SkeletalAnimatorConfig config;
        auto ext = StringUtils::getFileExt(path.filename().string());
        if (ext == ".json")
        {
            std::ifstream is(path);
            config.readJson(nlohmann::json::parse(is));
        }
        return config;
    }

    void SkeletalAnimatorConfigImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
    {
        auto ext = StringUtils::getFileExt(input.path.filename().string());
        auto config = read(input.path);
        if (ext == ".json")
        {
            return;
        }
        cereal::BinaryOutputArchive archive(out);
        archive(config);
    }

    const std::string& SkeletalAnimatorConfigImporter::getName() const noexcept
    {
        static const std::string name("animator");
        return name;
    }
}