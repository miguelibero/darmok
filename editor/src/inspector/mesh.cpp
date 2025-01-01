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

    const std::array<std::string, std::variant_size_v<MeshSource::Content>> MeshSourceInspectorEditor::_typeLabels =
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

            {
                auto currentType = src.content.index();
                if (ImGui::BeginCombo("Type", _typeLabels[currentType].c_str()))
                {
                    for (size_t i = 0; i < _typeLabels.size(); ++i)
                    {
                        auto selected = currentType == i;
                        if (ImGui::Selectable(_typeLabels[i].c_str(), selected))
                        {
                            changed = true;
                        }
                        if (selected)
                        {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
            }

            if (auto cubeContent = std::get_if<CubeMeshSource>(&src.content))
            {
                auto any = entt::forward_as_meta(cubeContent->cube);
                if (_container->render(any))
                {
                    changed = true;
                }
            }

            if (ImGui::Checkbox("32bit indices", &src.config.index32))
            {
                changed = true;
            }

            if (ImGui::Button("Delete"))
            {
                _editor->getProject().removeMesh(src);
                changed = true;
            }
        }
        return changed;
    }
}
