#pragma once

#include <darmok/export.h>
#include <darmok/render_scene.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/mesh.hpp>
#include <darmok/color.hpp>
#include <darmok/material.hpp>
#include <darmok/text_fwd.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/texture_atlas.pb.h>
#include <darmok/protobuf/text.pb.h>

#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <unordered_set>

#include <bx/bx.h>
#include <bgfx/bgfx.h>

namespace darmok
{
    class DARMOK_EXPORT BX_NO_VTABLE IFont
    {
    public:
		using Glyph = protobuf::TextureAtlasElement;

        virtual ~IFont() = default;

        virtual std::optional<Glyph> getGlyph(char32_t chr) const noexcept = 0;
        virtual std::shared_ptr<Texture> getTexture() const noexcept = 0;
        virtual float getLineSize() const noexcept = 0;
        [[nodiscard]] virtual expected<void, std::string> update(const std::unordered_set<char32_t>& chars) noexcept
        {
            return {};
        };
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFontLoader : public ILoader<IFont>{};

    struct DARMOK_EXPORT TextRenderConfig
    {
        using Axis = TextLayoutAxis;
        using Direction = TextLayoutDirection;
        using Orientation = TextOrientation;
        Axis axis = Axis::Horizontal;
        Direction direction = Direction::Positive;
        Direction lineDirection = Direction::Negative;
        Orientation orientation = Orientation::Left;
        glm::uvec2 contentSize = glm::uvec2(0);

        glm::vec2 getGlyphAdvanceFactor() const noexcept;
        bool fixEndOfLine(glm::vec2& pos, float lineStep) const;
    };

    class DARMOK_EXPORT Text final
    {
    public:
        using Orientation = TextOrientation;
        using RenderConfig = TextRenderConfig;

        Text(const std::shared_ptr<IFont>& font, const std::string& content = "") noexcept;
        std::shared_ptr<IFont> getFont() noexcept;
        Text& setFont(const std::shared_ptr<IFont>& font) noexcept;
        std::string getContentString() const;
        const std::u32string& getContent() const noexcept;
        Text& setContent(const std::string& str);
        Text& setContent(const std::u32string& str);
        const Color& getColor() const noexcept;
        Text& setColor(const Color& color) noexcept;

        const RenderConfig& getRenderConfig() const noexcept;
        RenderConfig& getRenderConfig() noexcept;
        Text& setRenderConfig(const RenderConfig& config) noexcept;
        const glm::uvec2& getContentSize() const noexcept;
        Text& setContentSize(const glm::uvec2& size) noexcept;
        Orientation getOrientation() const noexcept;
        Text& setOrientation(Orientation ori) noexcept;

        expected<void, std::string> update(const bgfx::VertexLayout& layout) noexcept;
        expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept;

        static MeshData createMeshData(std::u32string_view content, const IFont& font, const RenderConfig& config = {});
    private:
        std::shared_ptr<IFont> _font;
        std::u32string _content;
        std::optional<Mesh> _mesh;
        Color _color;
        bool _changed;
        uint32_t _vertexNum;
        uint32_t _indexNum;
        RenderConfig _renderConfig;
    };

    class DARMOK_EXPORT TextRenderer final : public ITypeCameraComponent<TextRenderer>
    {
    public:
        using Definition = protobuf::TextRenderer;
        TextRenderer(const std::shared_ptr<Program>& prog = nullptr) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> load(const Definition& def) noexcept;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
        static Definition createDefinition() noexcept;
    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        std::shared_ptr<Program> _prog;
        UniformHandle _colorUniform;
        UniformHandle _textureUniform;
    };

    struct TextureAtlas;
    class Program;

    class DARMOK_EXPORT TextureAtlasFont final : public IFont
    {
    public:
        TextureAtlasFont(const std::shared_ptr<TextureAtlas>& atlas) noexcept;
        std::optional<Glyph> getGlyph(char32_t chr) const noexcept override;
        float getLineSize() const noexcept override;
        std::shared_ptr<Texture> getTexture() const noexcept override;
    private:
        std::shared_ptr<TextureAtlas> _atlas;
    };

    class ITextureAtlasLoader;

    class DARMOK_EXPORT TextureAtlasFontLoader final : public IFontLoader
    {
    public:
        TextureAtlasFontLoader(ITextureAtlasLoader& atlasLoader) noexcept;
        Result operator()(std::filesystem::path path) noexcept override;
    private:
        ITextureAtlasLoader& _atlasLoader;
    };
}
