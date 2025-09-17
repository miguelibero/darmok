#include <darmok-editor/inspector/skeleton.hpp>
#include <darmok-editor/utils.hpp>

namespace darmok::editor
{

    std::string ArmatureInspectorEditor::getTitle() const noexcept
    {
        return "Armature";
    }

    ArmatureInspectorEditor::RenderResult ArmatureInspectorEditor::renderType(Armature::Definition& armature) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Name", "name", armature))
        {
            changed = true;
        }
        return changed;
    }

    std::string SkinnableInspectorEditor::getTitle() const noexcept
    {
        return "Skinnable";
    }

    SkinnableInspectorEditor::RenderResult SkinnableInspectorEditor::renderType(Skinnable::Definition& skinnable) noexcept
    {
        auto changed = false;
        return changed;
    }

    std::string SkeletalAnimatorDefinitionInspectorEditor::getTitle() const noexcept
    {
        return "Skeletal Animator";
    }

    SkeletalAnimatorDefinitionInspectorEditor::RenderResult SkeletalAnimatorDefinitionInspectorEditor::renderType(SkeletalAnimator::Definition& animator) noexcept
    {
        auto changed = false;
        return changed;
    }

    std::string SkeletalAnimatorInspectorEditor::getTitle() const noexcept
    {
        return "Skeletal Animator";
    }

    SkeletalAnimatorInspectorEditor::RenderResult SkeletalAnimatorInspectorEditor::renderType(SkeletalAnimator::Definition& animator) noexcept
    {
        auto changed = false;
        return changed;
    }
}