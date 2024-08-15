#pragma once

#include <darmok/export.h>
#include <darmok/render.hpp>
#include <darmok/render_graph.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/utf8.hpp>
#include <darmok/mesh.hpp>
#include <darmok/color.hpp>
#include <darmok/material.hpp>
#include <darmok/text_fwd.hpp>

#include <memory>
#include <string>
#include <optional>
#include <vector>
#include <unordered_set>
#include <bx/bx.h>
#include <bgfx/bgfx.h>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    struct DARMOK_EXPORT Glyph final
    {
        glm::uvec2 size = {};
        glm::uvec2 texturePosition = {};
        glm::vec2 offset = {};
        glm::vec2 originalSize = {};
    };

    class Text;

    class DARMOK_EXPORT BX_NO_VTABLE IFont
    {
    public:
        virtual ~IFont() = default;

        virtual std::optional<Glyph> getGlyph(const Utf8Char& chr) const = 0;
        virtual std::shared_ptr<Texture> getTexture() const = 0;
        virtual float getLineSize() const = 0;
        virtual void update(const std::unordered_set<Utf8Char>& chars) {};
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFontLoader
    {
    public:
        using result_type = std::shared_ptr<IFont>;

        virtual ~IFontLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

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
        ~Text();
        std::shared_ptr<IFont> getFont() noexcept;
        Text& setFont(const std::shared_ptr<IFont>& font) noexcept;
        std::string getContentString() const;
        const Utf8Vector& getContent() const noexcept;
        Text& setContent(const std::string& str);
        Text& setContent(const std::u8string& str);
        Text& setContent(const Utf8Vector& content);
        const Color& getColor() const noexcept;
        Text& setColor(const Color& color) noexcept;
        
        const RenderConfig& getRenderConfig() const noexcept;
        RenderConfig& getRenderConfig() noexcept;
        Text& setRenderConfig(const RenderConfig& config) noexcept;
        const glm::uvec2& getContentSize() const noexcept;
        Text& setContentSize(const glm::uvec2& size) noexcept;
        Orientation getOrientation() const noexcept;
        Text& setOrientation(Orientation ori) noexcept;

        bool update(const bgfx::VertexLayout& layout);
        bool render(bgfx::ViewId viewId, bgfx::Encoder& encoder) const;

        static MeshData createMeshData(const Utf8Vector& content, const IFont& font, const RenderConfig& config = {});
    private:
        std::shared_ptr<IFont> _font;
        Utf8Vector _content;
        std::optional<DynamicMesh> _mesh;
        Color _color;
        bool _changed;
        uint32_t _vertexNum;
        uint32_t _indexNum;
        RenderConfig _renderConfig;
    };

    // TODO: add PIMPL class
    class DARMOK_EXPORT TextRenderer final : public IRenderer, IRenderPass
    {
    public:
        TextRenderer(const std::shared_ptr<Program>& prog = nullptr) noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) override;
        void renderReset() noexcept override;
    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        bgfx::ViewId _viewId;
        std::shared_ptr<Program> _prog;
        bgfx::UniformHandle _colorUniform;
        bgfx::UniformHandle _textureUniform;

        void renderPassDefine(RenderPassDefinition& def) noexcept override;
        void renderPassConfigure(bgfx::ViewId viewId) noexcept override;
        void renderPassExecute(IRenderGraphContext& context) noexcept override;
    };

    struct TextureAtlas;
    class Program;

    class TextureAtlasFont final : public IFont
    {
    public:
        TextureAtlasFont(const std::shared_ptr<TextureAtlas>& atlas) noexcept;
        std::optional<Glyph> getGlyph(const Utf8Char& chr) const noexcept override;
        float getLineSize() const noexcept override;
        std::shared_ptr<Texture> getTexture() const override;
    private:
        std::shared_ptr<TextureAtlas> _atlas;
    };

    class ITextureAtlasLoader;
    class App;

    class TextureAtlasFontLoader final : public IFontLoader
    {
    public:
        TextureAtlasFontLoader(ITextureAtlasLoader& atlasLoader) noexcept;
        std::shared_ptr<IFont> operator()(std::string_view name) override;
    private:
        ITextureAtlasLoader& _atlasLoader;
    };
}