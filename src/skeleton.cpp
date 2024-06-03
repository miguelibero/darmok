#include <darmok/skeleton.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/transform.hpp>
#include <darmok/render.hpp>
#include <glm/gtx/quaternion.hpp>

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

    RenderableSkeleton::RenderableSkeleton(const std::shared_ptr<IMesh>& boneMesh, const std::shared_ptr<Material>& mat) noexcept
        : _boneMesh(boneMesh)
        , _material(mat)
    {
    }

    RenderableSkeleton::~RenderableSkeleton() noexcept
    {
        shutdown();
    }

    void RenderableSkeleton::init(Scene& scene) noexcept
    {
        _scene = scene;
        auto& registry = scene.getRegistry();
        auto entity = scene.getEntity(*this);
        if (entity != entt::null)
        {
            _ctrl = scene.getComponent<SkeletalAnimationController>(entity);
        }
        if (_ctrl && _boneMesh && _material)
        {
            auto& trans = scene.getOrAddComponent<Transform>(entity);
            auto& matrixes = _ctrl->getBoneMatrixes();
            for (auto& mtx : matrixes)
            {
                auto boneEntity = scene.createEntity();
                auto& boneTrans = scene.addComponent<Transform>(boneEntity, mtx, trans);
                scene.addComponent<Renderable>(boneEntity, _boneMesh, _material);
                _bones.emplace_back(boneTrans);
            }
        }
    }

    void RenderableSkeleton::update(float deltaTime) noexcept
    {
        if (!_ctrl)
        {
            return;
        }
        auto& matrixes = _ctrl->getBoneMatrixes();
        auto i = 0;
        for (auto& bone : _bones)
        {
            bone->setLocalMatrix(matrixes[i++]);
        }
    }

    void RenderableSkeleton::shutdown() noexcept
    {
        if (_scene)
        {
            for (auto& bone : _bones)
            {
                _scene->destroyEntity(_scene->getEntity(bone));
            }
        }
        _scene.reset();
        _bones.clear();
    }

    void SkeletalAnimationUpdater::init(Scene& scene, App& app) noexcept
    {
        _scene = scene;
        auto& registry = scene.getRegistry();
        registry.on_construct<RenderableSkeleton>().connect<&SkeletalAnimationUpdater::onSkeletonConstructed>(*this);
    }

    void SkeletalAnimationUpdater::onSkeletonConstructed(EntityRegistry& registry, Entity entity) noexcept
    {
        if (_scene)
        {
            auto& bones = registry.get<RenderableSkeleton>(entity);
            bones.init(_scene.value());
        }
    }

    void SkeletalAnimationUpdater::shutdown() noexcept
    {
        if (_scene)
        {
            auto& registry = _scene->getRegistry();
            registry.on_construct<RenderableSkeleton>().disconnect<&SkeletalAnimationUpdater::onSkeletonConstructed>(*this);
        }
    }

    void SkeletalAnimationUpdater::update(float deltaTime) noexcept
    {
        if (!_scene)
        {
            return;
        }
        auto ctrls = _scene->getRegistry().view<SkeletalAnimationController>();
        for (auto [entity, ctrl] : ctrls.each())
        {
            ctrl.update(deltaTime);
        }
        auto skels = _scene->getRegistry().view<RenderableSkeleton>();
        for (auto [entity, skel] : skels.each())
        {
            skel.update(deltaTime);
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

    OptionalRef<SkeletalAnimationController> SkeletalAnimationCameraComponent::getController(Entity entity) const noexcept
    {
        if (!_scene)
        {
            return nullptr;
        }
        auto& registry = _scene->getRegistry();
        return registry.try_get<SkeletalAnimationController>(entity);
    }

    void SkeletalAnimationCameraComponent::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder, bgfx::ViewId viewId) noexcept
    {
        auto ctrl = getController(entity);
        if (!ctrl)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        auto skinnable = registry.try_get<Skinnable>(entity);
        if (skinnable == nullptr)
        {
            return;
        }

        auto armature = skinnable->getArmature();
        _skinning.clear();
        _skinning.push_back(glm::mat4(1));
        if (armature != nullptr)
        {
            auto& bones = armature->getBones();
            _skinning.reserve(bones.size() + 1);
            for (auto& bone : bones)
            {
                auto model = ctrl->getModelMatrix(bone.joint);
                _skinning.push_back(model * bone.inverseBindPose);
            }
        }
        encoder.setUniform(_skinningUniform, &_skinning.front(), _skinning.size());
    }

}