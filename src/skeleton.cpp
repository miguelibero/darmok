#include <darmok/skeleton.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/transform.hpp>
#include <darmok/render.hpp>
#include <glm/gtx/quaternion.hpp>
#include <stdexcept>


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

    Armature::Armature(const std::vector<ArmatureBone>& bones) noexcept
        : _bones(bones)
    {
    }

    Armature::Armature(std::vector<ArmatureBone>&& bones) noexcept
        : _bones(std::move(bones))
    {
    }

    const std::vector<ArmatureBone>& Armature::getBones() const noexcept
    {
        return _bones;
    }

    Skinnable::Skinnable(const std::shared_ptr<Armature>& armature) noexcept
        : _armature(armature)
    {
    }

    const std::shared_ptr<Armature>& Skinnable::getArmature() const noexcept
    {
        return _armature;
    }

    void Skinnable::setArmature(const std::shared_ptr<Armature>& armature) noexcept
    {
        _armature = armature;
    }

    std::shared_ptr<Skeleton> EmptySkeletonLoader::operator()(std::string_view name)
    {
        throw std::runtime_error("no skeletal animation implementation");
    }

    std::shared_ptr<SkeletalAnimation> EmptySkeletalAnimationLoader::operator()(std::string_view name)
    {
        throw std::runtime_error("no skeletal animation implementation");
    }

    RenderableSkeleton::RenderableSkeleton(const std::shared_ptr<Material>& mat, const std::shared_ptr<IMesh>& boneMesh) noexcept
        : _boneMesh(boneMesh)
        , _material(mat)
    {
        if (!_boneMesh)
        {
            MeshCreator creator;
            if (_material)
            {
                auto prog = _material->getProgram();
                if (prog)
                {
                    creator.vertexLayout = prog->getVertexLayout();
                }
            }
            _boneMesh = creator.createBone();
        }
    }
   
    void RenderableSkeleton::update(Scene& scene, const std::vector<glm::mat4>& boneMatrixes) noexcept
    {
        while (_boneTransforms.size() > boneMatrixes.size())
        {
            auto boneEntity = scene.getEntity(_boneTransforms.back());
            if (boneEntity != entt::null)
            {
                scene.destroyEntity(boneEntity);
            }
            _boneTransforms.pop_back();
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

        size_t i = 0;
        for (auto& mtx : boneMatrixes)
        {
            if (i >= _boneTransforms.size())
            {
                auto boneEntity = scene.createEntity();
                scene.addComponent<Renderable>(boneEntity, _boneMesh, _material);
                auto& boneTrans = scene.addComponent<Transform>(boneEntity, mtx, trans);
                _boneTransforms.emplace_back(boneTrans);
            }
            else
            {
                _boneTransforms[i]->setLocalMatrix(mtx);
            }
            ++i;
        }
    }

    void SkeletalAnimationUpdater::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
    }

    void SkeletalAnimationUpdater::update(float deltaTime) noexcept
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
            _scene->getComponentsInChildren<RenderableSkeleton>(entity, skeletons);
            for (auto& skel : skeletons)
            {
                skel->update(_scene.value(), anim.getBoneMatrixes());
            }
        }
    }

    void SkeletalAnimationCameraComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
        _cam = cam;
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
            auto& bones = armature->getBones();
            _skinning.reserve(bones.size() + 1);
            for (auto& bone : bones)
            {
                auto model = animator->getModelMatrix(bone.joint);
                _skinning.push_back(model * bone.inverseBindPose);
            }
        }
        encoder.setUniform(_skinningUniform, &_skinning.front(), _skinning.size());
    }

    SkeletalAnimatorConfig& SkeletalAnimatorConfig::addState(const StateConfig& config) noexcept
    {
        auto fixedName = config.name.empty() ? config.motion->getName() : config.name;
        _states.emplace(fixedName, config);
        return *this;
    }

    SkeletalAnimatorConfig& SkeletalAnimatorConfig::addState(const std::shared_ptr<SkeletalAnimation>& animation, std::string_view name) noexcept
    {
        return addState(StateConfig{ animation, std::string(name) });
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
            return std::nullopt;
        }
        return itr->second;
    }

}