#include <darmok-editor/inspector/texture.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/image.hpp>

#include <imgui.h>

namespace darmok::editor
{
    std::string TextureDefinitionInspectorEditor::getTitle() const noexcept
    {
        return "Texture";
    }

    const std::string TextureDefinitionInspectorEditor::_imageFilter = "*.png *.jpg *.jpeg *.bmp";
    const glm::vec2 TextureDefinitionInspectorEditor::_maxPreviewSize{ 200.F };

    TextureDefinitionInspectorEditor::RenderResult TextureDefinitionInspectorEditor::renderType(Texture::Source& src) noexcept
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


        auto& proj = getProject();
        auto& assets = getApp().getAssets();

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

        if (!_tex)
        {
            Texture::Definition def;
			TextureDefinitionWrapper defWrapper{ def };
            if (auto result = defWrapper.loadSource(src, assets.getAllocator()))
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