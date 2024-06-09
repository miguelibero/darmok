#pragma once

#include <vector>
#include <memory>
#include <string>
#include <darmok/optional_ref.hpp>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <glm/gtc/type_ptr.hpp>

namespace bx
{
    struct FileReaderI;
    struct AllocatorI;
}

namespace darmok
{
    class Data;

    class DataView final
    {
    public:
        DLLEXPORT DataView(const void* ptr = nullptr, size_t size = 0) noexcept;
        DLLEXPORT DataView(const char* str) noexcept;
        DLLEXPORT DataView(const Data& data) noexcept;
        DLLEXPORT DataView(const bgfx::Memory& mem) noexcept;

        template<typename T>
        DataView(const std::vector<T>& v) noexcept
            : DataView(v.empty() ? nullptr : &v.front(), sizeof(T)* v.size())
        {
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        DataView(const glm::vec<L, T, Q>& v) noexcept
            : DataView(glm::value_ptr(v), L * sizeof(T))
        {
        }

        template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
        DataView(const glm::mat<L1, L2, T, Q>& v) noexcept
            : DataView(glm::value_ptr(v), L1* L2 * sizeof(T))
        {
        }

        [[nodiscard]] DLLEXPORT const void* ptr() const noexcept;
        [[nodiscard]] DLLEXPORT const void* end() const noexcept;
        [[nodiscard]] DLLEXPORT size_t size() const noexcept;
        [[nodiscard]] DLLEXPORT bool empty() const noexcept;
        [[nodiscard]] DLLEXPORT bool operator==(const DataView& other) const noexcept;
        [[nodiscard]] DLLEXPORT bool operator!=(const DataView& other) const noexcept;

        [[nodiscard]] DLLEXPORT std::string_view stringView(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT DataView view(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT const bgfx::Memory* makeRef(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT const bgfx::Memory* copyMem(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT std::string toHex(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT std::string toHeader(std::string_view varName, size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT std::string to_string() const noexcept;

        template<typename T>
        [[nodiscard]] DLLEXPORT static DataView fromArray(const T& arr) noexcept
        {
            return DataView(&arr, sizeof(T));
        }

    private:
        const void* _ptr;
        size_t _size;

        size_t fixOffset(size_t offset, void*& ptr) const noexcept;
        size_t fixSize(size_t size, size_t offset = 0) const noexcept;
    };

    class Data final
    {
    public:
        DLLEXPORT Data(const OptionalRef<bx::AllocatorI>& alloc) noexcept;
        DLLEXPORT Data(size_t size = 0, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        DLLEXPORT Data(const void* ptr, size_t size, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        DLLEXPORT ~Data() noexcept;

        template<typename T>
        Data(const std::vector<T>& v, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept
            : Data(v.empty() ? nullptr : &v.front(), sizeof(T)* v.size(), alloc)
        {
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        Data(const glm::vec<L, T, Q>& v, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept
            : Data(glm::value_ptr(v), L * sizeof(T), alloc)
        {
        }

        template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
        Data(const glm::mat<L1, L2, T, Q>& v, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept
            : Data(glm::value_ptr(v), L1* L2 * sizeof(T), alloc)
        {
        }

        DLLEXPORT Data(const Data& other) noexcept;
        DLLEXPORT Data& operator=(const Data& other) noexcept;
        DLLEXPORT Data(const DataView& other, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        DLLEXPORT Data& operator=(const DataView& other) noexcept;
        DLLEXPORT Data(Data&& other) noexcept;
        DLLEXPORT Data& operator=(Data&& other) noexcept;
        DLLEXPORT Data(std::string_view str, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        DLLEXPORT Data& operator=(std::string_view str) noexcept;
        DLLEXPORT Data& operator=(const char* str) noexcept;
        DLLEXPORT operator DataView() const;
        
        [[nodiscard]] DLLEXPORT void* ptr() const noexcept;
        [[nodiscard]] DLLEXPORT void* end() const noexcept;
        [[nodiscard]] DLLEXPORT size_t size() const noexcept;
        [[nodiscard]] DLLEXPORT bool empty() const noexcept;

        [[nodiscard]] DLLEXPORT bool operator==(const Data& other) const noexcept;
        [[nodiscard]] DLLEXPORT bool operator!=(const Data& other) const noexcept;

        [[nodiscard]] DLLEXPORT std::string_view stringView(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT DataView view(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT const bgfx::Memory* makeRef(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DLLEXPORT const bgfx::Memory* copyMem(size_t offset = 0, size_t size = -1) const noexcept;

        DLLEXPORT void release() noexcept;
        DLLEXPORT void clear() noexcept;
        DLLEXPORT void resize(size_t size) noexcept;
        DLLEXPORT void fill(const DataView& data) noexcept;

        [[nodiscard]] DLLEXPORT std::string to_string() const noexcept;
        [[nodiscard]] DLLEXPORT static Data fromHex(std::string_view hex);
        [[nodiscard]] DLLEXPORT static Data fromFile(const std::string& path);

    private:
        void* _ptr;
        size_t _size;
        OptionalRef<bx::AllocatorI> _alloc;

        static void* malloc(size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept;
    };

	class BX_NO_VTABLE IDataLoader
	{
	public:
        // TODO: change to Data to avoid having to copy memory
        using result_type = Data;

        DLLEXPORT virtual ~IDataLoader() = default;
        DLLEXPORT virtual result_type operator()(std::string_view name) = 0;
	};

    class FileDataLoader final : public IDataLoader
	{
	public:
		FileDataLoader(bx::FileReaderI* fileReader, const OptionalRef<bx::AllocatorI>& alloc = nullptr);
		IDataLoader::result_type operator()(std::string_view filePath) override;
	private:
		bx::FileReaderI* _fileReader;
		OptionalRef<bx::AllocatorI> _allocator;
	};
}