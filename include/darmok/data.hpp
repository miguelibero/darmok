#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <string_view>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
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

    class Data;

    class DataView final
    {
    public:
        DataView(const void* ptr = nullptr, size_t size = 0) noexcept;
        DataView(const Data& data) noexcept;
        DataView(const bgfx::Memory& mem) noexcept;
        const void* ptr() const noexcept;
        size_t size() const noexcept;
        bool empty() const noexcept;

    private:
        size_t _size;
        const void* _ptr;
    };

    class Data final
    {
    public:
        Data(size_t size = 0, bx::AllocatorI* alloc = nullptr) noexcept;
        Data(const void* ptr, size_t size, bx::AllocatorI* alloc = nullptr) noexcept;
        ~Data() noexcept;

        Data(const Data& other) noexcept;
        Data& operator=(const Data& other) noexcept;
        Data(Data&& other) noexcept;
        Data& operator=(Data&& other) noexcept;
        operator DataView() const;
        
        void* ptr() const noexcept;
        size_t size() const noexcept;
        bool empty() const noexcept;
        void clear() noexcept;
        const bgfx::Memory* makeRef() const noexcept;

        template<typename T>
        static Data copy(const std::vector<T>& v, bx::AllocatorI* alloc = nullptr) noexcept
        {
            return Data(&v.front(), sizeof(T) * v.size(), alloc);
        }

        template<typename T>
        static Data copy(const T* ptr, size_t num, bx::AllocatorI* alloc = nullptr) noexcept
        {
            return Data(ptr, sizeof(T) * num, alloc);
        }

        template<typename T>
        static Data copy(const T& v, bx::AllocatorI* alloc = nullptr) noexcept
        {
            return Data(&v, sizeof(T), alloc);
        }

    private:
        size_t _size;
        void* _ptr;
        bx::AllocatorI* _alloc;

        static void* malloc(size_t size, bx::AllocatorI* alloc) noexcept;
    };

	class BX_NO_VTABLE IDataLoader
	{
	public:
        using result_type = std::shared_ptr<Data>;

		virtual ~IDataLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
	};

}