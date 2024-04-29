#pragma once

#include <CEGUI/Texture.h>
#include <CEGUI/String.h>
#include <CEGUI/Sizef.h>
#include <bgfx/bgfx.h>
#include <memory>

namespace darmok
{
    class Texture;

    class CeguiTexture final : public CEGUI::Texture
    {
    public:
        CeguiTexture(bx::AllocatorI* alloc, const CEGUI::String& name = "") noexcept;
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
    private:
        CEGUI::String _name;
        bx::AllocatorI* _alloc;
        std::unique_ptr<darmok::Texture> _texture;
        glm::vec2 _texel;
        CEGUI::Sizef _size;

        void updateTexture() noexcept;
    };
}