#pragma once

#include <CEGUI/ImageCodec.h>
#include <bx/allocator.h>

namespace darmok
{
    class CeguiImageCodec final : public CEGUI::ImageCodec
    {
    public:
        CeguiImageCodec(bx::AllocatorI* alloc) noexcept;
        CEGUI::Texture* load(const CEGUI::RawDataContainer& data, CEGUI::Texture* result) override;
    private:
        bx::AllocatorI* _alloc;
    };
}