#pragma once

#include <darmok/export.h>
#include <darmok/glm.hpp>
#include <bgfx/bgfx.h>
#include <memory>

namespace darmok
{
    class Texture;

    class DARMOK_EXPORT FrameBuffer final
    {
    public:
        FrameBuffer() noexcept;
        FrameBuffer(const glm::uvec2& size) noexcept;
        ~FrameBuffer() noexcept;
        const bgfx::FrameBufferHandle& getHandle() const noexcept;
    private:
        bgfx::FrameBufferHandle _handle;
    };
}