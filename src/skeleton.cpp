#include <darmok/skeleton.hpp>

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
        auto ctrls = _scene->getRegistry().view<SkeletalAnimationController>();
        for (auto [entity, ctrl] : ctrls.each())
        {
            ctrl.update(deltaTime);
        }
    }

    void SkeletalAnimationCameraComponent::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _scene = scene;
    }

    void SkeletalAnimationCameraComponent::getEntityTransforms(Entity entity, std::vector<glm::mat4>& transforms) const
    {
        if (!_scene)
        {
            return;
        }
        auto& registry = _scene->getRegistry();
        auto ctrl = registry.try_get<SkeletalAnimationController>(entity);
        if (ctrl == nullptr)
        {
            return;
        }
        auto skinnable = registry.try_get<Skinnable>(entity);
        if (skinnable != nullptr)
        {
            auto armature = skinnable->getArmature();
            if (armature != nullptr)
            {
                for (auto& bone : armature->getBones())
                {
                    transforms.emplace_back(ctrl->getModelMatrix(bone.joint) * bone.inverseBindPose);
                }
            }
        }
    }

}