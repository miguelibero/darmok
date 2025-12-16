#pragma once

#include <darmok/export.h>
#include <darmok/vertex_fwd.hpp>
#include <darmok/data.hpp>
#include <darmok/collection.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/glm.hpp>

#include <string_view>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>

#include <bgfx/bgfx.h>
#include <glm/gtc/type_ptr.hpp>

namespace bx
{
    struct AllocatorI;
}

namespace darmok
{

    template<typename T>
    struct DARMOK_EXPORT BaseBgfxHandle
    {
    public:
        BaseBgfxHandle() noexcept
            : _bgfx{ bgfx::kInvalidHandle }
        {
        }

        BaseBgfxHandle(T handle) noexcept
            : _bgfx{ handle }
        {
        }

        ~BaseBgfxHandle() noexcept
        {
            reset();
        }

        BaseBgfxHandle(const BaseBgfxHandle& other) = delete;
        BaseBgfxHandle& operator=(const BaseBgfxHandle& other) = delete;
        
        BaseBgfxHandle(BaseBgfxHandle&& other) noexcept
            : _bgfx{ other._bgfx }
        {
            other._bgfx.idx = bgfx::kInvalidHandle;
        }

        BaseBgfxHandle& operator=(BaseBgfxHandle&& other) noexcept
        {
            reset();
            _bgfx = other._bgfx;
            other._bgfx.idx = bgfx::kInvalidHandle;
            return *this;
        }

        operator T() const noexcept
        {
            return get();
        }

        const T& get() const noexcept
        {
            return _bgfx;
        }

        bool reset() noexcept
        {
            if (isValid(_bgfx))
            {
                bgfx::destroy(_bgfx);
                _bgfx.idx = bgfx::kInvalidHandle;
                return true;
            }
            return false;
        }

        operator bool() const noexcept
        {
            return valid();
        }

        bool valid() const noexcept
        {
            return isValid(_bgfx);
        }

        uint16_t idx() const noexcept
        {
            return _bgfx.idx;
        }

    private:
        T _bgfx;
    };

    struct DARMOK_EXPORT VertexBufferHandle final : BaseBgfxHandle<bgfx::VertexBufferHandle>
    {
    public:
        VertexBufferHandle() = default;
        VertexBufferHandle(const bgfx::Memory* mem, const bgfx::VertexLayout& layout, uint16_t flags = BGFX_BUFFER_NONE) noexcept;
    };

    struct DARMOK_EXPORT IndexBufferHandle final : BaseBgfxHandle<bgfx::IndexBufferHandle>
    {
    public:
        IndexBufferHandle() = default;
        IndexBufferHandle(const bgfx::Memory* mem, uint16_t flags = BGFX_BUFFER_NONE) noexcept;
    };

    struct DARMOK_EXPORT DynamicVertexBufferHandle final : BaseBgfxHandle<bgfx::DynamicVertexBufferHandle>
    {
    public:
        DynamicVertexBufferHandle() = default;
        DynamicVertexBufferHandle(uint32_t num, const bgfx::VertexLayout& layout, uint16_t flags = BGFX_BUFFER_NONE) noexcept;
        DynamicVertexBufferHandle(const bgfx::Memory* mem, const bgfx::VertexLayout& layout, uint16_t flags = BGFX_BUFFER_NONE) noexcept;
    };

    struct DARMOK_EXPORT DynamicIndexBufferHandle final : BaseBgfxHandle<bgfx::DynamicIndexBufferHandle>
    {
    public:
        DynamicIndexBufferHandle() = default;
        DynamicIndexBufferHandle(uint32_t num, uint16_t flags = BGFX_BUFFER_NONE) noexcept;
        DynamicIndexBufferHandle(const bgfx::Memory* mem, uint16_t flags = BGFX_BUFFER_NONE) noexcept;
    };

    struct DARMOK_EXPORT TransientVertexBuffer final
    {
        TransientVertexBuffer() noexcept;
        static expected<TransientVertexBuffer, std::string> create(uint32_t vertNum, const bgfx::VertexLayout& layout) noexcept;
        static expected<TransientVertexBuffer, std::string> create(DataView data, const bgfx::VertexLayout& layout) noexcept;
        const bgfx::TransientVertexBuffer& get() const noexcept;
        uint16_t idx() const noexcept;
        DataView data() noexcept;
        const DataView data() const noexcept;
    private:
        TransientVertexBuffer(bgfx::TransientVertexBuffer bgfx) noexcept;
        bgfx::TransientVertexBuffer _bgfx;
    };

    struct DARMOK_EXPORT TransientIndexBuffer final
    {
        TransientIndexBuffer() noexcept;
        static expected<TransientIndexBuffer, std::string> create(uint32_t idxNum, bool index32) noexcept;
        static expected<TransientIndexBuffer, std::string> create(DataView data, bool index32) noexcept;
        const bgfx::TransientIndexBuffer& get() const noexcept;
        uint16_t idx() const noexcept;
        DataView data() noexcept;
        const DataView data() const noexcept;
    private:
        TransientIndexBuffer(bgfx::TransientIndexBuffer bgfx) noexcept;
        bgfx::TransientIndexBuffer _bgfx;
    };

    class DARMOK_EXPORT VertexDataWriter final
    {
    public:
        template<typename T>
        struct Input final
        {
            const T* ptr;
            size_t size;
        };

    private:

        template<typename T>
        static void convertInput(bgfx::Attrib::Enum attr, const Input<T>& input, std::array<float, 4>& finput) noexcept
        {
            for (uint8_t i = 0; i < finput.size(); i++)
            {
                finput.at(i) = i < input.size ? static_cast<float>(input.ptr[i]) : 0.F;
            }
        }
        
    public:
        VertexDataWriter(const bgfx::VertexLayout& layout, uint32_t size, OptionalRef<bx::AllocatorI> alloc = nullptr) noexcept;
        void load(Data&& data) noexcept;
        const bgfx::VertexLayout& getLayout() const noexcept;
        uint32_t getSize() const noexcept;

        template<typename Iter, typename Filter>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, Iter begin, Iter end, Filter filter) noexcept
        {
            if (_layout.has(attr))
            {
                for (uint32_t i = 0; i < _size; i++)
                {
                    write(attr, i, filter(begin));
                    if (++begin == end)
                    {
                        break;
                    }
                }
            }
            return *this;
        }

        template<typename T>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, uint32_t index, const Input<T>& input) noexcept
        {
            std::array<float, 4> finput{};
            convertInput(attr, input, finput);
            write(attr, index, finput);
            return *this;
        }

        template<typename T>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const Input<T>& input, bool overwrite = true) noexcept
        {
            std::array<float, 4> finput{};
            convertInput(attr, input, finput);
            for (uint32_t i = 0; i < _size; i++)
            {
                if (overwrite || !wasWritten(attr, i))
                {
                    write(attr, i, finput, false);
                }
            }
            markAll(attr);
            return *this;
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        static Input<T> getVecInput(const glm::vec<L, T, Q>& input) noexcept
        {
            return { glm::value_ptr(input), sizeof(T) * L };
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, uint32_t index, const glm::vec<L, T, Q>& input) noexcept
        {
            return write(attr, index, getVecInput(input));
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const glm::vec<L, T, Q>& input, bool overwrite = true) noexcept
        {
            return write(attr, getVecInput(input), overwrite);
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const RefCollection<glm::vec<L, T, Q>>& input) noexcept
        {
            return write(attr, input.begin(), input.end(), [](auto& itr) {
                return getVecInput(*itr);
            });
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const ConstRefCollection<glm::vec<L, T, Q>>& input) noexcept
        {
            return write(attr, input.begin(), input.end(), [](auto& itr) {
                return getVecInput(*itr);
            });
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const ValCollection<glm::vec<L, T, Q>>& input) noexcept
        {
            if (_layout.has(attr))
            {
                for (auto i = 0; i < _size && i < input.size(); i++)
                {
                    auto elm = input[i];
                    write(attr, i, getVecInput(elm));
                }
            }
            return *this;
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const std::vector<glm::vec<L, T, Q>>& input) noexcept
        {
            return write(attr, input.begin(), input.end(), [](auto& itr) {
                return getVecInput(*itr);
            });
        }

        [[nodiscard]] bool wasWritten(bgfx::Attrib::Enum attr, uint32_t index) const noexcept;
        [[nodiscard]] bool wasWritten(bgfx::Attrib::Enum attr) const noexcept;

        Data&& finish() noexcept;

    private:
        const bgfx::VertexLayout& _layout;
        uint32_t _size;
        Data _data;
        std::unordered_set<bgfx::Attrib::Enum> _markedAll;
        std::unordered_map<bgfx::Attrib::Enum, std::unordered_set<uint32_t>> _marked;

        void* prepareData() noexcept;
        bool write(bgfx::Attrib::Enum attr, uint32_t index, const std::array<float, 4>& finput, bool mark = true) noexcept;

        void markOne(bgfx::Attrib::Enum attr, uint32_t index) noexcept;
        void markAll(bgfx::Attrib::Enum attr) noexcept;
    };
}