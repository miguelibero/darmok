#include <darmok-editor/inspector/mesh.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/assimp.hpp>
#include <darmok/mesh_assimp.hpp>
#include <darmok/shape.hpp>

#include <imgui.h>
#include <assimp/scene.h>

namespace darmok::editor
{
    enum class MeshSourceValueType
    {
        Cube,
        Sphere,
        Capsule,
        Rectangle,
        Cone,
        Cylinder,
        Data,
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

        auto progResult = renderChild(*src.mutable_program());
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        changed |= *progResult;

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
        else if (src.has_cone())
        {
            type = MeshSourceValueType::Cone;
        }
        else if (src.has_cylinder())
        {
            type = MeshSourceValueType::Cylinder;
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
        else if (type == MeshSourceValueType::Cone)
        {
            result = renderCone(src);
        }
        else if (type == MeshSourceValueType::Cylinder)
        {
            result = renderCylinder(src);
        }
        else if (type == MeshSourceValueType::Data)
        {
            result = renderData(src);
        }
        if (!result)
        {
            return result;
        }
        changed |= *result;
        return changed;
    }

    expected<void, std::string> MeshSourceInspectorEditor::loadSceneMesh(protobuf::DataMeshSource& src) noexcept
    {
        if (!_externalScene)
        {
            return unexpected{ "no scene loaded" };
        }
        if (_externalMeshIndex < 0 || _externalMeshIndex >= _externalMeshes.size())
        {
            return unexpected{ "invalid mesh index" };
        }
        OptionalRef<const aiMesh> assimpMesh;
        auto& meshName = _externalMeshes[_externalMeshIndex];

        for (size_t i = 0; i < _externalScene->mNumMeshes; i++)
        {
            auto& m = _externalScene->mMeshes[i];
            if (AssimpUtils::getStringView(m->mName) == meshName)
            {
                assimpMesh = m;
                break;
            }
        }

        if (!assimpMesh)
        {
            return unexpected{ "selected mesh not found in scene" };
        }

        AssimpMeshSourceConverter converter{ *assimpMesh, src };
        return converter();
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderData(Mesh::Source& src) noexcept
    {
        auto changed = false;
        auto& dataSrc = *src.mutable_data();

		std::filesystem::path path;
        FileDialogOptions dialogOptions;
        dialogOptions.filters = { "*.fbx", "*.glb" };
        dialogOptions.filterDesc = "3D Model Files";
        if (getApp().drawFileInput("Load File", path, dialogOptions))
        {
            auto& assets = getApp().getAssets();
            auto dataResult = assets.getDataLoader()(path);
            if (!dataResult)
            {
                return unexpected{ std::move(dataResult).error() };
            }
            AssimpLoader loader;
            AssimpLoader::Config config;
            config.setPath(path);
            auto sceneResult = loader.loadFromMemory(dataResult.value(), config);
            if (!sceneResult)
            {
                return unexpected{ std::move(sceneResult).error() };
            }
            _externalScene = std::move(*sceneResult);
            _externalMeshes.clear();
            _externalMeshIndex = 0;
            for(size_t i = 0; i < _externalScene->mNumMeshes; i++)
            {
                auto& name = _externalScene->mMeshes[i]->mName;
                _externalMeshes.push_back(AssimpUtils::getString(name));
			}
            auto result = loadSceneMesh(dataSrc);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            changed = true;
        }
        if (!_externalMeshes.empty())
        {
            if (ImguiUtils::drawListCombo("Mesh Name", _externalMeshIndex, _externalMeshes))
            {
                auto result = loadSceneMesh(dataSrc);
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
                changed = true;
            }
        }

        if (dataSrc.vertices_size() > 0)
        {
            auto desc = fmt::format("{} vertices, {} indices, {} bones",
                dataSrc.vertices_size(),
                dataSrc.indices_size(),
                dataSrc.bones_size());
            ImGui::Text("%s", desc.c_str());
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
            sphere = Sphere::createDefinition();
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
            cube = Cube::createDefinition();
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
            capsule = Capsule::createDefinition();
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
            rect = Rectangle::createDefinition();
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

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderCone(Mesh::Source& src) noexcept
    {
        auto create = !src.has_cone();
        auto& coneSrc = *src.mutable_cone();
        auto& cone = *coneSrc.mutable_shape();
        auto changed = false;
        if (create)
        {
            coneSrc.set_lod(32);
            cone = Cone::createDefinition();
        }
        if (ImguiUtils::beginFrame("Cone"))
        {
            auto result = renderChild(cone);
            if (!result)
            {
                return result;
            }
            if (*result)
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Level of Detail", "lod", coneSrc))
            {
                changed = true;
            }
            ImguiUtils::endFrame();
        }

        return changed;
    }

    MeshSourceInspectorEditor::RenderResult MeshSourceInspectorEditor::renderCylinder(Mesh::Source& src) noexcept
    {
        auto create = !src.has_cylinder();
        auto& cylinderSrc = *src.mutable_cylinder();
        auto& cylinder = *cylinderSrc.mutable_shape();
        auto changed = false;
        if (create)
        {
            cylinderSrc.set_lod(32);
            cylinder = Cylinder::createDefinition();
        }
        if (ImguiUtils::beginFrame("Cylinder"))
        {
            auto result = renderChild(cylinder);
            if (!result)
            {
                return result;
            }
            if (*result)
            {
                changed = true;
            }
            if (ImguiUtils::drawProtobufInput("Level of Detail", "lod", cylinderSrc))
            {
                changed = true;
            }
            ImguiUtils::endFrame();
        }

        return changed;
    }
}
