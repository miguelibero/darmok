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

    const std::string TextureDefinitionInspectorEditor::_imageFilter = "*.png *.jpg *.jpeg *.bmp";
    const glm::vec2 TextureDefinitionInspectorEditor::_maxPreviewSize(200.F);

    bool TextureDefinitionInspectorEditor::renderType(Texture::Source& src) noexcept
    {
        auto changed = false;

        if (ImGui::CollapsingHeader("Texture"))
        {
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
                    }
                }
            }

            if (!_tex)
            {
                Texture::Definition def;
                auto loadResult = TextureUtils::loadSource(def, src, _app->getAssets().getAllocator());
                if (loadResult)
                {
					_tex.emplace(def);
                }
            }

            if (_tex)
            {
                ImguiTextureData texData{ _tex->getHandle() };
                glm::vec2 size{ _tex->getSize() };
                auto availSize = ImGui::GetContentRegionAvail();
                glm::vec2 maxSize{ glm::min(_maxPreviewSize.x, availSize.x), glm::min(_maxPreviewSize.y, availSize.y) };
                auto ratio = glm::min(size.x / maxSize.x, size.y / maxSize.y);
                size /= ratio;
                ImGui::Image(texData, ImVec2{ size.x, size.y });
            }

            ImGui::SameLine();

            if (ImGui::Button("Delete"))
            {
                // proj.removeTexture(def);
                changed = true;
            }
        }
        return false;
    }
}