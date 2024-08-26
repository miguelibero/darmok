#include <darmok/framebuffer.hpp>

namespace darmok
{
    FrameBuffer::FrameBuffer() noexcept
        : _handle(bgfx::createFrameBuffer(bgfx::BackbufferRatio::Equal, bgfx::TextureFormat::RGBA16F))
    {
    }

    FrameBuffer::FrameBuffer(const glm::uvec2& size) noexcept
        : _handle(bgfx::createFrameBuffer(size.x, size.y, bgfx::TextureFormat::RGBA16F, bgfx::TextureFormat::D16))
    {
    }

    FrameBuffer::~FrameBuffer() noexcept
    {
        bgfx::destroy(_handle);
    }

    const bgfx::FrameBufferHandle& FrameBuffer::getHandle() const noexcept
    {
        return _handle;
    }
}