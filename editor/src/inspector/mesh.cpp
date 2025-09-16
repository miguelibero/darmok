#include <darmok-editor/inspector/mesh.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/shape_serialize.hpp>
#include <darmok/mesh_assimp.hpp>
#include <imgui.h>

namespace darmok::editor
{
    enum class MeshSourceValueType
    {
        Cube,
        Sphere,
        Capsule,
        Rectangle,
        Data,
    };

    std::string MeshSourceInspectorEditor::getTitle() const noexcept
    {
        return "Mesh";
    }

    const std::string MeshSourceInspectorEditor::_externalFilter = "*.fbx *.glb";

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderType(Mesh::Source& src) noexcept
    {
        auto changed = false;

        if (ImguiUtils::drawProtobufInput("Name", "name", src))
        {
            changed = true;
        }
        MeshSourceValueType type = MeshSourceValueType::Data;
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
        else if (src.has_data())
        {
            type = MeshSourceValueType::Data;
        }
        if (ImguiUtils::drawEnumCombo("Type", type))
        {
            changed = true;
        }
		RenderResult result;
        if (type == MeshSourceValueType::Sphere)
        {
			result = renderSphere(src);
        }
        else if (type == MeshSourceValueType::Cube)
        {
            result = renderCube(src);
        }
        else if (type == MeshSourceValueType::Capsule)
        {
            result = renderCapsule(src);
        }
        else if (type == MeshSourceValueType::Rectangle)
        {
            result = renderRectangle(src);
        }
        else if (type == MeshSourceValueType::Data)
        {
            result = renderData(src);
        }
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        changed |= *result;
        return changed;
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderData(Mesh::Source& src) noexcept
    {
        auto& dataSrc = *src.mutable_data();

        auto createConverter = [&]()
        {
            auto format = _externalPath.extension().string();
            return AssimpMeshSourceConverter{ _externalData, format, dataSrc };
        };

        auto changed = false;

        if (ImguiUtils::drawFileInput("Load File", _externalPath, _externalFilter))
        {
            auto& assets = getApp().getAssets();
            auto result = assets.getDataLoader()(_externalPath);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            _externalData = std::move(result).value();
            auto convert = createConverter();
            _externalMeshes = convert.getMeshNames();
            _selectedExternalMeshIndex = 0;
        }
        if (!_externalMeshes.empty())
        {
            if (ImguiUtils::drawListCombo("Mesh Name", _selectedExternalMeshIndex, _externalMeshes))
            {
                auto convert = createConverter();
                auto result = convert(_externalMeshes[_selectedExternalMeshIndex]);
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
                changed = true;
            }
        }

		return changed;
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderSphere(Mesh::Source& src) noexcept
    {
        auto create = !src.has_sphere();
        auto& sphereSrc = *src.mutable_sphere();
        auto& sphere = *sphereSrc.mutable_shape();
		auto changed = false;
        if (create)
        {
            sphereSrc.set_lod(32);
            sphere = protobuf::convert(Sphere{});
        }
        if (ImguiUtils::beginFrame("Sphere"))
        {
            auto result = renderChild(sphere);
            if (!result)
            {
                return result;
            }
            if (*result)
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Level of Detail", "lod", sphereSrc))
            {
                changed = true;
            }
            ImguiUtils::endFrame();
        }

        return changed;
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderCube(Mesh::Source& src) noexcept
    {
        auto changed = false;
        auto create = !src.has_cube();
        auto& cubeSrc = *src.mutable_cube();
        auto& cube = *cubeSrc.mutable_shape();
        if (create)
        {
            cube = protobuf::convert(Cube{});
        }
        if (ImguiUtils::beginFrame("Cube"))
        {
            auto result = renderChild(cube);
            if (!result)
            {
                return result;
            }
            if (*result)
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Rectangle Type", "type", cubeSrc))
            {
                changed = true;
            }
            ImguiUtils::endFrame();
        }
        return changed;
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderCapsule(Mesh::Source& src) noexcept
    {
        auto create = !src.has_capsule();
        auto& capSrc = *src.mutable_capsule();
        auto& capsule = *capSrc.mutable_shape();
        if (create)
        {
            capSrc.set_lod(32);
            capsule = protobuf::convert(Capsule{});
        }
        auto changed = false;
        if (ImguiUtils::beginFrame("Capsule"))
        {
            auto result = renderChild(capsule);
            if (!result)
            {
                return result;
            }
            if (*result)
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Level of Detail", "lod", capSrc))
            {
                changed = true;
            }
            ImguiUtils::endFrame();
        }
        return changed;
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderRectangle(Mesh::Source& src) noexcept
    {
        auto create = !src.has_rectangle();
        auto& rectSrc = *src.mutable_rectangle();
        auto& rect = *rectSrc.mutable_shape();
        if (create)
        {
            rect = protobuf::convert(Rectangle{});
        }
        auto changed = false;
        if (ImguiUtils::beginFrame("Rectangle"))
        {
            auto result = renderChild(rect);
            if (!result)
            {
                return result;
            }
            if (*result)
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Rectangle Type", "type", rectSrc))
            {
                changed = true;
            }
            ImguiUtils::endFrame();
        }
        return changed;
    }
}
