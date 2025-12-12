#include <darmok-editor/inspector/physics3d.hpp>
#include <darmok-editor/utils.hpp>

namespace darmok::editor
{
    std::string Physics3dBodyInspectorEditor::getTitle() const noexcept
    {
        return "Physics3d Body";
    }

    Physics3dBodyInspectorEditor::RenderResult Physics3dBodyInspectorEditor::renderType(physics3d::PhysicsBody::Definition& def) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Shape", "shape", def))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Motion", "motion", def))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Mass", "mass", def))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Inertia factor", "inertia_factor", def))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Friction", "friction", def))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Gravity Factor", "gravity_factor", def))
        {
            changed = true;
        }
		// TODO: Layer mask editor
        if (ImguiUtils::drawProtobufInput("Layer", "layer", def))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Trigger", "trigger", def))
        {
            changed = true;
        }
        return changed;
    }

    std::string Physics3dCharacterControllerInspectorEditor::getTitle() const noexcept
    {
        return "Physics3d Character Controller";
    }

    Physics3dCharacterControllerInspectorEditor::RenderResult Physics3dCharacterControllerInspectorEditor::renderType(physics3d::CharacterController::Definition& def) noexcept
    {
        return false;
    }
}