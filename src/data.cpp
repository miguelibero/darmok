#include <darmok/data.hpp>

namespace darmok
{
    VertexBuffer::VertexBuffer(const bgfx::Memory* mem, bgfx::VertexLayout layout) noexcept
        : _layout(layout)
    {
        if(mem != nullptr)
        {
            _handle = bgfx::createVertexBuffer(mem, _layout);
        }
    }

    VertexBuffer::VertexBuffer(bgfx::VertexLayout layout) noexcept
        : VertexBuffer(nullptr, layout)
    {
    }

    VertexBuffer::~VertexBuffer() noexcept
    {
        clear();
    }

    VertexBuffer::VertexBuffer(VertexBuffer&& other) noexcept
        : _layout(other._layout)
        , _handle(other._handle)
    {
        other._handle = { bgfx::kInvalidHandle };

    }

    VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) noexcept
    {
        _handle = other._handle;
        _layout = other._layout;
        other._handle = { bgfx::kInvalidHandle };
        return *this;
    }

    void VertexBuffer::clear() noexcept
    {
        bgfx::destroy(_handle);
        _handle = { bgfx::kInvalidHandle };
    }
    
    const bgfx::VertexBufferHandle& VertexBuffer::getHandle() const noexcept
    {
        return _handle;
    }


    void VertexBuffer::set(const bgfx::Memory* mem) noexcept
    {
        clear();
        if(mem != nullptr)
        {
            _handle = bgfx::createVertexBuffer(mem, _layout);
        }
        else
        {
            _handle = { bgfx::kInvalidHandle };
        }
    }

    IndexBuffer::IndexBuffer() noexcept
        : _handle{ bgfx::kInvalidHandle }
    {
    }

    IndexBuffer::IndexBuffer(const std::vector<VertexIndex>& vector) noexcept
        : _handle(bgfx::createIndexBuffer(makeVectorRef(vector)))
    {
    }

    IndexBuffer::IndexBuffer(IndexBuffer&& other) noexcept
        : _handle(other._handle)
    {
        other._handle = { bgfx::kInvalidHandle };
    }

    IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other) noexcept
    {
        _handle = other._handle;
        other._handle = { bgfx::kInvalidHandle };
        return *this;
    }

    void IndexBuffer::set(const std::vector<VertexIndex>& vector) noexcept
    {
        bgfx::destroy(_handle);
        _handle = bgfx::createIndexBuffer(makeVectorRef(vector));
    }

    IndexBuffer::IndexBuffer(const bgfx::Memory* mem) noexcept
    {
        if(mem != nullptr)
        {
            _handle = bgfx::createIndexBuffer(mem);
        }
    }

    IndexBuffer::~IndexBuffer() noexcept
    {
        clear();
    }

    void IndexBuffer::clear() noexcept
    {
        bgfx::destroy(_handle);
        _handle = { bgfx::kInvalidHandle };
    }
    
    const bgfx::IndexBufferHandle& IndexBuffer::getHandle() const noexcept
    {
        return _handle;
    }

    void IndexBuffer::set(const bgfx::Memory* mem) noexcept
    {
        clear();
        if(mem != nullptr)
        {
            _handle = bgfx::createIndexBuffer(mem);
        }
        else
        {
            _handle = { bgfx::kInvalidHandle };
        }
    }
}