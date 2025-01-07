#include <darmok-editor/inspector/texture.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/image.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

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

    bool TextureDefinitionInspectorEditor::renderType(TextureDefinition& def) noexcept
    {
        auto changed = false;

        if (ImGui::CollapsingHeader("Texture"))
        {
            {
                if (ImGui::InputText("Name", &def.name))
                {
                    changed = true;
                }
                ImGui::Spacing();
            }

            if (_app)
            {
                auto& proj = _app->getProject();
                auto& assets = _app->getAssets();

                std::filesystem::path imgPath;
                if (ImguiUtils::drawFileInput("Load Image", imgPath, _imageFilter))
                {
                    auto& imgLoader = assets.getImageLoader();
                    if (auto img = imgLoader(imgPath))
                    {
                        def.loadImage(*img);
                        changed = true;
                    }
                }

                auto defPtr = proj.findTexture(def);
                if (defPtr)
                {
                    auto& texLoader = assets.getTextureLoader();
                    auto tex = texLoader.loadResource(defPtr);
                    ImguiTextureData texData(tex->getHandle());
                    auto size = glm::vec2(tex->getSize());
                    auto availSize = ImGui::GetContentRegionAvail();
                    glm::vec2 maxSize(glm::min(_maxPreviewSize.x, availSize.x), glm::min(_maxPreviewSize.y, availSize.y));
                    auto ratio = glm::min(size.x / maxSize.x, size.y / maxSize.y);
                    size /= ratio;
                    ImGui::Image(texData, ImVec2{ size.x, size.y });
                }

                if (ImGui::Button("Apply Changes"))
                {
                    proj.reloadTexture(def);
                }

                ImGui::SameLine();

                if (ImGui::Button("Delete"))
                {
                    proj.removeTexture(def);
                    changed = true;
                }
            }
        }
        return false;
    }
}