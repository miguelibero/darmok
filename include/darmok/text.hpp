#pragma once

#include <darmok/export.h>
#include <darmok/camera.hpp>
#include <memory>
#include <string>
#include <bx/bx.h>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class FontImpl;

    class DARMOK_EXPORT Font final
    {
    public:
        Font(std::unique_ptr<FontImpl>&& impl) noexcept;
        ~Font();
        FontImpl& getImpl();
        const FontImpl& getImpl() const;
    private:
        std::unique_ptr<FontImpl> _impl;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFontLoader
    {
    public:
        using result_type = std::shared_ptr<Font>;

        virtual ~IFontLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    class TextImpl;

    class DARMOK_EXPORT Text final
    {
    public:
        Text(const std::shared_ptr<Font>& font, const std::string& content = "") noexcept;
        ~Text();
        TextImpl& getImpl();
        const TextImpl& getImpl() const;
    private:
        std::unique_ptr<TextImpl> _impl;
    };

    class TextRendererImpl;

    class DARMOK_EXPORT TextRenderer final : public ICameraComponent
    {
    public:
        TextRenderer() noexcept;
        ~TextRenderer() noexcept;
        void init(Camera& cam, Scene& scene, App& app) noexcept override;
        void shutdown() noexcept override;
        void update(float deltaTime) override;
        bgfx::ViewId afterRender(bgfx::ViewId viewId) override;
    private:
        std::unique_ptr<TextRendererImpl> _impl;
    };

    class Image;

    class FontAtlasGeneratorImpl;

    class DARMOK_EXPORT FontAtlasGenerator final
    {
    public:
        FontAtlasGenerator(const Font& font, bx::AllocatorI& alloc) noexcept;
        ~FontAtlasGenerator() noexcept;
        Image operator()(std::string_view chars);
    private:
        std::unique_ptr<FontAtlasGeneratorImpl> _impl;
    };
}