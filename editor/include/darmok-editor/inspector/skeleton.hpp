#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/skeleton.hpp>

namespace darmok::editor
{
    class ArmatureInspectorEditor final : public AssetObjectEditor<Armature::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Armature::Definition& armature) noexcept override;
    };

    class SkinnableInspectorEditor final : public ComponentObjectEditor<Skinnable>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Skinnable::Definition& skinnable) noexcept override;
    };

    class SkeletalAnimatorDefinitionInspectorEditor final : public AssetObjectEditor<SkeletalAnimator::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SkeletalAnimator::Definition& animator) noexcept override;
    };

    class SkeletalAnimatorInspectorEditor final : public ComponentObjectEditor<SkeletalAnimator>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(SkeletalAnimator::Definition& animator) noexcept override;
    };
}