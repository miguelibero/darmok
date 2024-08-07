#pragma once

#include <darmok/export.h>
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>
#include <darmok/optional_ref.hpp>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <darmok/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace bx
{
    struct FileReaderI;
    struct AllocatorI;
}

namespace darmok
{
    class Data;

    class DARMOK_EXPORT DataView final
    {
    public:
        DataView(const void* ptr = nullptr, size_t size = 0) noexcept;
        DataView(const char* str) noexcept;
        DataView(const Data& data) noexcept;
        DataView(const bgfx::Memory& mem) noexcept;

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

        [[nodiscard]] const void* ptr() const noexcept;
        [[nodiscard]] const void* end() const noexcept;
        [[nodiscard]] size_t size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] bool operator==(const DataView& other) const noexcept;
        [[nodiscard]] bool operator!=(const DataView& other) const noexcept;

        [[nodiscard]] std::string_view stringView(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DataView view(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* makeRef(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* copyMem(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] std::string toHex(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] std::string toHeader(std::string_view varName, size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] std::string toString() const noexcept;

        template<typename T>
        [[nodiscard]] static DataView fromStatic(const T& data) noexcept
        {
            return DataView(&data, sizeof(T));
        }

    private:
        const void* _ptr;
        size_t _size;

        size_t fixOffset(size_t offset, void*& ptr) const noexcept;
        size_t fixSize(size_t size, size_t offset = 0) const noexcept;
    };

    class DARMOK_EXPORT Data final
    {
    public:
        Data(const OptionalRef<bx::AllocatorI>& alloc) noexcept;
        Data(size_t size = 0, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        Data(const void* ptr, size_t size, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        ~Data() noexcept;

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

        Data(const Data& other) noexcept;
        Data& operator=(const Data& other) noexcept;
        Data(const DataView& other, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        Data& operator=(const DataView& other) noexcept;
        Data(Data&& other) noexcept;
        Data& operator=(Data&& other) noexcept;
        Data(std::string_view str, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        Data& operator=(std::string_view str) noexcept;
        Data& operator=(const char* str) noexcept;
        
        [[nodiscard]] void* ptr() const noexcept;
        [[nodiscard]] void* end() const noexcept;
        [[nodiscard]] size_t size() const noexcept;
        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] bool operator==(const Data& other) const noexcept;
        [[nodiscard]] bool operator!=(const Data& other) const noexcept;

        [[nodiscard]] std::string_view stringView(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] DataView view(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* makeRef(size_t offset = 0, size_t size = -1) const noexcept;
        [[nodiscard]] const bgfx::Memory* copyMem(size_t offset = 0, size_t size = -1) const noexcept;

        void release() noexcept;
        void clear() noexcept;
        void resize(size_t size) noexcept;
        void fill(const DataView& data) noexcept;

        [[nodiscard]] std::string toString() const noexcept;
        [[nodiscard]] static Data fromHex(std::string_view hex);
        [[nodiscard]] static Data fromFile(const std::filesystem::path& path);

    private:
        void* _ptr;
        size_t _size;
        OptionalRef<bx::AllocatorI> _alloc;

        static void* malloc(size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept;
    };

	class DARMOK_EXPORT BX_NO_VTABLE IDataLoader
	{
	public:
        using result_type = Data;

        virtual ~IDataLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
	};

    class DARMOK_EXPORT FileDataLoader final : public IDataLoader
	{
	public:
        FileDataLoader(bx::FileReaderI& fileReader, const OptionalRef<bx::AllocatorI>& alloc = nullptr);
        [[nodiscard]] IDataLoader::result_type operator()(std::string_view filePath) override;
	private:
		bx::FileReaderI& _fileReader;
		OptionalRef<bx::AllocatorI> _allocator;
	};
}

DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::DataView& data);