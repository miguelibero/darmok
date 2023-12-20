#include <darmok/data.hpp>
#include <bx/allocator.h>

namespace darmok
{
    Data::Data() noexcept
        : _ptr(nullptr)
        , _size(0)
        , _alloc(nullptr)
        , _own(false)
    {
    }

    Data::Data(void* ptr, uint64_t size, bool own) noexcept
        : _ptr(ptr)
        , _size(size)
        , _alloc(nullptr)
        , _own(own)
    {
    }

    Data::Data(void* ptr, uint64_t size, bx::AllocatorI* alloc, bool own) noexcept
        : _ptr(ptr)
        , _size(size)
        , _alloc(alloc)
        , _own(own)
    {
    }

    void* Data::ptr() const noexcept
    {
        return _ptr;
    }

    uint64_t Data::size() const noexcept
    {
        return _size;
    }

    bool Data::empty() const noexcept
    {
        return _size == 0ll;
    }

    Data::~Data() noexcept
    {
        clear();
    }

    void Data::clear() noexcept
    {
        if (_own)
        {
            if (_alloc == nullptr)
            {
                delete _ptr;
            }
            else if(_ptr != nullptr)
            {
                bx::free(_alloc, _ptr);
            }
        }
        _ptr = nullptr;
        _size = 0;
        _own = false;
    }

    Data::Data(const Data& other) noexcept
        : _ptr(other._ptr)
        , _size(other._size)
        , _alloc(other._alloc)
        , _own(other._own)
    {
        if (_own)
        {
            bx::memCopy(_ptr, other._ptr, _size);
        }
    }

    Data& Data::operator=(const Data& other) noexcept
    {
        clear();
        _size = other._size;
        _alloc = other._alloc;
        _own = other._own;
        if (_own && other._ptr)
        {
            bx::memCopy(_ptr, other._ptr, _size);
        }
        else
        {
            _ptr = other._ptr;
        }
        return *this;
    }

    Data::Data(Data&& other) noexcept
        : _ptr(other._ptr)
        , _size(other._size)
        , _alloc(other._alloc)
        , _own(other._own)
    {
        other._ptr = nullptr;
        other._size = 0;
        other._own = false;
    }

    Data& Data::operator=(Data&& other) noexcept
    {
        clear();
        _ptr = other._ptr;
        _size = other._size;
        _alloc = other._alloc;
        other._ptr = nullptr;
        other._size = 0;
        return *this;
    }

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