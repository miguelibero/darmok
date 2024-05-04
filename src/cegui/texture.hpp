#pragma once

#include <CEGUI/Texture.h>
#include <CEGUI/String.h>
#include <CEGUI/Sizef.h>
#include <bgfx/bgfx.h>
#include <memory>
#include <unordered_map>
#include <optional>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Texture;
    class Image;

    class CeguiTexture final : public CEGUI::Texture
    {
    public:
        CeguiTexture(bx::AllocatorI* alloc = nullptr, const CEGUI::String& name = "", uint64_t flags = 0) noexcept;
        ~CeguiTexture() noexcept;
        const CEGUI::String& getName() const override;
        const CEGUI::Sizef& getSize() const override;
        const CEGUI::Sizef& getOriginalDataSize() const override;
        const glm::vec2& getTexelScaling() const override;
        
        void loadFromFile(const CEGUI::String& filename,
            const CEGUI::String& resourceGroup) override;
        void loadFromMemory(const void* buffer,
            const CEGUI::Sizef& buffer_size,
            PixelFormat pixel_format) override;
        void loadFromSize(const CEGUI::Sizef& buffer_size);

        void blitFromMemory(const void* sourceData, const CEGUI::Rectf& area) override;
        void blitToMemory(void* targetData) override;
        bool isPixelFormatSupported(const PixelFormat fmt) const override;

        OptionalRef<darmok::Texture> getDarmokTexture() const noexcept;
        bgfx::TextureHandle getBgfxHandle() const noexcept;

        static bgfx::TextureFormat::Enum convertFormat(CEGUI::Texture::PixelFormat format) noexcept;
        static std::optional<CEGUI::Texture::PixelFormat> convertFormat(bgfx::TextureFormat::Enum format) noexcept;

    private:
        static const uint64_t _defaultFlags;
        static const std::unordered_map<bgfx::TextureFormat::Enum, CEGUI::Texture::PixelFormat> _formatMap;
        uint64_t _flags;
        CEGUI::String _name;
        bx::AllocatorI* _alloc;
        std::unique_ptr<darmok::Texture> _texture;
        glm::vec2 _texel;
        CEGUI::Sizef _size;

        void onTextureChanged() noexcept;
    };
}