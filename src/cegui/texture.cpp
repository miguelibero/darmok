#include "texture.hpp"
#include "utils.hpp"
#include <bimg/bimg.h>
#include <bx/bx.h>
#include <exception>

#include <darmok/texture.hpp>
#include <darmok/image.hpp>
#include <darmok/data.hpp>
#include <CEGUI/Sizef.h>
#include <CEGUI/Rectf.h>
#include <CEGUI/String.h>
#include <CEGUI/System.h>
#include <CEGUI/DataContainer.h>
#include <CEGUI/ResourceProvider.h>


namespace darmok
{
    const uint64_t CeguiTexture::_defaultFlags = BGFX_TEXTURE_BLIT_DST;

    CeguiTexture::CeguiTexture(bx::AllocatorI* alloc, const CEGUI::String& name, uint64_t flags) noexcept
        : _name(name)
        , _alloc(alloc)
        , _texel(0)
        , _size(0, 0)
        , _flags(flags)
    {
    }

    CeguiTexture::CeguiTexture(std::unique_ptr<darmok::Texture>&& texture, bx::AllocatorI* alloc, const CEGUI::String& name, uint64_t flags) noexcept
        : _name(name)
        , _alloc(alloc)
        , _texel(0)
        , _size(0, 0)
        , _texture(std::move(texture))
        , _flags(flags)
    {
        onTextureChanged();
    }

    CeguiTexture::~CeguiTexture() noexcept
    {
        // intentionally left black for std::unique_ptr<darmok::Texture>
    }

    const CEGUI::String& CeguiTexture::getName() const
    {
        return _name;
    }

    const CEGUI::Sizef& CeguiTexture::getSize() const
    {
        return _size;
    }

    const CEGUI::Sizef& CeguiTexture::getOriginalDataSize() const
    {
        // TODO: probably depends on the resource group?
        // may have different qualities
        return getSize();
    }

    const glm::vec2& CeguiTexture::getTexelScaling() const
    {
        return _texel;
    }

    void CeguiTexture::loadFromFile(const CEGUI::String& filename,
        const CEGUI::String& resourceGroup)
    {
        auto resourceProvider = CEGUI::System::getSingleton().getResourceProvider();
        CEGUI::RawDataContainer container;
        resourceProvider->loadRawDataContainer(filename, container, resourceGroup);

        try
        {
            Image img(DataView(container.getDataPtr(), container.getSize()), bimg::TextureFormat::Count, _alloc);
            auto flags = _defaultFlags | _flags;
            _texture = std::make_unique<darmok::Texture>(img, flags);
        }
        catch (const std::exception& ex)
        {
            resourceProvider->unloadRawDataContainer(container);
            throw;
        }
        onTextureChanged();
        resourceProvider->unloadRawDataContainer(container);
    }

    void CeguiTexture::loadFromSize(const CEGUI::Sizef& buffer_size)
    {
        TextureConfig cfg;
        cfg.size = glm::uvec2(buffer_size.d_width, buffer_size.d_height);
        auto flags = _defaultFlags | _flags;
        _texture = std::make_unique<darmok::Texture>(cfg, flags);
        onTextureChanged();
    }

    void CeguiTexture::loadFromMemory(const void* buffer,
        const CEGUI::Sizef& buffer_size,
        PixelFormat pixel_format)
    {
        TextureConfig cfg;
        cfg.size = glm::uvec2(buffer_size.d_width, buffer_size.d_height);
        switch (pixel_format)
        {
        case PixelFormat::Rgb:
            cfg.format = bgfx::TextureFormat::RGB8;
            break;
        case PixelFormat::Rgba:
            cfg.format = bgfx::TextureFormat::RGBA8;
            break;
        case PixelFormat::Rgba4444:
            cfg.format = bgfx::TextureFormat::RGBA4;
            break;
        case PixelFormat::Pvrtc2:
            cfg.format = bgfx::TextureFormat::PTC12;
            break;
        case PixelFormat::Pvrtc4:
            cfg.format = bgfx::TextureFormat::PTC14;
            break;
        case PixelFormat::RgbaDxt1:
            cfg.format = bgfx::TextureFormat::BC1;
            break;
        case PixelFormat::RgbaDxt3:
            cfg.format = bgfx::TextureFormat::BC2;
            break;
        case PixelFormat::RgbaDxt5:
            cfg.format = bgfx::TextureFormat::BC3;
            break;
        default:
            throw RendererException("texture format not supported");
            break;
        }
        DataView data(buffer, cfg.getInfo().storageSize);
        auto flags = _defaultFlags | _flags;
        _texture = std::make_unique<darmok::Texture>(data, cfg, flags);
        onTextureChanged();
    }

    void CeguiTexture::onTextureChanged() noexcept
    {
        if (_texture != nullptr)
        {
            _texture->setName(CeguiUtils::convert(_name));
            auto& size = _texture->getSize();
            _texel = glm::vec2(1) / glm::vec2(size);
            _size = CEGUI::Sizef(size.x, size.y);
        }
        else
        {
            _texel = glm::vec2(0);
            _size = CEGUI::Sizef(0, 0);
        }
    }

    void CeguiTexture::blitFromMemory(const void* sourceData, const CEGUI::Rectf& area)
    {
        if (_texture == nullptr)
        {
            return;
        }
        auto size = glm::uvec2(area.getWidth(), area.getHeight());
        auto origin = glm::uvec2(area.d_min.x, area.d_min.y);
        auto memSize = size.x * size.y * _texture->getBitsPerPixel() / 8;
        _texture->update(DataView(sourceData, memSize), size, origin);
    }

    void CeguiTexture::blitToMemory(void* targetData)
    {
        Data data;
        _texture->read(data);
        targetData = data.ptr();
        data.release();
    }

    bool CeguiTexture::isPixelFormatSupported(const PixelFormat fmt) const
    {
        return true;
    }

    OptionalRef<darmok::Texture> CeguiTexture::getDarmokTexture() const noexcept
    {
        return _texture == nullptr ? nullptr : _texture.get();
    }

    bgfx::TextureHandle CeguiTexture::getBgfxHandle() const noexcept
    {
        return _texture == nullptr ? bgfx::TextureHandle{ bgfx::kInvalidHandle } : _texture->getHandle();
    }
}