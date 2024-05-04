#include "image.hpp"
#include "texture.hpp"
#include <CEGUI/Texture.h>
#include <CEGUI/DataContainer.h>
#include <CEGUI/Sizef.h>
#include <darmok/image.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    CeguiImageCodec::CeguiImageCodec(bx::AllocatorI* alloc) noexcept
        : ImageCodec("darmok"), _alloc(alloc)
    {
    }

    CEGUI::Texture* CeguiImageCodec::load(const CEGUI::RawDataContainer& data, CEGUI::Texture* result)
    {
        Image img(DataView(data.getDataPtr(), data.getSize()), _alloc);
        auto size = img.getSize();

        auto format = CeguiTexture::convertFormat(bgfx::TextureFormat::Enum(img.getFormat()));
        if (!format)
        {
            throw std::runtime_error("could not convert image format to CEGUI");
        }

        result->loadFromMemory(img.getData().ptr(), CEGUI::Sizef(size.x, size.y), format.value());

        return result;
    }
}

#include "CEGUI/ImageCodecModules/STB/ImageCodecModule.h" 

CEGUI::ImageCodec* createImageCodec(void)
{
    static bx::DefaultAllocator alloc;
    return new darmok::CeguiImageCodec(&alloc);
}


void destroyImageCodec(CEGUI::ImageCodec* imageCodec)
{
    delete imageCodec;
}