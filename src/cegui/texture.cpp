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
#include <CEGUI/ImageCodec.h>


namespace darmok
{
    const uint64_t CeguiTexture::_defaultFlags = BGFX_TEXTURE_BLIT_DST | defaultTextureLoadFlags;

    CeguiTexture::CeguiTexture(bx::AllocatorI* alloc, const CEGUI::String& name, uint64_t flags) noexcept
        : _name(name)
        , _alloc(alloc)
        , _texel(0)
        , _size(0, 0)
        , _flags(flags)
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
            CEGUI::System::getSingleton().getImageCodec().load(container, this);
        }
        catch (const std::exception& ex)
        {
            resourceProvider->unloadRawDataContainer(container);
            throw;
        }
        resourceProvider->unloadRawDataContainer(container);
    }

    void CeguiTexture::loadFromMemory(const void* buffer,
        const CEGUI::Sizef& buffer_size,
        PixelFormat pixel_format)
    {
        TextureConfig cfg;
        cfg.size = glm::uvec2(buffer_size.d_width, buffer_size.d_height);
        cfg.format = convertFormat(pixel_format);
        if (cfg.format == bgfx::TextureFormat::Unknown)
        {
            throw RendererException("texture format not supported");
        }
        DataView data(buffer, cfg.getInfo().storageSize);
        auto flags = _defaultFlags | _flags;
        _texture = std::make_unique<darmok::Texture>(data, cfg, flags);
        onTextureChanged();
    }

    void CeguiTexture::loadFromSize(const CEGUI::Sizef& buffer_size)
    {
        if (_size == buffer_size)
        {
            return;
        }
        auto glmSize = glm::uvec2(buffer_size.d_width, buffer_size.d_height);
        auto flags = _defaultFlags | _flags;
        TextureConfig cfg;
        cfg.size = glmSize;
        auto canReadBack = flags & BGFX_TEXTURE_READ_BACK;
        if (_texture == nullptr || !canReadBack)
        {
            _texture = std::make_unique<darmok::Texture>(cfg, flags);
        }
        else
        {
            Data data;
            _texture->read(data);
            _texture = std::make_unique<darmok::Texture>(data.view(), cfg, flags);
        }

        onTextureChanged();
    }

    const std::unordered_map<bgfx::TextureFormat::Enum, CEGUI::Texture::PixelFormat> CeguiTexture::_formatMap =
    {
        { bgfx::TextureFormat::RGB8, CEGUI::Texture::PixelFormat::Rgb },
        { bgfx::TextureFormat::RGBA8, CEGUI::Texture::PixelFormat::Rgba },
        { bgfx::TextureFormat::RGBA4, CEGUI::Texture::PixelFormat::Rgba4444 },
        { bgfx::TextureFormat::PTC12, CEGUI::Texture::PixelFormat::Pvrtc2 },
        { bgfx::TextureFormat::PTC14, CEGUI::Texture::PixelFormat::Pvrtc4 },
        { bgfx::TextureFormat::BC1, CEGUI::Texture::PixelFormat::RgbaDxt1 },
        { bgfx::TextureFormat::BC2, CEGUI::Texture::PixelFormat::RgbaDxt3 },
        { bgfx::TextureFormat::BC3, CEGUI::Texture::PixelFormat::RgbaDxt5 },
    };

    bgfx::TextureFormat::Enum CeguiTexture::convertFormat(CEGUI::Texture::PixelFormat format) noexcept
    {
        for (auto& elm : _formatMap)
        {
            if (elm.second == format)
            {
                return elm.first;
            }
        }
        return bgfx::TextureFormat::Unknown;
    }

    std::optional<CEGUI::Texture::PixelFormat> CeguiTexture::convertFormat(bgfx::TextureFormat::Enum format) noexcept
    {
        auto itr = _formatMap.find(format);
        if (itr == _formatMap.end())
        {
            return std::nullopt;
        }
        return itr->second;
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
        return convertFormat(fmt) != bgfx::TextureFormat::Unknown;
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