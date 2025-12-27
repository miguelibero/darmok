#include <darmok-editor/inspector/shape.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/assimp.hpp>

#include <imgui.h>

namespace darmok::editor
{

    std::string CubeInspectorEditor::getTitle() const noexcept
    {
        return "Cube";
    }

    CubeInspectorEditor::RenderResult CubeInspectorEditor::renderType(Cube::Definition& cube) noexcept
    {
        auto changed = false;

        if (ImguiUtils::drawProtobufInput("Size", "size", cube))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", cube))
        {
            changed = true;
        }

        return changed;
    }

    std::string SphereInspectorEditor::getTitle() const noexcept
    {
        return "Sphere";
    }

    CubeInspectorEditor::RenderResult SphereInspectorEditor::renderType(Sphere::Definition& sphere) noexcept
    {
        auto changed = false;
 
        if (ImguiUtils::drawProtobufInput("Radius", "radius", sphere))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", sphere))
        {
            changed = true;
        }

        return changed;
    }

    std::string CapsuleInspectorEditor::getTitle() const noexcept
    {
        return "Capsule";
    }

    CubeInspectorEditor::RenderResult CapsuleInspectorEditor::renderType(Capsule::Definition& capsule) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Cylinder height", "cylinder_height", capsule))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Radius", "radius", capsule))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", capsule))
        {
            changed = true;
        }

        return changed;
    }

    std::string RectangleInspectorEditor::getTitle() const noexcept
    {
        return "Rectangle";
    }

    CubeInspectorEditor::RenderResult RectangleInspectorEditor::renderType(Rectangle::Definition& rect) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Size", "size", rect))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Origin", "origin", rect))
        {
            changed = true;
        }
        return changed;
    }

    std::string PolygonInspectorEditor::getTitle() const noexcept
    {
        return "Polygon";
    }

    PolygonInspectorEditor::RenderResult PolygonInspectorEditor::renderType(Polygon::Definition& poly) noexcept
    {
        auto meshResult = _meshInput.draw(getApp());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }
        auto changed = meshResult.value();
        if (_meshInput.mesh && changed)
        {
            poly = convert<Polygon::Definition>(*_meshInput.mesh);
        }
        if (poly.triangles_size() > 0)
        {
            ImGui::Text("%d triangles", poly.triangles_size());
        }
        return changed;
    }


    std::string BoundingBoxInspectorEditor::getTitle() const noexcept
    {
        return "Bounding Box";
    }

    BoundingBoxInspectorEditor::RenderResult BoundingBoxInspectorEditor::renderType(BoundingBox::Definition& bbox) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Min", "min", bbox))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Max", "max", bbox))
        {
            changed = true;
        }
        return changed;
    };

    std::string ConeInspectorEditor::getTitle() const noexcept
    {
        return "Cone";
    }

    ConeInspectorEditor::RenderResult ConeInspectorEditor::renderType(Cone::Definition& cone) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Height", "height", cone))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Radius", "radius", cone))
        {
            changed = true;
        }
        return changed;
    }

    std::string CylinderInspectorEditor::getTitle() const noexcept
    {
        return "Cylinder";
    }

    CylinderInspectorEditor::RenderResult CylinderInspectorEditor::renderType(Cylinder::Definition& cylinder) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Height", "height", cylinder))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Radius", "radius", cylinder))
        {
            changed = true;
        }
        return changed;
    }
}
