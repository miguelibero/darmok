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
    
    RenderableSkeleton::RenderableSkeleton(const std::shared_ptr<Material>& mat, const std::shared_ptr<IMesh>& boneMesh) noexcept
        : _boneMesh{ boneMesh }
        , _material{ mat }
    {
        if (!_material)
        {
            _material = std::make_shared<Material>();
        }
        if (!_boneMesh)
        {
            bgfx::VertexLayout layout;
            if (auto prog = _material->program)
            {
                layout = prog->getVertexLayout();
            }
            else
            {
                layout = MeshData::getDefaultVertexLayout();
            }
            const Line line{ glm::vec3{0}, glm::vec3{1, 0, 0} };
            _boneMesh = MeshData{ line, Mesh::Definition::Arrow }.createMesh(layout);
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

    DataSkeletalAnimatorDefinitionLoader::DataSkeletalAnimatorDefinitionLoader(IDataLoader& dataLoader)
        : _dataLoader(dataLoader)
    {
    }

    DataSkeletalAnimatorDefinitionLoader::Result DataSkeletalAnimatorDefinitionLoader::operator()(std::filesystem::path path)
    {
        auto dataResult = _dataLoader(path);
        if (!dataResult)
        {
            return unexpected{ dataResult.error() };
        }
        auto format = protobuf::getFormat(path);
        auto res = std::make_shared<Resource>();

		expected<void, std::string> result;
        if(format == protobuf::Format::Json)
        {
            auto json = nlohmann::json::parse(dataResult.value().toString());
			result = SkeletalAnimatorUtils::read(*res, json);
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
        auto entities = _scene->getUpdateEntities<SkeletalAnimator>();
        std::vector<OptionalRef<RenderableSkeleton>> skeletons;
        for (auto entity : entities)
        {
            auto& anim = _scene->getComponent<SkeletalAnimator>(entity).value();
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

    void SkeletalAnimationRenderComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
        // TODO: maybe we should use u_model[X] but the bgfx API setTransform forces you to pass all at the same time
        _skinningUniform = bgfx::createUniform("u_skinning", bgfx::UniformType::Mat4, DARMOK_SKELETON_MAX_BONES);
    }

    void SkeletalAnimationRenderComponent::shutdown() noexcept
    {
        _scene.reset();
        _cam.reset();
        if (isValid(_skinningUniform))
        {
            bgfx::destroy(_skinningUniform);
            _skinningUniform.idx = bgfx::kInvalidHandle;
        }
    }

    OptionalRef<SkeletalAnimator> SkeletalAnimationRenderComponent::getAnimator(Entity entity) const noexcept
    {
        if (!_scene)
        {
            return nullptr;
        }
        return _scene->getComponentInParent<SkeletalAnimator>(entity);
    }

    void SkeletalAnimationRenderComponent::beforeRenderEntity(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        auto animator = getAnimator(entity);
        if (!animator)
        {
            return;
        }
        auto skinnable = _scene->getComponent<Skinnable>(entity);
        if (!skinnable)
        {
            return;
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
    }

    float SkeletalAnimatorUtils::calcTween(const TweenDefinition& tween, float position)
    {
        return Easing::apply(static_cast<Easing::Type>(tween.easing()), position, 0.F, 1.F);
    }

    float SkeletalAnimatorUtils::calcBlendWeight(const StateDefinition& state, const glm::vec2& pos, const glm::vec2& animPos)
    {
        std::vector<std::pair<glm::vec2, glm::vec2>> parts;
        parts.reserve(state.animations_size());
        if (state.blend() == SkeletalAnimator::StateDefinition::Directional)
        {
            auto lenAnim = glm::length(animPos);
            auto len = glm::length(pos);
            for (auto& anim : state.animations())
            {
                auto blendPos = protobuf::convert(anim.blend_position());
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
            for (auto& anim : state.animations())
            {
                auto b = protobuf::convert(anim.blend_position()) - animPos;
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

    std::vector<float> SkeletalAnimatorUtils::calcBlendWeights(const StateDefinition& state, const glm::vec2& pos)
    {
        // logic based on the Motion Interpolation explanation here
        // https://runevision.com/thesis/rune_skovbo_johansen_thesis.pdf
        // FreeFormDirectional: Gradient Bands in polar space
        // FreeFormCartesian: Gradiend band
        // direct: distance is weight
        std::vector<float> weights;
        weights.reserve(state.animations_size());

        for (auto& anim : state.animations())
        {
            auto w = calcBlendWeight(state, pos, protobuf::convert(anim.blend_position()));
            weights.push_back(w);
        }

        return weights;
    }

    SkeletalAnimationMap SkeletalAnimatorUtils::loadAnimations(const Definition& animator, ISkeletalAnimationLoader& loader)
    {
        AnimationMap anims;
        auto getAnimationName = [&animator](const auto& key)
        {
            auto& pattern = animator.animation_pattern();
            if (pattern.empty())
            {
                return std::string{ key };
            }
            std::string v = pattern;
            StringUtils::replace(v, "*", key);
            return v;
        };
        for (auto& state : animator.states())
        {
            for (auto& anim : state.animations())
            {
                auto& name = anim.name();
                auto result = loader(getAnimationName(name));
                if (result)
                {
                    anims.emplace(name, result.value());
                }
            }
        }
        return anims;
    }

    OptionalRef<const SkeletalAnimatorUtils::StateDefinition> SkeletalAnimatorUtils::getState(const Definition& def, std::string_view name)
    {
        auto itr = std::find_if(def.states().begin(), def.states().end(),
            [name](auto& stateDef) { return stateDef.name() == name; });
        if (itr == def.states().end())
        {
            return nullptr;
        }
        return *itr;
    }

    OptionalRef<const SkeletalAnimatorUtils::TransitionDefinition> SkeletalAnimatorUtils::getTransition(const Definition& def, std::string_view src, std::string_view dst)
    {
        auto& ts = def.transitions();
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

    namespace SkeletalAnimatorUtils
    {
        expected<void, std::string> read(AnimationDefinition& def, const nlohmann::json& json)
        {
            if (json.is_string())
            {
                def.set_name(json.get<std::string>());
                def.set_loop(true);
                return {};
            }
            auto itr = json.find("value");
            if (itr != json.end())
            {
                auto vec = itr->get<glm::vec2>();
				*def.mutable_blend_position() = protobuf::convert(vec);
            }
            itr = json.find("animation");
            if (itr != json.end())
            {
				def.set_name(itr->get<std::string>());
            }
            itr = json.find("loop");
            if (itr != json.end())
            {
				def.set_loop(itr->get<bool>());
            }
            else
            {
                def.set_loop(true);
            }
            return {};
        }

        expected<void, std::string> read(TweenDefinition& def, const nlohmann::json& json)
        {
            auto itr = json.find("duration");
            if (itr != json.end())
            {
				def.set_duration(itr->get<float>());
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
				def.set_easing(*easing);
            }
            return {};
        }

        std::optional<SkeletalAnimator::StateDefinition::BlendType> getBlendType(const std::string& name) noexcept
        {
            SkeletalAnimator::StateDefinition::BlendType val;
            if (!SkeletalAnimator::StateDefinition::BlendType_Parse(name, &val))
            {
                return std::nullopt;
            }
            return val;
        }

        expected<void, std::string> read(StateDefinition& def, const nlohmann::json& json)
        {
            auto itr = json.find("elements");
            if (itr != json.end())
            {
                for (auto& elm : *itr)
                {
                    auto& anim = *def.add_animations();
                    auto result = read(anim, elm);
                    if(!result)
                    {
                        return result;
					}
                }
            }
            itr = json.find("animation");
            if (itr != json.end())
            {
                auto& anim = *def.add_animations();
                auto result = read(anim, *itr);
                if (!result)
                {
                    return result;
                }
            }
            itr = json.find("name");
            if (itr != json.end())
            {
                def.set_name(itr->get<std::string>());
            }
            itr = json.find("threshold");
            if (itr != json.end())
            {
				def.set_threshold(itr->get<float>());
            }
            itr = json.find("blend");
            if (itr != json.end())
            {
                auto blend = getBlendType(itr->get<std::string>());
                if (!blend)
                {
                    return unexpected{ "invalid blend type" };
                }
                def.set_blend(*blend);
            }
            itr = json.find("tween");
            if (itr != json.end())
            {
                auto result = read(*def.mutable_tween(), *itr);
                if (!result)
                {
                    return result;
				}
            }
            itr = json.find("nextState");
            if (itr != json.end())
            {
				def.set_next_state(itr->get<std::string>());
            }
            itr = json.find("speed");
            if (itr != json.end())
            {
				def.set_speed(itr->get<float>());
            }
            else
            {
				def.set_speed(1.f);
            }
            return {};
        }

        expected<void, std::string> read(TransitionDefinition& def, const nlohmann::json& json)
        {
			auto result = read(*def.mutable_tween(), json);
            if (!result)
            {
                return result;
            }
            auto itr = json.find("offset");
            if (itr != json.end())
            {
				def.set_offset(itr->get<float>());
            }
			return {};
        }

        std::pair<std::string_view, std::string_view> parseTransitionKey(std::string_view key)
        {
            const char sep = '>';
            auto pos = key.find(sep);
            if (pos == std::string::npos)
            {
                return { {}, key };
            }
            return { key.substr(0, pos), key.substr(pos + 1) };
        }
    }

    expected<void, std::string> SkeletalAnimatorUtils::read(Definition& def, const nlohmann::json& json)
    {
        auto itr = json.find("animationNamePattern");
        if (itr != json.end())
        {
            def.set_animation_pattern(itr->get<std::string>());
        }
        itr = json.find("states");
        if (itr != json.end())
        {
            for (auto& [jsonKey, jsonVal] : itr->items())
            {
				auto& state = *def.add_states();    
                state.set_name(jsonKey);
                auto result = read(state, jsonVal);
                if(!result)
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
                auto& trans = *def.add_transitions();
                auto result = read(trans, jsonVal);
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
            _joints.emplace_back(joint.name(), protobuf::convert(joint.inverse_bind_pose()));
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
}