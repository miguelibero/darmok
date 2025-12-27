#include <darmok-editor/inspector/text.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/text_freetype.hpp>

namespace darmok::editor
{
    std::string FreetypeFontInspectorEditor::getTitle() const noexcept
    {
        return "Freetype Font";
    }

    FreetypeFontInspectorEditor::RenderResult FreetypeFontInspectorEditor::renderType(protobuf::FreetypeFont& font) noexcept
    {
        return {};
    }

    std::string TextInspectorEditor::getTitle() const noexcept
    {
        return "Text";
    }

    TextInspectorEditor::RenderResult TextInspectorEditor::renderType(Text::Definition& text) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Content", "content", text))
        {
            changed = true;
        }

        auto fontDragType = getApp().getAssetDragType<protobuf::FreetypeFont>().value_or("");
        auto result = ImguiUtils::drawProtobufAssetReferenceInput("Font", "font_path", text, fontDragType.c_str());
        if (result == ReferenceInputAction::Changed)
        {
            changed = true;
        }

        if (ImguiUtils::drawProtobufInput("Color", "color", text))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Axis", "axis", text))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Direction", "direction", text))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Line Direction", "line_direction", text))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Orientation", "orientation", text))
        {
            changed = true;
        }
        if (ImguiUtils::drawProtobufInput("Content Size", "content_size", text))
        {
            changed = true;
        }

        return changed;
    }

    std::string TextRendererInspectorEditor::getTitle() const noexcept
    {
        return "Text Renderer";
    }

    TextRendererInspectorEditor::RenderResult TextRendererInspectorEditor::renderType(TextRenderer::Definition& text) noexcept
    {
        return {};
    }

}