#pragma once

#include <darmok/export.h>
#include <darmok/camera.hpp>
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
        glm::ivec2 offset = {};
        glm::uvec2 originalSize = {};
    };

    class Text;

    class DARMOK_EXPORT BX_NO_VTABLE IFont
    {
    public:
        virtual ~IFont() = default;

        virtual std::optional<Glyph> getGlyph(const Utf8Char& chr) const = 0;
        virtual const Material& getMaterial() const = 0;
        virtual float getLineSize() const = 0;
        virtual void update() {};
        virtual void addContent(const Utf8Vector& content) {};
        virtual void removeContent(const Utf8Vector& content) {};
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFontLoader
    {
    public:
        using result_type = std::shared_ptr<IFont>;

        virtual ~IFontLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    class DARMOK_EXPORT Text final
    {
    public:
        using Orientation = TextOrientation;
        Text(const std::shared_ptr<IFont>& font, const std::string& content = "") noexcept;
        ~Text();
        std::shared_ptr<IFont> getFont() noexcept;
        Text& setFont(const std::shared_ptr<IFont>& font) noexcept;
        std::string getContentString();
        Text& setContent(const std::string& str);
        Text& setContent(const std::u8string& str);
        Text& setContent(const Utf8Vector& content);
        const Color& getColor() const noexcept;
        Text& setColor(const Color& color) noexcept;
        const glm::vec2& getContentSize() const noexcept;
        Text& setContentSize(const glm::vec2& size) noexcept;
        Orientation getOrientation() const noexcept;
        Text& setOrientation(Orientation ori) noexcept;

        bool update();
        bool render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const;

        static MeshData createMeshData(const Utf8Vector& content, const IFont& font, const glm::vec2& size = glm::vec2(0), Orientation ori = Orientation::Left);
    private:
        std::shared_ptr<IFont> _font;
        Utf8Vector _content;
        std::optional<DynamicMesh> _mesh;
        Color _color;
        bool _changed;
        uint32_t _vertexNum;
        uint32_t _indexNum;
        glm::vec2 _contentSize;
        Orientation _orientation;

        void onContentChanged(const Utf8Vector& oldContent);
    };

    class DARMOK_EXPORT TextRenderer final : public ICameraComponent
    {
    public:
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) override;
        bgfx::ViewId afterRender(bgfx::ViewId viewId) override;
    private:
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        static const std::string _name;
    };

    struct TextureAtlas;
    class Program;

    class TextureAtlasFont final : public IFont
    {
    public:
        TextureAtlasFont(const std::shared_ptr<TextureAtlas>& atlas, const std::shared_ptr<Program>& prog) noexcept;
        std::optional<Glyph> getGlyph(const Utf8Char& chr) const noexcept override;
        const Material& getMaterial() const noexcept override;
        float getLineSize() const noexcept override;
    private:
        std::shared_ptr<TextureAtlas> _atlas;
        Material _material;
    };

    class ITextureAtlasLoader;
    class App;

    class TextureAtlasFontLoader final : public IFontLoader
    {
    public:
        TextureAtlasFontLoader(ITextureAtlasLoader& atlasLoader) noexcept;
        void init(App& app);
        void shutdown();
        std::shared_ptr<IFont> operator()(std::string_view name) override;
    private:
        ITextureAtlasLoader& _atlasLoader;
        std::shared_ptr<Program> _program;
    };
}