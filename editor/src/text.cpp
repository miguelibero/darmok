#include <darmok-editor/text.hpp>
#include <darmok-editor/inspector/text.hpp>
#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>

namespace darmok::editor
{
    expected<void, std::string> TextEditorAppComponent::renderMainMenu(MainMenuSection section) noexcept
    {
        if (!_app)
        {
            return unexpected{ "not initialized" };
        }
        if (section == MainMenuSection::AddAsset)
        {
            DARMOK_TRY(_app->drawAssetComponentMenu("Freetype Font", FreetypeFontLoader::createDefinition()));
        }
        else if (section == MainMenuSection::AddEntityComponent)
        {
            DARMOK_TRY(_app->drawEntityComponentMenu<Text>("Text"));
        }
        else if (section == MainMenuSection::AddCameraComponent)
        {
            DARMOK_TRY(_app->drawCameraComponentMenu<TextRenderer>("Text"));
        }
        return {};
    }

    expected<void, std::string> TextEditorAppComponent::init(EditorApp& app) noexcept
    {
        _app = app;
        auto& inspector = app.getInspectorView();
        DARMOK_TRY(inspector.addEditor<TextInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<TextRendererInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<FreetypeFontInspectorEditor>());
        return {};
    }

    expected<void, std::string> TextEditorAppComponent::shutdown() noexcept
    {
        _app.reset();
        return {};
    }
}