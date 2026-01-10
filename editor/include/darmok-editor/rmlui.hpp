#pragma once

#include <darmok-editor/app.hpp>
#include <darmok-editor/editor.hpp>
#include <darmok/rmlui.hpp>

namespace darmok::editor
{
    class RmluiRendererInspectorEditor final : public CameraComponentObjectEditor<RmluiRenderer>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(RmluiRenderer::Definition& text) noexcept override;
    };

    class RmluiCanvasInspectorEditor final : public EntityComponentObjectEditor<RmluiCanvas>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(RmluiCanvas::Definition& text) noexcept override;
    };

    class RmluiEditorAppComponent final : public IEditorAppComponent
    {
        expected<void, std::string> renderMainMenu(MainMenuSection section) noexcept override;
        expected<void, std::string> init(EditorApp& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;

        OptionalRef<EditorApp> _app;
    };
}