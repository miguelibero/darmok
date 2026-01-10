#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/skeleton.hpp>
#include <darmok-editor/utils.hpp>

struct aiScene;

namespace darmok::editor
{
    class ArmatureInspectorEditor final : public AssetObjectEditor<Armature::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Armature::Definition& armature) noexcept override;
    private:
        MeshFileInput _meshInput;
    };

    class SkinnableInspectorEditor final : public EntityComponentObjectEditor<Skinnable>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Skinnable::Definition& skinnable) noexcept override;
    };

    class SkeletalAnimatorInspectorEditor final : public EntityComponentObjectEditor<SkeletalAnimator>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SkeletalAnimator::Definition& animator) noexcept override;
    };

    class SkeletalAnimationSceneComponentInspectorEditor final : public SceneComponentObjectEditor<SkeletalAnimationSceneComponent>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SkeletalAnimationSceneComponent::Definition& def) noexcept override;
    };

    class SkeletalAnimationRenderComponentInspectorEditor final : public CameraComponentObjectEditor<SkeletalAnimationRenderComponent>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SkeletalAnimationRenderComponent::Definition& def) noexcept override;
    };
}