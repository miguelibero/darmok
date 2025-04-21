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
        : _boneMesh(boneMesh)
        , _material(mat)
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
            const Line line(glm::vec3(0), glm::vec3(1, 0, 0));
            _boneMesh = MeshData(line, MeshData::LineMeshType::Arrow).createMesh(layout);
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

    float SkeletalAnimatorUtils::calcTween(const TweenDefinition& tween, float position)
    {
        return Easing::apply(static_cast<Easing::Type>(tween.easing()), position, 0.F, 1.F);
    }

    float SkeletalAnimatorUtils::calcBlendWeight(const StateDefinition& state, const glm::vec2& pos, const glm::vec2& animPos)
    {
        std::vector<std::pair<glm::vec2, glm::vec2>> parts;
        parts.reserve(state.animations_size());
        if (state.blend() == protobuf::SkeletalAnimatorBlendType::Directional)
        {
            auto lenAnim = glm::length(animPos);
            auto len = glm::length(pos);
            for (auto& anim : state.animations())
            {
                auto blendPos = GlmProtobufUtils::convert(anim.blend_position());
                auto lenAnim2 = glm::length(blendPos);
                auto p = lenAnim + lenAnim2;
                auto a = glm::vec2((len - lenAnim) * p, glm::angle(pos, animPos));
                auto b = glm::vec2((lenAnim2 - lenAnim) * p, glm::angle(blendPos, animPos));
                parts.emplace_back(a, b);
            }
        }
        else
        {
            auto a = pos - animPos;
            for (auto& anim : state.animations())
            {
                auto b = GlmProtobufUtils::convert(anim.blend_position()) - animPos;
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
            auto w = calcBlendWeight(state, pos, GlmProtobufUtils::convert(anim.blend_position()));
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
                anims.emplace(name, loader(getAnimationName(name)));
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

    Armature::Armature(const std::vector<ArmatureJoint>& joints) noexcept
        : _joints(joints)
    {
    }

    Armature::Armature(std::vector<ArmatureJoint>&& joints) noexcept
        : _joints(std::move(joints))
    {
    }

    const std::vector<ArmatureJoint>& Armature::getJoints() const noexcept
    {
        return _joints;
    }

    Skinnable::Skinnable(const std::shared_ptr<Armature>& armature) noexcept
        : _armature(armature)
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
}