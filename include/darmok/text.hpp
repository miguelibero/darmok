#pragma once

#include <darmok/export.h>
#include <darmok/camera.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/utf8.hpp>
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
        glm::uvec2 size;
        glm::uvec2 position;
    };

    class Text;
    using TextContent = std::vector<Utf8Char>;

    class DARMOK_EXPORT BX_NO_VTABLE IFont
    {
    public:
        virtual ~IFont() = default;

        virtual std::optional<Glyph> getGlyph(const Utf8Char& chr) const = 0;
        virtual OptionalRef<const Texture> getTexture() const = 0;
        virtual void update() {};
        virtual void onTextContentChanged(Text& text, const TextContent& oldContent, const TextContent& newContent) {};
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
        Text(const std::shared_ptr<IFont>& font, const std::string& content = "") noexcept;
        ~Text();
        std::shared_ptr<IFont> getFont() noexcept;
        Text& setFont(const std::shared_ptr<IFont>& font) noexcept;
        std::string getContentString();
        Text& setContent(const std::string& str);
        Text& setContent(const std::u8string& str);
        void render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const;
    private:
        std::shared_ptr<IFont> _font;
        TextContent _content;
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
        OptionalRef<bx::AllocatorI> _alloc;
    };
}