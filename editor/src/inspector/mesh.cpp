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

    bool MeshSourceInspectorEditor::render(MeshSource& src) noexcept
    {
        if (ImGui::CollapsingHeader("Mesh"))
        {
            ImGui::InputText("Name", &src.name);

            {
                auto currentType = src.content.index();
                if (ImGui::BeginCombo("Type", _typeLabels[currentType].c_str()))
                {
                    for (size_t i = 0; i < _typeLabels.size(); ++i)
                    {
                        auto selected = currentType == i;
                        if (ImGui::Selectable(_typeLabels[i].c_str(), selected))
                        {
                           
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
                _container->render(any);
            }

            auto index32 = src.config.index32;
            if (ImGui::Checkbox("32bit indices", &index32))
            {
                src.config.index32 = index32;
            }

            if (ImGui::Button("Delete"))
            {
                _editor->getProject().removeMesh(src);
            }
        }
        return true;
    }
}
