#pragma once

#include <darmok/export.h>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>

#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <filesystem>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <glm/gtc/type_ptr.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

namespace bx
{
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
        [[nodiscard]] static Data fromHex(std::string_view hex, const OptionalRef<bx::AllocatorI>& alloc = nullptr);
        [[nodiscard]] static Data fromFile(const std::filesystem::path& path, const OptionalRef<bx::AllocatorI>& alloc = nullptr);

    private:
        void* _ptr;
        size_t _size;
        OptionalRef<bx::AllocatorI> _alloc;

        static void* malloc(size_t size, const OptionalRef<bx::AllocatorI>& alloc) noexcept;
    };

    template <class Archive, 
        cereal::traits::DisableIf<cereal::traits::is_output_serializable<cereal::BinaryData<uint8_t>, Archive>::value>
        = cereal::traits::sfinae
    >
    std::string save_minimal(const Archive&, const DataView& dataView)
    {
        return dataView.toHex();
    }

    template <class Archive,
        cereal::traits::DisableIf<cereal::traits::is_output_serializable<cereal::BinaryData<uint8_t>, Archive>::value>
        = cereal::traits::sfinae>
    std::string save_minimal(const Archive&, const Data& data)
    {
        return data.view().toHex();
    }

    template <class Archive,
        cereal::traits::DisableIf<cereal::traits::is_input_serializable<cereal::BinaryData<uint8_t>, Archive>::value>
        = cereal::traits::sfinae>
    void load_minimal(const Archive&, Data& data, const std::string& hex)
    {
        data = std::move(Data::fromHex(hex));
    }

    template <class Archive,
        cereal::traits::EnableIf<cereal::traits::is_output_serializable<cereal::BinaryData<uint8_t>, Archive>::value>
        = cereal::traits::sfinae>
    void save(Archive& archive, const DataView& dataView)
    {
        auto size = dataView.size();
        archive(cereal::make_size_tag(size));
        archive(cereal::binary_data(static_cast<const uint8_t*>(dataView.ptr()), size));
    }

    template <class Archive,
        cereal::traits::EnableIf<cereal::traits::is_output_serializable<cereal::BinaryData<uint8_t>, Archive>::value>
        = cereal::traits::sfinae>
    void save(Archive& archive, const Data& data)
    {
        auto size = data.size();
        archive(cereal::make_size_tag(size));
        archive(cereal::binary_data(static_cast<const uint8_t*>(data.ptr()), size));
    }

    template <class Archive,
        cereal::traits::EnableIf<cereal::traits::is_input_serializable<cereal::BinaryData<uint8_t>, Archive>::value>
        = cereal::traits::sfinae>
    void load(Archive& archive, Data& data)
    {
        auto size = data.size();
        archive(cereal::make_size_tag(size));
        data.resize(size);
        archive(cereal::binary_data(static_cast<uint8_t*>(data.ptr()), size));
    }

    class DARMOK_EXPORT BX_NO_VTABLE IDataLoader
    {
    public:
        using Resource = Data;
        virtual ~IDataLoader() = default;
        [[nodiscard]] virtual Data operator()(const std::filesystem::path& path) = 0;
    };

    class DARMOK_EXPORT DataLoader final : public IDataLoader
	{
	public:
        using Resource = Data;
        DataLoader(const OptionalRef<bx::AllocatorI>& alloc = nullptr);

        DataLoader& addBasePath(const std::filesystem::path& basePath) noexcept;
        bool removeBasePath(const std::filesystem::path& basePath) noexcept;

        [[nodiscard]] Data operator()(const std::filesystem::path& path) noexcept override;
	private:
        std::vector<std::filesystem::path> _basePaths;
		OptionalRef<bx::AllocatorI> _alloc;
	};
}

DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::DataView& data);
DARMOK_EXPORT std::istream& operator>>(std::istream& in, darmok::Data& data);