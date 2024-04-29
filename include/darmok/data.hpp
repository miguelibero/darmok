#pragma once

#include <vector>
#include <stdexcept>
#include <memory>
#include <string_view>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <bx/allocator.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
    template<typename T>
    static const bgfx::Memory* makeVectorRef(const std::vector<T>& v) noexcept
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
        DataView(const char* str) noexcept;
        DataView(const Data& data) noexcept;
        DataView(const bgfx::Memory& mem) noexcept;
        [[nodiscard]] const void* ptr() const noexcept;
        [[nodiscard]] size_t size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] bool operator==(const DataView& other) const noexcept;
        [[nodiscard]] bool operator!=(const DataView& other) const noexcept;

        [[nodiscard]] DataView view(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* makeRef(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* copyMem(size_t offset = 0, size_t size = -1) const noexcept;

    private:
        const void* _ptr;
        size_t _size;

        size_t fixOffset(size_t offset, void*& ptr) const noexcept;
        size_t fixSize(size_t size, size_t offset = 0) const noexcept;
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
        
        [[nodiscard]] void* ptr() const noexcept;
        [[nodiscard]] size_t size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] bool operator==(const Data& other) const noexcept;
        [[nodiscard]] bool operator!=(const Data& other) const noexcept;

        [[nodiscard]] DataView view(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* makeRef(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* copyMem(size_t offset = 0, size_t size = -1) const noexcept;

        void release() noexcept;
        void clear() noexcept;
        void resize(size_t size) noexcept;

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

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        static Data copy(const glm::vec<L, T, Q>& v, bx::AllocatorI* alloc = nullptr) noexcept
        {
            return Data(glm::value_ptr(v), L*sizeof(T), alloc);
        }

        template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
        static Data copy(const glm::mat<L1, L2, T, Q>& v, bx::AllocatorI* alloc = nullptr) noexcept
        {
            return Data(glm::value_ptr(v), L1 * L2 * sizeof(T), alloc);
        }

    private:
        void* _ptr;
        size_t _size;
        bx::AllocatorI* _alloc;

        static void* malloc(size_t size, bx::AllocatorI* alloc) noexcept;

    };

	class BX_NO_VTABLE IDataLoader
	{
	public:
        // TODO: change to Data to avoid having to copy memory
        using result_type = std::shared_ptr<Data>;

		virtual ~IDataLoader() = default;
		virtual result_type operator()(std::string_view name) = 0;
        virtual std::vector<std::string> find(std::string_view name)
        {
            return std::vector<std::string>();
        }
	};
}