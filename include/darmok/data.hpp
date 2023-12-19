#pragma once

#include <vector>
#include <bgfx/bgfx.h>

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