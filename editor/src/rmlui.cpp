#include <darmok-editor/rmlui.hpp>

namespace darmok::editor
{
    std::string RmluiRendererInspectorEditor::getTitle() const noexcept
    {
        return "Rmlui Renderer";
    }

    RmluiRendererInspectorEditor::RenderResult RmluiRendererInspectorEditor::renderType(RmluiRenderer::Definition& text) noexcept
    {
        auto changed = false;
        return changed;
    }

    std::string RmluiCanvasInspectorEditor::getTitle() const noexcept
    {
        return "Rmlui Canvas";
    }

    RmluiCanvasInspectorEditor::RenderResult RmluiCanvasInspectorEditor::renderType(RmluiCanvas::Definition& text) noexcept
    {
        auto changed = false;
        return changed;
    }

    expected<void, std::string> RmluiEditorAppComponent::renderMainMenu(MainMenuSection section) noexcept
    {
        if (!_app)
        {
            return unexpected{ "not initialized" };
        }
        else if (section == MainMenuSection::AddEntityComponent)
        {
            DARMOK_TRY(_app->drawEntityComponentMenu<RmluiCanvas>("Rmlui Canvas"));
        }
        else if (section == MainMenuSection::AddCameraComponent)
        {
            DARMOK_TRY(_app->drawCameraComponentMenu<RmluiRenderer>("Rmlui"));
        }
        return {};
    }

    expected<void, std::string> RmluiEditorAppComponent::init(EditorApp& app) noexcept
    {
        _app = app;
        auto& inspector = app.getInspectorView();
        DARMOK_TRY(inspector.addEditor<RmluiRendererInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<RmluiCanvasInspectorEditor>());
        return {};
    }

    expected<void, std::string> RmluiEditorAppComponent::shutdown() noexcept
    {
        _app.reset();
        return {};
    }
}