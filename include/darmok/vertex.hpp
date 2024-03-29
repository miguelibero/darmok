#pragma once

#include <darmok/data.hpp>
#include <darmok/utils.hpp>
#include <string_view>
#include <vector>
#include <array>
#include <unordered_set>
#include <unordered_map>

#include <bgfx/bgfx.h>
#include <bx/bx.h>
#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
	class BX_NO_VTABLE IVertexLayoutLoader
	{
	public:
		virtual ~IVertexLayoutLoader() = default;
		virtual bgfx::VertexLayout operator()(std::string_view name) = 0;
	};

    using VertexIndex = uint16_t;
    using Color = glm::u8vec4;

    class VertexDataWriter final
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
        bool convertInput(bgfx::Attrib::Enum attr, const Input<T>& input, std::array<float, 4>& finput) noexcept
        {
            uint8_t num;
            bgfx::AttribType::Enum type;
            bool normalize;
            bool asInt;
            _layout.decode(attr, num, type, normalize, asInt);
            if (num == 0 || type == bgfx::AttribType::Count)
            {
                return false;
            }
            for(uint8_t i = 0; i < finput.size() && i < num; i++)
            {
                finput.at(i) = i < input.size ? static_cast<float>(input.ptr[i]) : 0.F;
            }
            return true;
        }


    public:
        VertexDataWriter(const bgfx::VertexLayout& layout, size_t size, bx::AllocatorI* alloc = nullptr) noexcept;
        VertexDataWriter(const bgfx::VertexLayout& layout, const DataView& data) noexcept;

        Data&& finish() noexcept;

        template<typename Iter, typename Filter>
        VertexDataWriter& setIter(bgfx::Attrib::Enum attr, Iter begin, Iter end, Filter filter) noexcept
        {
            if (_layout.has(attr))
            {
                auto data = _dataView.ptr();
                for (auto i = 0; i < _size; i++)
                {
                    set(attr, i, filter(begin));
                    if (++begin == end)
                    {
                        break;
                    }
                }
            }
            return *this;
        }

        template<typename T>
        VertexDataWriter& set(bgfx::Attrib::Enum attr, uint32_t index, const Input<T>& input) noexcept
        {
            std::array<float, 4> finput{};
            if (convertInput(attr, input, finput))
            {
                auto data = _dataView.ptr();
                bgfx::vertexPack(&finput.front(), false, attr, _layout, data, index);
                mark(attr, index);
            }

            return *this;
        }

        template<typename T>
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const Input<T>& input, bool overwrite = true) noexcept
        {
            std::array<float, 4> finput{};
            if (convertInput(attr, input, finput))
            {
                auto data = _dataView.ptr();
                for (auto i = 0; i < _size; i++)
                {
                    if (overwrite || !hasBeenSet(attr, i))
                    {
                        bgfx::vertexPack(&finput.front(), false, attr, _layout, data, i);
                    }
                }
                markAll(attr);
            }
            return *this;
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& set(bgfx::Attrib::Enum attr, uint32_t index, const glm::vec<L, T, Q>& input) noexcept
        {
            return set(attr, index, Input{ glm::value_ptr(input), L });
        }

        template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const glm::vec<L, T, Q>& input, bool overwrite = true) noexcept
        {
            return set(attr, Input{ glm::value_ptr(input), L }, overwrite);
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const BaseReadOnlyCollection<glm::vec<L, T, Q>>& input) noexcept
        {
            return setIter(attr, input.begin(), input.end(), [](auto& itr) {
                return Input{ glm::value_ptr(*itr), L };
            });
        }

        template<int L, typename T, glm::qualifier Q = glm::defaultp>
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const std::vector<glm::vec<L, T, Q>>& input) noexcept
        {
            return setIter(attr, input.begin(), input.end(), [](auto& itr) {
                return Input{ glm::value_ptr(*itr), L };
            });
        }

        [[nodiscard]] bool hasBeenSet(bgfx::Attrib::Enum attr, uint32_t index) const noexcept;
        [[nodiscard]] bool hasBeenSet(bgfx::Attrib::Enum attr) const noexcept;

    private:
        const bgfx::VertexLayout& _layout;
        size_t _size;
        Data _data;
        DataView _dataView;
        std::unordered_set<bgfx::Attrib::Enum> _markedAll;
        std::unordered_map<bgfx::Attrib::Enum, std::unordered_set<uint32_t>> _marked;

        void mark(bgfx::Attrib::Enum attr, uint32_t index) noexcept;
        void markAll(bgfx::Attrib::Enum attr) noexcept;
    };
}