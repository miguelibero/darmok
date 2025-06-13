#include <darmok-editor/inspector/mesh.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>

#include <imgui.h>

namespace darmok::editor
{
    void MeshSourceInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
    {
        _app = app;
        _container = container;
    }

    void MeshSourceInspectorEditor::shutdown()
    {
        _app.reset();
        _container.reset();
    }

    bool MeshSourceInspectorEditor::renderType(Mesh::Source& src) noexcept
    {
        auto changed = false;
        if (ImGui::CollapsingHeader("Mesh"))
        {
            if (ImguiUtils::drawProtobufInput("Name", "name", src))
            {
                changed = true;
            }
        }
        return changed;
    }
}
