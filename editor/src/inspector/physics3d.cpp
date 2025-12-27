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
            result = renderSphere(shape);
        }
        else if (type == Physics3dShapeType::Cube)
        {
            result = renderCube(shape);
        }
        else if (type == Physics3dShapeType::Capsule)
        {
            result = renderCapsule(shape);
        }
        else if (type == Physics3dShapeType::Polygon)
        {
            result = renderPolygon(shape);
        }
        else if (type == Physics3dShapeType::BoundingBox)
        {
            result = renderBoundingBox(shape);
        }
        if (!result)
        {
            return result;
        }
        changed |= *result;
        return changed;
    }

    Physics3dShapeInspectorEditor::RenderResult Physics3dShapeInspectorEditor::renderCube(Object& shape) noexcept
    {
        auto create = !shape.has_cube();
        auto& cube = *shape.mutable_cube();
        auto changed = false;
        if (create)
        {
            cube = Cube::createDefinition();
        }
        return renderChild(cube);
    }

    Physics3dShapeInspectorEditor::RenderResult Physics3dShapeInspectorEditor::renderSphere(Object& shape) noexcept
    {
        auto create = !shape.has_sphere();
        auto& sphere = *shape.mutable_sphere();
        auto changed = false;
        if (create)
        {
            sphere = Sphere::createDefinition();
        }
        return renderChild(sphere);
    }

    Physics3dShapeInspectorEditor::RenderResult Physics3dShapeInspectorEditor::renderCapsule(Object& shape) noexcept
    {
        auto create = !shape.has_capsule();
        auto& capsule = *shape.mutable_capsule();
        auto changed = false;
        if (create)
        {
            capsule = Capsule::createDefinition();
        }
        return renderChild(capsule);
    }

    Physics3dShapeInspectorEditor::RenderResult Physics3dShapeInspectorEditor::renderPolygon(Object& shape) noexcept
    {
        auto create = !shape.has_polygon();
        auto& polygon = *shape.mutable_polygon();
        auto changed = false;
        if (create)
        {
            polygon = Polygon::createDefinition();
        }
        return renderChild(polygon);
    }

    Physics3dShapeInspectorEditor::RenderResult Physics3dShapeInspectorEditor::renderBoundingBox(Object& shape) noexcept
    {
        auto create = !shape.has_bounding_box();
        auto& bbox = *shape.mutable_bounding_box();
        auto changed = false;
        if (create)
        {
            bbox = BoundingBox::createDefinition();
        }
        return renderChild(bbox);
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