#include <darmok-editor/inspector/mesh.hpp>
#include <darmok-editor/app.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    void MeshSourceInspectorEditor::init(EditorApp& editor, ObjectEditorContainer& container)
    {
        _editor = editor;
        _container = container;
    }

    void MeshSourceInspectorEditor::shutdown()
    {
        _editor.reset();
        _container.reset();
    }

    const std::array<std::string, std::variant_size_v<MeshSource::Content>> MeshSourceInspectorEditor::_contentOptions =
    {
        "Cube",
        "Sphere",
        "Capsule",
        "Rectangle",
        "Plane",
    };

    bool MeshSourceInspectorEditor::renderType(MeshSource& src) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Mesh"))
        {
            if (ImGui::InputText("Name", &src.name))
            {
                changed = true;
            }

            auto currentContent = src.content.index();
            if (ImguiUtils::drawArrayCombo("Type", currentContent, _contentOptions))
            {
                switch (currentContent)
                {
                case 0:
                    src.content.emplace<CubeMeshSource>();
                    break;
                case 1:
                    src.content.emplace<SphereMeshSource>();
                    break;
                case 2:
                    src.content.emplace<CapsuleMeshSource>();
                    break;
                case 3:
                    src.content.emplace<RectangleMeshSource>();
                    break;
                case 4:
                    src.content.emplace<PlaneMeshSource>();
                    break;
                };
                changed = true;
            }

            if (auto cubeContent = std::get_if<CubeMeshSource>(&src.content))
            {
                if (_container->renderType(cubeContent->cube))
                {
                    changed = true;
                }
            }
            else if (auto sphereContent = std::get_if<SphereMeshSource>(&src.content))
            {
                if (_container->renderType(sphereContent->sphere))
                {
                    changed = true;
                }
            }

            if (ImGui::Checkbox("32bit indices", &src.config.index32))
            {
                changed = true;
            }

            auto proj = _editor->getProject();

            if (ImGui::Button("Apply Changes"))
            {
                proj.reloadMesh(src);
            }

            ImGui::SameLine();

            if (ImGui::Button("Delete"))
            {
                proj.removeMesh(src);
                changed = true;
            }
        }
        return changed;
    }
}
