#include "vertex.hpp"
#include <darmok/color.hpp>

namespace darmok
{
    XmlDataVertexLayoutLoader::XmlDataVertexLayoutLoader(IDataLoader& dataLoader)
        : _dataLoader(dataLoader)
        {
        }

	bgfx::VertexLayout XmlDataVertexLayoutLoader::operator()(std::string_view name)
    {
        bgfx::VertexLayout layout;
        return layout;
    }

    VertexDataWriter::VertexDataWriter(const bgfx::VertexLayout& layout, size_t size, bx::AllocatorI* alloc) noexcept
        : _layout(layout)
        , _size(size)
        , _data(layout.getSize(size), alloc)
        , _alloc(alloc)
    {
        for (int i = 0; i < bgfx::Attrib::Count; i++)
        {
            _marked[i] = 0;
        }
    }

    Data&& VertexDataWriter::release() noexcept
    {
        for (auto i = 0; i < bgfx::Attrib::Count; i++)
        {
            auto attr = (bgfx::Attrib::Enum)i;
            if (!_layout.has(attr) || hasBeenSet(attr))
            {
                continue;
            }
            uint8_t num;
            bgfx::AttribType::Enum type;
            bool normalize;
            bool asInt;
            _layout.decode(attr, num, type, normalize, asInt);
            
            float input[] = { 0.f, 0.f, 0.f, 0.f };
            if (attr == bgfx::Attrib::Normal && num == 3 && type == bgfx::AttribType::Float)
            {
                input[2] = 1.f;
            }
            else if (attr >= bgfx::Attrib::Color0 && attr <= bgfx::Attrib::Color3)
            {
                input[0] = input[1] = input[2] = input[3] = 1.f;
            }
            for (auto j = 0; j < _size; j++)
            {
                bgfx::vertexPack(input, true, attr, _layout, _data.ptr(), j);
            }
        }
        return std::move(_data);
    }

    VertexDataWriter& VertexDataWriter::set(bgfx::Attrib::Enum attr, const BaseReadOnlyCollection<glm::vec2>& input) noexcept
    {
        return setIter(attr, input.begin(), input.end(), [](auto& itr) { return glm::value_ptr(*itr); });
    }
    VertexDataWriter& VertexDataWriter::set(bgfx::Attrib::Enum attr, const BaseReadOnlyCollection<glm::vec3>& input) noexcept
    {
        return setIter(attr, input.begin(), input.end(), [](auto& itr) { return glm::value_ptr(*itr); });
    }

    VertexDataWriter& VertexDataWriter::set(bgfx::Attrib::Enum attr, const BaseReadOnlyCollection<Color>& input) noexcept
    {
        return setIter(attr, input.begin(), input.end(), [](auto& itr) { return itr->ptr(); });
    }

    VertexDataWriter& VertexDataWriter::set(bgfx::Attrib::Enum attr, const std::vector<glm::vec2>& input) noexcept
    {
        return setIter(attr, input.begin(), input.end(), [](auto& itr) { return glm::value_ptr(*itr); });
    }

    VertexDataWriter& VertexDataWriter::set(bgfx::Attrib::Enum attr, const std::vector<glm::vec3>& input) noexcept
    {
        return setIter(attr, input.begin(), input.end(), [](auto& itr) { return glm::value_ptr(*itr); });
    }

    VertexDataWriter& VertexDataWriter::set(bgfx::Attrib::Enum attr, const std::vector<Color>& input) noexcept
    {
        return setIter(attr, input.begin(), input.end(), [](auto& itr) { return itr->ptr(); });
    }

    bool VertexDataWriter::hasBeenSet(bgfx::Attrib::Enum attr) const noexcept
    {
        uint32_t v = (1 << _size) - 1;
        return (_marked[attr] & v) == v;
    }

    bool VertexDataWriter::hasBeenSet(bgfx::Attrib::Enum attr, uint32_t index) const noexcept
    {
        return _marked[attr] & 1 >> index;
    }

    void VertexDataWriter::mark(bgfx::Attrib::Enum attr, uint32_t index) noexcept
    {
        _marked[attr] |= 1 << index;
    }

    void VertexDataWriter::markAll(bgfx::Attrib::Enum attr) noexcept
    {
        _marked[attr] = (1 << _size) - 1;
    }
}