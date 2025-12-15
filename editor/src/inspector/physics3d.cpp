#include <darmok-editor/inspector/physics3d.hpp>
#include <darmok-editor/utils.hpp>

namespace darmok::editor
{
    std::string Physics3dShapeInspectorEditor::getTitle() const noexcept
    {
        return "Physics3d Shape";
    }
    
    Physics3dShapeInspectorEditor::RenderResult Physics3dShapeInspectorEditor::renderType(physics3d::PhysicsBody::ShapeDefinition& shape) noexcept
    {
        auto changed = false;

        Physics3dShapeType type = Physics3dShapeType::Sphere;
        if (shape.has_capsule())
        {
            type = Physics3dShapeType::Capsule;
        }
        else if (shape.has_cube())
        {
            type = Physics3dShapeType::Cube;
        }
        else if (shape.has_sphere())
        {
            type = Physics3dShapeType::Sphere;
        }
        else if (shape.has_polygon())
        {
            type = Physics3dShapeType::Polygon;
        }
        else if (shape.has_bounding_box())
        {
            type = Physics3dShapeType::BoundingBox;
        }
        if (ImguiUtils::drawEnumCombo("Type", type))
        {
            changed = true;
        }

        RenderResult result;
        if (type == Physics3dShapeType::Sphere)
        {
            result = renderChild(*shape.mutable_sphere());
        }
        else if (type == Physics3dShapeType::Cube)
        {
            result = renderChild(*shape.mutable_cube());
        }
        else if (type == Physics3dShapeType::Capsule)
        {
            result = renderChild(*shape.mutable_capsule());
        }
        else if (type == Physics3dShapeType::Polygon)
        {
            result = renderChild(*shape.mutable_polygon());
        }
        else if (type == Physics3dShapeType::BoundingBox)
        {
            result = renderChild(*shape.mutable_bounding_box());
        }
        if (!result)
        {
            return result;
        }
        changed |= *result;
        return changed;
    }

    std::string Physics3dBodyInspectorEditor::getTitle() const noexcept
    {
        return "Physics3d Body";
    }

    Physics3dBodyInspectorEditor::RenderResult Physics3dBodyInspectorEditor::renderType(physics3d::PhysicsBody::Definition& def) noexcept
    {
        auto changed = false;
        auto result = renderChild(*def.mutable_shape(), true);
        if (!result)
        {
            return result;
        }
        changed |= *result;
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

    std::string Physics3dSystemInspectorEditor::getTitle() const noexcept
    {
        return "Physics3d System";
    }
    
    Physics3dSystemInspectorEditor::RenderResult Physics3dSystemInspectorEditor::renderType(physics3d::PhysicsSystem::Definition& def) noexcept
    {
        return false;
    }
}