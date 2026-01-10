#include <darmok-editor/inspector/skeleton.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/assimp.hpp>
#include <darmok/skeleton_assimp.hpp>
#include <darmok/stream.hpp>

#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <fmt/format.h>

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

        auto meshResult = _meshInput.draw(getApp());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }
        if (meshResult.value() && _meshInput.mesh)
        {
            AssimpArmatureDefinitionConverter converter{ *_meshInput.mesh, armature };
            auto convertResult = converter();
            if (!convertResult)
            {
                return unexpected{ std::move(convertResult).error() };
            }
            changed = true;
        }

        if (armature.joints_size() > 0)
        {
            auto desc = fmt::format("{} joints", armature.joints_size());
            ImGui::Text("%s", desc.c_str());
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

        auto armatureDragType = getApp().getAssetDragType<Armature::Definition>().value_or("");

        auto result = ImguiUtils::drawProtobufAssetReferenceInput("Armature", "armature_path", skinnable, armatureDragType.c_str());
        if (result == ReferenceInputAction::Changed)
        {
            changed = true;
        }

        return changed;
    }

    std::string SkeletalAnimatorInspectorEditor::getTitle() const noexcept
    {
        return "Skeletal Animator";
    }

    SkeletalAnimatorInspectorEditor::RenderResult SkeletalAnimatorInspectorEditor::renderType(SkeletalAnimator::Definition& animator) noexcept
    {
        auto changed = false;
        std::filesystem::path path;
        FileDialogOptions dialogOptions;
        dialogOptions.filters = { "*.json" };
        if (getApp().drawFileInput("Load File", path, dialogOptions))
        {
            auto jsonResult = StreamUtils::parseJson(path);
            if(!jsonResult)
            {
                return unexpected{ "error parsing json: " + jsonResult.error() };
            }

            SkeletalAnimatorDefinitionWrapper wrapper{ animator };
            auto result = wrapper.read(*jsonResult);
            if (!result)
            {
                return unexpected{ result.error() };
            }
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Animation Name Pattern", "animation_pattern", animator))
        {
            changed = true;
        }
        if (ImguiUtils::beginFrame("States"))
        {

        }
        ImguiUtils::endFrame();
        if (ImguiUtils::beginFrame("Transitions"))
        {

        }
        ImguiUtils::endFrame();
        if(animator.states_size() > 0 || animator.transitions_size() > 0)
        {
            auto desc = fmt::format("{} states, {} transitions", animator.states_size(), animator.transitions_size());
            ImGui::Text("%s", desc.c_str());
		}
        return changed;
    }

    std::string SkeletalAnimationSceneComponentInspectorEditor::getTitle() const noexcept
    {
        return "Skeletal Animation";
    }

    SkeletalAnimationSceneComponentInspectorEditor::RenderResult SkeletalAnimationSceneComponentInspectorEditor::renderType(SkeletalAnimationSceneComponent::Definition& def) noexcept
    {
        auto changed = false;
        return false;
    }


    std::string SkeletalAnimationRenderComponentInspectorEditor::getTitle() const noexcept
    {
        return "Skeletal Animation";
    }

    SkeletalAnimationRenderComponentInspectorEditor::RenderResult SkeletalAnimationRenderComponentInspectorEditor::renderType(SkeletalAnimationRenderComponent::Definition& def) noexcept
    {
        auto changed = false;
        return false;
    }
}