#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>

namespace darmok::editor
{
    class FreetypeFontInspectorEditor final : public AssetObjectEditor<protobuf::FreetypeFont>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(protobuf::FreetypeFont& font) noexcept override;
    };

    class TextInspectorEditor final : public EntityComponentObjectEditor<Text>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Text::Definition& text) noexcept override;
    };

    class TextRendererInspectorEditor final : public CameraComponentObjectEditor<TextRenderer>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(TextRenderer::Definition& text) noexcept override;
    };
}