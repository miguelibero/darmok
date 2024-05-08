#pragma once

#include <darmok/data.hpp>
#include <darmok/collection.hpp>
#include <darmok/optional_ref.hpp>
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
    class BX_NO_VTABLE IVertexLayoutLoader
    {
    public:
        virtual ~IVertexLayoutLoader() = default;
        virtual bgfx::VertexLayout operator()(std::string_view name) = 0;
    };

    class VertexDataWriter final
    {
    public:
        template<typename T>
        struct Input final
        {
            T* ptr;
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
        VertexDataWriter(const bgfx::VertexLayout& layout, size_t size, const OptionalRef<bx::AllocatorI>& alloc = nullptr) noexcept;
        void load(Data&& data) noexcept;

        template<typename Iter, typename Filter>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, Iter begin, Iter end, Filter filter) noexcept
        {
            if (_layout.has(attr))
            {
                for (auto i = 0; i < _size; i++)
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
            std::array<float, 4> finput;
            convertInput(attr, input, finput);
            write(attr, index, finput);
            return *this;
        }

        template<typename T>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const Input<T>& input, bool overwrite = true) noexcept
        {
            std::array<float, 4> finput;
            convertInput(attr, input, finput);
            for (auto i = 0; i < _size; i++)
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
        VertexDataWriter& write(bgfx::Attrib::Enum attr, uint32_t index, const glm::vec<L, T, Q>& input) noexcept
        {
            return write(attr, index, Input{ glm::value_ptr(input), L });
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const glm::vec<L, T, Q>& input, bool overwrite = true) noexcept
        {
            return write(attr, Input{ glm::value_ptr(input), L }, overwrite);
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const RefCollection<glm::vec<L, T, Q>>& input) noexcept
        {
            return write(attr, input.begin(), input.end(), [](auto& itr) {
                return Input{ glm::value_ptr(*itr), L };
            });
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const ConstRefCollection<glm::vec<L, T, Q>>& input) noexcept
        {
            return write(attr, input.begin(), input.end(), [](auto& itr) {
                return Input{ glm::value_ptr(*itr), L };
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
                    write(attr, i, Input{ glm::value_ptr(elm), L });
                }
            }
            return *this;
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& write(bgfx::Attrib::Enum attr, const std::vector<glm::vec<L, T, Q>>& input) noexcept
        {
            return write(attr, input.begin(), input.end(), [](auto& itr) {
                return Input{ glm::value_ptr(*itr), L };
            });
        }

        [[nodiscard]] bool wasWritten(bgfx::Attrib::Enum attr, uint32_t index) const noexcept;
        [[nodiscard]] bool wasWritten(bgfx::Attrib::Enum attr) const noexcept;

        Data&& finish() noexcept;

    private:
        const bgfx::VertexLayout& _layout;
        size_t _size;
        Data _data;
        std::unordered_set<bgfx::Attrib::Enum> _markedAll;
        std::unordered_map<bgfx::Attrib::Enum, std::unordered_set<uint32_t>> _marked;

        void* prepareData() noexcept;
        bool write(bgfx::Attrib::Enum attr, uint32_t index, const std::array<float, 4>& finput, bool mark = true) noexcept;

        void markOne(bgfx::Attrib::Enum attr, uint32_t index) noexcept;
        void markAll(bgfx::Attrib::Enum attr) noexcept;
    };
}