#include <darmok-editor/inspector/texture.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/image.hpp>

#include <imgui.h>

namespace darmok::editor
{
    std::string TextureInspectorEditor::getTitle() const noexcept
    {
        return "Texture";
    }

    const glm::vec2 TextureInspectorEditor::_maxPreviewSize{ 200.F };

    TextureInspectorEditor::RenderResult TextureInspectorEditor::renderType(Texture::Source& src) noexcept
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
        FileDialogOptions dialogOptions;
        dialogOptions.filters = { "*.png", "*.jpg", "*.jpeg", "*.bmp" };
        if (getApp().drawFileInput("Load Image", imgPath, dialogOptions))
        {
            if (auto dataResult = assets.getDataLoader()(imgPath))
            {
                auto encoding = Image::getEncodingForPath(imgPath);
                auto result = TextureSourceWrapper{ src }.loadData(dataResult.value(), encoding);
                if (!result)
                {
                    return unexpected<std::string>{ result.error() };
                }
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
                if (auto texResult = Texture::load(def))
                {
                    _tex = std::make_shared<Texture>(std::move(texResult).value());
                }
            }
        }

        if (_tex)
        {
			ImguiUtils::drawTexturePreview(*_tex, _maxPreviewSize);
        }

        return changed;
    }
}