#include <darmok-editor/inspector/texture.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/image.hpp>

#include <imgui.h>

namespace darmok::editor
{
    void TextureDefinitionInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
    {
        _app = app;
    }

    void TextureDefinitionInspectorEditor::shutdown()
    {
        _app.reset();
    }

    std::string TextureDefinitionInspectorEditor::getTitle() const noexcept
    {
        return "Texture";
    }

    const std::string TextureDefinitionInspectorEditor::_imageFilter = "*.png *.jpg *.jpeg *.bmp";
    const glm::vec2 TextureDefinitionInspectorEditor::_maxPreviewSize{ 200.F };

    bool TextureDefinitionInspectorEditor::renderType(Texture::Source& src) noexcept
    {
        auto changed = false;

        if (ImguiUtils::drawProtobufInput("Name", "name", src))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Format", "format", src))
        {
            changed = true;
        }

        if (_app)
        {
            auto& proj = _app->getProject();
            auto& assets = _app->getAssets();

            std::filesystem::path imgPath;
            if (ImguiUtils::drawFileInput("Load Image", imgPath, _imageFilter))
            {
                if (auto dataResult = assets.getDataLoader()(imgPath))
                {
                    src.set_image_data(dataResult.value().toString());
                    _tex.reset();
                    changed = true;
                }
            }
        }

        if (!_tex)
        {
            Texture::Definition def;
			TextureDefinitionWrapper defWrapper{ def };
            if (auto result = defWrapper.loadSource(src, _app->getProject().getAssets().getAllocator()))
            {
				_tex = std::make_shared<Texture>(def);
            }
        }

        if (_tex)
        {
			ImguiUtils::drawTexturePreview(*_tex, _maxPreviewSize);
        }

        return changed;
    }
}