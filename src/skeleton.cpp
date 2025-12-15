#include <darmok/skeleton.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/string.hpp>
#include <darmok/text.hpp>
#include <darmok/shape.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/asset_pack.hpp>

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>


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

    expected<Mesh, std::string> RenderableSkeleton::createBoneMesh(const std::optional<bgfx::VertexLayout>& layout) noexcept
    {
        bgfx::VertexLayout flayout = layout ? *layout : MeshData::getDefaultVertexLayout();
        const Line line{ glm::vec3{0}, glm::vec3{1, 0, 0} };
        return MeshData{ line, Mesh::Definition::Arrow }.createMesh(flayout);
    }
    
    RenderableSkeleton::RenderableSkeleton(const std::shared_ptr<Mesh>& boneMesh, const std::shared_ptr<Material>& mat) noexcept
        : _boneMesh{ boneMesh }
        , _material{ mat}
    {
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

        for (const auto& [name, mtx] : boneMatrixes)
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

            auto textEntity = scene.createEntity();
            auto& textTrans = scene.addComponent<Transform>(textEntity);
            textTrans.setParent(boneTrans);
            textTrans.setScale(glm::vec3(5.F));

            // TODO: add border
            scene.addComponent<Text>(textEntity, _font, name)
                .setColor(Colors::black())
                .setOrientation(TextOrientation::Center);
        }
    }

    DataSkeletalAnimatorDefinitionLoader::DataSkeletalAnimatorDefinitionLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    DataSkeletalAnimatorDefinitionLoader::Result DataSkeletalAnimatorDefinitionLoader::operator()(std::filesystem::path path) noexcept
    {
        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
            return unexpected{ dataResult.error() };
        }
        auto format = protobuf::getPathFormat(path);
        auto res = std::make_shared<Resource>(SkeletalAnimator::createDefinition());

		expected<void, std::string> result;
        if(format == protobuf::Format::Json)
        {
            auto jsonResult = StringUtils::parseJson(dataResult.value().toString());
            if(!jsonResult)
            {
                return unexpected{ jsonResult.error() };
			}
            result = SkeletalAnimatorDefinitionWrapper{ *res }.read(*jsonResult);
		}
        else
        {
            DataInputStream input{ dataResult.value() };
            result = protobuf::read(*res, input, format);
        }
        if (!result)
        {
            return unexpected{ result.error() };
        }
        return res;
    }


    expected<void, std::string> SkeletalAnimationSceneComponent::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
		return {};
    }

    expected<void, std::string> SkeletalAnimationSceneComponent::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return unexpected<std::string>{"scene not loaded"};
        }
        auto entities = _scene->getUpdateEntities<SkeletalAnimator>();
        std::vector<OptionalRef<RenderableSkeleton>> skeletons;
        for (auto entity : entities)
        {
            auto& anim = _scene->getComponent<SkeletalAnimator>(entity).value();
            auto updateResult = anim.update(deltaTime);
            if (!updateResult)
            {
                return updateResult;
            }
            skeletons.clear();
            auto matrixes = anim.getJointModelMatrixes();
            _scene->getComponentsInChildren<RenderableSkeleton>(entity, skeletons);
            for (auto& skel : skeletons)
            {
                skel->update(_scene.value(), matrixes);
            }
        }
        return {};
    }

    SkeletalAnimationSceneComponent::Definition SkeletalAnimationSceneComponent::createDefinition() noexcept
    {
        Definition def;
		return def;
    }

    expected<void, std::string> SkeletalAnimationSceneComponent::load(const Definition& def) noexcept
    {
		return {};
    }

    expected<void, std::string> SkeletalAnimationRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
        // TODO: maybe we should use u_model[X] but the bgfx API setTransform forces you to pass all at the same time
        _skinningUniform = bgfx::createUniform("u_skinning", bgfx::UniformType::Mat4, DARMOK_SKELETON_MAX_BONES);
        return {};
    }

    expected<void, std::string> SkeletalAnimationRenderComponent::shutdown() noexcept
    {
        _scene.reset();
        _cam.reset();
        if (isValid(_skinningUniform))
        {
            bgfx::destroy(_skinningUniform);
            _skinningUniform.idx = bgfx::kInvalidHandle;
        }
        return {};
    }

    OptionalRef<SkeletalAnimator> SkeletalAnimationRenderComponent::getAnimator(Entity entity) const noexcept
    {
        if (!_scene)
        {
            return nullptr;
        }
        return _scene->getComponentInParent<SkeletalAnimator>(entity);
    }

    expected<void, std::string> SkeletalAnimationRenderComponent::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        auto skinnable = _scene->getComponent<Skinnable>(entity);
        if (!skinnable)
        {
            return {};
        }
        auto animator = getAnimator(entity);
        if (!animator)
        {
            return {};
        }

        auto armature = skinnable->getArmature();
        _skinning.clear();
        _skinning.push_back(glm::mat4{ 1 });
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
        return {};
    }

    SkeletalAnimationRenderComponent::Definition SkeletalAnimationRenderComponent::createDefinition() noexcept
    {
        Definition def;
        return def;
    }

    expected<void, std::string> SkeletalAnimationRenderComponent::load(const Definition& def) noexcept
    {
        return {};
    }

    ConstSkeletalAnimatorTweenDefinitionWrapper::ConstSkeletalAnimatorTweenDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
    }

    float ConstSkeletalAnimatorTweenDefinitionWrapper::calcTween(float position) const noexcept
    {
        return Easing::apply(static_cast<Easing::Type>(_def->easing()), position, 0.F, 1.F);
    }

    SkeletalAnimatorTweenDefinitionWrapper::SkeletalAnimatorTweenDefinitionWrapper(Definition& def) noexcept
        : ConstSkeletalAnimatorTweenDefinitionWrapper( def )
		, _def{ def }
    {
    }

    expected<void, std::string> SkeletalAnimatorTweenDefinitionWrapper::read(const nlohmann::json& json) noexcept
    {
        auto itr = json.find("duration");
        if (itr != json.end())
        {
            _def->set_duration(itr->get<float>());
        }
        itr = json.find("easing");
        if (itr != json.end())
        {
            auto val = itr->get<std::string>();
            auto easing = StringUtils::readEnum<protobuf::Easing::Type>(val);
            if (!easing)
            {
                return unexpected{ "invalid easing type" };
            }
            _def->set_easing(*easing);
        }
        return {};
    }

    ConstSkeletalAnimatorStateDefinitionWrapper::ConstSkeletalAnimatorStateDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
    }

    float ConstSkeletalAnimatorStateDefinitionWrapper::calcBlendWeight(const glm::vec2& pos, const glm::vec2& animPos) const noexcept
    {
        std::vector<std::pair<glm::vec2, glm::vec2>> parts;
        parts.reserve(_def->animations_size());
        if (_def->blend() == SkeletalAnimator::StateDefinition::Directional)
        {
            auto lenAnim = glm::length(animPos);
            auto len = glm::length(pos);
            for (auto& anim : _def->animations())
            {
                auto blendPos = convert<glm::vec2>(anim.blend_position());
                auto lenAnim2 = glm::length(blendPos);
                auto p = lenAnim + lenAnim2;
                glm::vec2 a{ (len - lenAnim) * p, glm::angle(pos, animPos) };
                glm::vec2 b{ (lenAnim2 - lenAnim) * p, glm::angle(blendPos, animPos) };
                parts.emplace_back(a, b);
            }
        }
        else
        {
            auto a = pos - animPos;
            for (auto& anim : _def->animations())
            {
                auto b = convert<glm::vec2>(anim.blend_position()) - animPos;
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

    std::vector<float> ConstSkeletalAnimatorStateDefinitionWrapper::calcBlendWeights(const glm::vec2& pos) const noexcept
    {
        // logic based on the Motion Interpolation explanation here
        // https://runevision.com/thesis/rune_skovbo_johansen_thesis.pdf
        // FreeFormDirectional: Gradient Bands in polar space
        // FreeFormCartesian: Gradiend band
        // direct: distance is weight
        std::vector<float> weights;
        weights.reserve(_def->animations_size());

        for (auto& anim : _def->animations())
        {
            auto w = calcBlendWeight(pos, convert<glm::vec2>(anim.blend_position()));
            weights.push_back(w);
        }

        return weights;
    }

    SkeletalAnimatorStateDefinitionWrapper::SkeletalAnimatorStateDefinitionWrapper(Definition& def) noexcept
        : ConstSkeletalAnimatorStateDefinitionWrapper(def)
		, _def{ def }
    {
    }


    std::optional<SkeletalAnimatorStateDefinitionWrapper::Definition::BlendType> SkeletalAnimatorStateDefinitionWrapper::getBlendType(const std::string& name) noexcept
    {
        Definition::BlendType val;
        if (!Definition::BlendType_Parse(name, &val))
        {
            return std::nullopt;
        }
        return val;
    }

    expected<void, std::string> SkeletalAnimatorStateDefinitionWrapper::read(const nlohmann::json& json) noexcept
    {
        auto itr = json.find("elements");
        if (itr != json.end())
        {
            for (auto& elm : *itr)
            {
                auto& anim = *_def->add_animations();
                auto result = SkeletalAnimatorAnimationDefinitionWrapper{ anim }.read(elm);
                if (!result)
                {
                    return result;
                }
            }
        }
        itr = json.find("animation");
        if (itr != json.end())
        {
            auto& anim = *_def->add_animations();
            expected<void, std::string> result;
            if(itr->is_string())
            {
                result = SkeletalAnimatorAnimationDefinitionWrapper{ anim }.read(json);
            }
            else
            {
                result = SkeletalAnimatorAnimationDefinitionWrapper{ anim }.read(*itr);
            }
            if (!result)
            {
                return result;
            }
        }
        itr = json.find("name");
        if (itr != json.end())
        {
            _def->set_name(itr->get<std::string>());
        }
        itr = json.find("threshold");
        if (itr != json.end())
        {
            _def->set_threshold(itr->get<float>());
        }
        itr = json.find("blend");
        if (itr != json.end())
        {
            auto blend = getBlendType(itr->get<std::string>());
            if (!blend)
            {
                return unexpected{ "invalid blend type" };
            }
            _def->set_blend(*blend);
        }
        itr = json.find("tween");
        if (itr != json.end())
        {
            auto result = SkeletalAnimatorTweenDefinitionWrapper{ *_def->mutable_tween() }.read(*itr);
            if (!result)
            {
                return result;
            }
        }
        itr = json.find("nextState");
        if (itr != json.end())
        {
            _def->set_next_state(itr->get<std::string>());
        }
        itr = json.find("speed");
        if (itr != json.end())
        {
            _def->set_speed(itr->get<float>());
        }
        else
        {
            _def->set_speed(1.f);
        }
        return {};
    }

    ConstSkeletalAnimatorAnimationDefinitionWrapper::ConstSkeletalAnimatorAnimationDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
    }

    SkeletalAnimatorAnimationDefinitionWrapper::SkeletalAnimatorAnimationDefinitionWrapper(Definition& def) noexcept
		: ConstSkeletalAnimatorAnimationDefinitionWrapper(def)
		, _def{ def }
    {
    }

    expected<void, std::string> SkeletalAnimatorAnimationDefinitionWrapper::read(const nlohmann::json& json) noexcept
    {
        if (json.is_string())
        {
            _def->set_name(json.get<std::string>());
            _def->set_loop(true);
            return {};
        }
        auto itr = json.find("value");
        if (itr != json.end())
        {
            auto vec = itr->get<glm::vec2>();
            *_def->mutable_blend_position() = convert<protobuf::Vec2>(vec);
        }
        itr = json.find("animation");
        if (itr != json.end())
        {
            _def->set_name(itr->get<std::string>());
        }
        itr = json.find("loop");
        if (itr != json.end())
        {
            _def->set_loop(itr->get<bool>());
        }
        else
        {
            _def->set_loop(true);
        }
        return {};
    }

    ConstSkeletalAnimatorTransitionDefinitionWrapper::ConstSkeletalAnimatorTransitionDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
    }

    SkeletalAnimatorTransitionDefinitionWrapper::SkeletalAnimatorTransitionDefinitionWrapper(Definition& def) noexcept
        : ConstSkeletalAnimatorTransitionDefinitionWrapper(def)
        , _def{ def }
    {
    }

    expected<void, std::string> SkeletalAnimatorTransitionDefinitionWrapper::read(const nlohmann::json& json) noexcept
    {
        auto result = SkeletalAnimatorTweenDefinitionWrapper{ *_def->mutable_tween() }.read(json);
        if (!result)
        {
            return result;
        }
        auto itr = json.find("offset");
        if (itr != json.end())
        {
            _def->set_offset(itr->get<float>());
        }
        return {};
    }
    
    ConstSkeletalAnimatorDefinitionWrapper::ConstSkeletalAnimatorDefinitionWrapper(const Definition& def) noexcept
        : _def{ def }
    {
    }

    ConstSkeletalAnimatorDefinitionWrapper::AnimationMap ConstSkeletalAnimatorDefinitionWrapper::loadAnimations(ISkeletalAnimationLoader& loader) const noexcept
    {
        AnimationMap anims;
        auto getAnimationPath = [this](const auto& key) -> std::filesystem::path
        {
            auto& pattern = _def->animation_pattern();
            if (pattern.empty())
            {
                return std::string{ key };
            }
            std::string v = pattern;
            StringUtils::replace(v, "*", key);
            return v;
        };
        for (auto& state : _def->states())
        {
            for (auto& anim : state.animations())
            {
                auto& name = anim.name();
                auto result = loader(getAnimationPath(name));
                if (result)
                {
                    anims.emplace(name, result.value());
                }
            }
        }
        return anims;
    }

    OptionalRef<const ConstSkeletalAnimatorDefinitionWrapper::StateDefinition> ConstSkeletalAnimatorDefinitionWrapper::getState(std::string_view name) const noexcept
    {
        auto itr = std::find_if(_def->states().begin(), _def->states().end(),
            [name](auto& stateDef) { return stateDef.name() == name; });
        if (itr == _def->states().end())
        {
            return nullptr;
        }
        return *itr;
    }

    OptionalRef<const ConstSkeletalAnimatorDefinitionWrapper::TransitionDefinition> ConstSkeletalAnimatorDefinitionWrapper::getTransition(std::string_view src, std::string_view dst) const noexcept
    {
        auto& ts = _def->transitions();
        auto itr = std::find_if(ts.begin(), ts.end(), [&](auto& trans) { return trans.src_state() == src && trans.dst_state() == dst; });
		if (itr != ts.end())
		{
			return *itr;
		}
        itr = std::find_if(ts.begin(), ts.end(), [&](auto& trans) { return trans.src_state() == src && trans.dst_state().empty(); });
        if (itr != ts.end())
        {
            return *itr;
        }
        itr = std::find_if(ts.begin(), ts.end(), [&](auto& trans) { return trans.src_state().empty() && trans.dst_state() == dst; });
        if (itr != ts.end())
        {
            return *itr;
        }
        itr = std::find_if(ts.begin(), ts.end(), [&](auto& trans) { return trans.src_state().empty() && trans.dst_state().empty(); });
        if (itr != ts.end())
        {
            return *itr;
        }
		return nullptr;
    }

    SkeletalAnimatorDefinitionWrapper::SkeletalAnimatorDefinitionWrapper(Definition& def) noexcept
		: ConstSkeletalAnimatorDefinitionWrapper{ def }
        , _def{ def }
    {
	}

    std::pair<std::string_view, std::string_view> SkeletalAnimatorDefinitionWrapper::parseTransitionKey(std::string_view key) noexcept
    {
        const char sep = '>';
        auto pos = key.find(sep);
        if (pos == std::string::npos)
        {
            return { {}, key };
        }
        return { key.substr(0, pos), key.substr(pos + 1) };
    }

    expected<void, std::string> SkeletalAnimatorDefinitionWrapper::read(const nlohmann::json& json) noexcept
    {
        auto itr = json.find("animationNamePattern");
        if (itr != json.end())
        {
            _def->set_animation_pattern(itr->get<std::string>());
        }
        itr = json.find("states");
        if (itr != json.end())
        {
            for (auto& [jsonKey, jsonVal] : itr->items())
            {
                auto& state = *_def->add_states();
                state.set_name(jsonKey);
                auto result = SkeletalAnimatorStateDefinitionWrapper{ state }.read(jsonVal);
                if (!result)
                {
                    return result;
                }
            }
        }
        itr = json.find("transitions");
        if (itr != json.end())
        {
            for (auto& [jsonKey, jsonVal] : itr->items())
            {
                auto [src, dst] = parseTransitionKey(jsonKey);
                auto& trans = *_def->add_transitions();
                auto result = SkeletalAnimatorTransitionDefinitionWrapper{ trans }.read(jsonVal);
                if (!result)
                {
                    return result;
                }
            }
        }
        return {};
    }

    SkeletalAnimatorDefinitionFileImporter::SkeletalAnimatorDefinitionFileImporter() noexcept
        : ProtobufFileImporter<ISkeletalAnimatorDefinitionLoader>{ _loader, "skeletal-animator" }
        , _loader{ _dataLoader }
        {
        }

    Armature::Armature(const Definition& def) noexcept
    {
        _joints.reserve(def.joints_size());
        for (auto& joint : def.joints())
        {
            _joints.emplace_back(joint.name(), convert<glm::mat4>(joint.inverse_bind_pose()));
        }
    }

    Armature::Armature(const std::vector<ArmatureJoint>& joints) noexcept
        : _joints{ joints }
    {
    }

    Armature::Armature(std::vector<ArmatureJoint>&& joints) noexcept
        : _joints{ std::move(joints) }
    {
    }

    const std::vector<ArmatureJoint>& Armature::getJoints() const noexcept
    {
        return _joints;
    }

    Armature::Definition Armature::createDefinition() noexcept
    {
        return {};
    }

    Skinnable::Skinnable(const std::shared_ptr<Armature>& armature) noexcept
        : _armature{ armature }
    {
    }

    std::shared_ptr<Armature> Skinnable::getArmature() const noexcept
    {
        return _armature;
    }

    void Skinnable::setArmature(const std::shared_ptr<Armature>& armature) noexcept
    {
        _armature = armature;
    }

    expected<void, std::string> Skinnable::load(const Definition& def, IComponentLoadContext& ctxt)
    {
        auto result = ctxt.getAssets().getArmatureLoader()(def.armature_path());
        if (!result)
        {
            return unexpected{ result.error() };
        }
        setArmature(result.value());
        return {};
    }

    Skinnable::Definition Skinnable::createDefinition() noexcept
    {
        return {};
    }
}