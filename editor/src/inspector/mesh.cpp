#include <darmok-editor/inspector/mesh.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/shape_serialize.hpp>

#include <imgui.h>

namespace darmok::editor
{
    enum class MeshSourceValueType
    {
        Cube,
        Sphere,
        Capsule,
        Rectangle,
        External,
    };

    std::string MeshSourceInspectorEditor::getTitle() const noexcept
    {
        return "Mesh";
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderType(Mesh::Source& src) noexcept
    {
        auto changed = false;

        if (ImguiUtils::drawProtobufInput("Name", "name", src))
        {
            changed = true;
        }
        MeshSourceValueType type = MeshSourceValueType::External;
        if (src.has_capsule())
        {
            type = MeshSourceValueType::Capsule;
        }
        else if (src.has_cube())
        {
            type = MeshSourceValueType::Cube;
        }
        else if (src.has_sphere())
        {
            type = MeshSourceValueType::Sphere;
        }
        else if (src.has_rectangle())
        {
            type = MeshSourceValueType::Rectangle;
        }
        if (ImguiUtils::drawEnumCombo("Type", type))
        {
            changed = true;
        }
        if (type == MeshSourceValueType::Sphere)
        {
            auto create = !src.has_sphere();
            auto& sphereSrc = *src.mutable_sphere();
            auto& sphere = *sphereSrc.mutable_shape();
            if (create)
            {
                sphereSrc.set_lod(32);
                sphere = protobuf::convert(Sphere{});
            }
            if (ImguiUtils::beginFrame("Sphere"))
            {
                if (_container->render(sphere))
                {
                    changed = true;
                }
                if (ImguiUtils::drawProtobufInput("Level of Detail", "lod", sphereSrc))
                {
                    changed = true;
                }
                ImguiUtils::endFrame();
            }
        }
        else if (type == MeshSourceValueType::Cube)
        {
            auto create = !src.has_cube();
            auto& cubeSrc = *src.mutable_cube();
            auto& cube = *cubeSrc.mutable_shape();
            if (create)
            {
                cube = protobuf::convert(Cube{});
            }
            if (ImguiUtils::beginFrame("Cube"))
            {
                if (_container->render(cube))
                {
                    changed = true;
                }
                if (ImguiUtils::drawProtobufInput("Rectangle Type", "type", cubeSrc))
                {
                    changed = true;
                }
                ImguiUtils::endFrame();
            }
        }
        else if (type == MeshSourceValueType::Capsule)
        {
            auto create = !src.has_capsule();
            auto& capSrc = *src.mutable_capsule();
            auto& capsule = *capSrc.mutable_shape();
            if (create)
            {
                capSrc.set_lod(32);
                capsule = protobuf::convert(Capsule{});
            }
            if (ImguiUtils::beginFrame("Capsule"))
            {
                if (_container->render(capsule))
                {
                    changed = true;
                }
                if (ImguiUtils::drawProtobufInput("Level of Detail", "lod", capSrc))
                {
                    changed = true;
                }
                ImguiUtils::endFrame();
            }
        }
        else if (type == MeshSourceValueType::Rectangle)
        {
            auto create = !src.has_rectangle();
            auto& rectSrc = *src.mutable_rectangle();
            auto& rect = *rectSrc.mutable_shape();
            if (create)
            {
                rect = protobuf::convert(Rectangle{});
            }
            if (ImguiUtils::beginFrame("Rectangle"))
            {
                if (_container->render(rect))
                {
                    changed = true;
                }
                if (ImguiUtils::drawProtobufInput("Rectangle Type", "type", rectSrc))
                {
                    changed = true;
                }
                ImguiUtils::endFrame();
            }
        }
        return changed;
    }
}
