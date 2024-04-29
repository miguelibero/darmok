#include "texture.hpp"
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
    CeguiTexture::CeguiTexture(bx::AllocatorI* alloc, const CEGUI::String& name) noexcept
        : _name(name), _alloc(alloc)
    {
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
            _texture = std::make_unique<darmok::Texture>(img);
        }
        catch (const std::exception& ex)
        {
            resourceProvider->unloadRawDataContainer(container);
            throw;
        }
        updateTexture();
        resourceProvider->unloadRawDataContainer(container);
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
            throw std::runtime_error("texture format not supported");
            break;
        }
        DataView data(buffer, cfg.getInfo().storageSize);
        _texture = std::make_unique<darmok::Texture>(data, cfg);
        updateTexture();
    }

    void CeguiTexture::loadFromSize(const CEGUI::Sizef& buffer_size)
    {
        TextureConfig cfg;
        cfg.size = glm::uvec2(buffer_size.d_width, buffer_size.d_height);
        _texture = std::make_unique<darmok::Texture>(cfg);
        updateTexture();
    }

    void CeguiTexture::updateTexture() noexcept
    {
        if (_texture != nullptr)
        {
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
        auto memSize = size.x * size.y * _texture->getBitsPerPixel();
        _texture->update(DataView(sourceData, memSize), origin, size);
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
}