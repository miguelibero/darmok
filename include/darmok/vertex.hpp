#pragma once

#include <darmok/data.hpp>
#include <darmok/utils.hpp>
#include <string_view>

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

    typedef uint16_t VertexIndex;

    struct Color;

    class VertexDataWriter final
    {
    private:
        template<typename T>
        bool convertInput(bgfx::Attrib::Enum attr, const T* input, float finput[4])
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
            for (auto i = 0; i < num && i < 4; i++)
            {
                finput[i] = static_cast<const float>(*input++);
            }
            return true;
        }


    public:
        VertexDataWriter(const bgfx::VertexLayout& layout, size_t size, bx::AllocatorI* alloc = nullptr) noexcept;
        Data&& release() noexcept;

        template<typename Iter, typename Filter>
        VertexDataWriter& setIter(bgfx::Attrib::Enum attr, Iter begin, Iter end, Filter filter) noexcept
        {
            if (_layout.has(attr))
            {
                auto data = _data.ptr();
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
        VertexDataWriter& set(bgfx::Attrib::Enum attr, uint32_t index, const T* input) noexcept
        {
            float finput[4];
            if (convertInput(attr, input, finput))
            {
                auto data = _data.ptr();
                bgfx::vertexPack(finput, false, attr, _layout, data, index);
                mark(attr, index);
            }

            return *this;
        }

        template<typename T>
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const T* input, bool overwrite = true) noexcept
        {
            float finput[4];
            if (convertInput(attr, input, finput))
            {
                auto data = _data.ptr();
                for (auto i = 0; i < _size; i++)
                {
                    if (overwrite || !hasBeenSet(attr, i))
                    {
                        bgfx::vertexPack(finput, false, attr, _layout, data, i);
                    }
                }
                markAll(attr);
            }
            return *this;
        }

        VertexDataWriter& set(bgfx::Attrib::Enum attr, const BaseReadOnlyCollection<glm::vec2>& input) noexcept;
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const BaseReadOnlyCollection<glm::vec3>& input) noexcept;
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const BaseReadOnlyCollection<Color>& input) noexcept;
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const std::vector<Color>& input) noexcept;
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const std::vector<glm::vec2>& input) noexcept;
        VertexDataWriter& set(bgfx::Attrib::Enum attr, const std::vector<glm::vec3>& input) noexcept;

        bool hasBeenSet(bgfx::Attrib::Enum attr, uint32_t index) const noexcept;
        bool hasBeenSet(bgfx::Attrib::Enum attr) const noexcept;

    private:
        uint32_t _marked[bgfx::Attrib::Count];
        Data _data;
        const bgfx::VertexLayout& _layout;
        size_t _size;
        bx::AllocatorI* _alloc;

        void mark(bgfx::Attrib::Enum attr, uint32_t index) noexcept;
        void markAll(bgfx::Attrib::Enum attr) noexcept;
    };
}