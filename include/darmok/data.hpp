#pragma once

#include <vector>
#include <stdexcept>
#include <bgfx/bgfx.h>
#include <bx/allocator.h>

namespace darmok
{
    template<typename T>
    static const bgfx::Memory* makeVectorRef(const std::vector<T>& v)
    {
        if (v.empty())
        {
            return nullptr;
        }
        return bgfx::makeRef(&v.front(), v.size() * sizeof(T));
    }

    class Data final
    {
    public:
        Data() noexcept;
        Data(void* ptr, size_t size, bool own = true) noexcept;
        Data(void* ptr, size_t size, bx::AllocatorI* alloc, bool own = true) noexcept;
        ~Data() noexcept;
        Data(const Data& other) noexcept;
        Data& operator=(const Data& other) noexcept;
        Data(Data&& other) noexcept;
        Data& operator=(Data&& other) noexcept;
        void* ptr() const noexcept;
        size_t size() const noexcept;
        bool empty() const noexcept;
        void clear() noexcept;
        const bgfx::Memory* makeRef() const noexcept;

        template<typename T>
        static Data copy(const T& v, bx::AllocatorI* alloc = nullptr)
        {
            void* ptr;
            size_t size = sizeof(T);
            if (alloc != nullptr)
            {
                ptr = bx::alloc(alloc, size);
            }
            else
            {
                ptr = std::malloc(size);
            }
            bx::memCopy(ptr, &v, size);
            return Data(ptr, size, true);
        }

        template<>
        static Data copy<Data>(const Data& v, bx::AllocatorI* alloc)
        {
            void* ptr;
            size_t size = v.size();
            if (alloc != nullptr)
            {
                ptr = bx::alloc(alloc, size);
            }
            else
            {
                ptr = std::malloc(size);
            }
            bx::memCopy(ptr, v.ptr(), size);
            return Data(ptr, size, alloc, true);
        }

        template<typename T>
        const T& as() const
        {
            if (_size < sizeof(T))
            {
                throw std::runtime_error("size too small");
            }
            return (T&)*_ptr;
        }

        template<typename T>
        T& as()
        {
            if (_size < sizeof(T))
            {
                throw std::runtime_error("size too small");
            }
            return (T&)*_ptr;
        }

    private:

        void* _ptr;
        size_t _size;
        bx::AllocatorI* _alloc;
        bool _own;
    };

    class VertexBuffer final
    {
    public:
        template<typename T>
        VertexBuffer(const std::vector<T>& vector, bgfx::VertexLayout layout) noexcept
            :VertexBuffer(makeVectorRef(vector), layout)
        {
        }

        template<typename T>
        void set(const std::vector<T>& vector) noexcept
        {
            set(makeVectorRef(vector));
        }

        VertexBuffer(bgfx::VertexLayout layout) noexcept;
        VertexBuffer(VertexBuffer&& other) noexcept;
        VertexBuffer& operator=(VertexBuffer&& other) noexcept;
        ~VertexBuffer() noexcept;
        
        const bgfx::VertexBufferHandle& getHandle() const noexcept;
        void clear() noexcept;


    private:
        bgfx::VertexLayout _layout;
        bgfx::VertexBufferHandle _handle = { bgfx::kInvalidHandle };

        VertexBuffer(const bgfx::Memory* mem, bgfx::VertexLayout layout) noexcept;
        void set(const bgfx::Memory* mem) noexcept;

        VertexBuffer(const VertexBuffer& other) = delete;
        VertexBuffer& operator=(const VertexBuffer& other) = delete;
    };

    typedef uint16_t VertexIndex;

    class IndexBuffer final
    {
    public:
        IndexBuffer() noexcept;
        IndexBuffer(const std::vector<VertexIndex>& vector) noexcept;
        IndexBuffer(IndexBuffer&& other) noexcept;
        IndexBuffer& operator=(IndexBuffer&& other) noexcept;
        ~IndexBuffer() noexcept;

        const bgfx::IndexBufferHandle& getHandle() const noexcept;
        void clear() noexcept;
        void set(const std::vector<VertexIndex>& vector) noexcept;
    private:
        bgfx::IndexBufferHandle _handle;

        IndexBuffer(const bgfx::Memory* mem = nullptr) noexcept;
        void set(const bgfx::Memory* mem) noexcept;

        IndexBuffer(const IndexBuffer& other) = delete;
        IndexBuffer& operator=(const IndexBuffer& other) = delete;
    };

}