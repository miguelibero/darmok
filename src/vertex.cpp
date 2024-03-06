#include "vertex.hpp"
#include <darmok/color.hpp>

namespace darmok
{
    XmlDataVertexLayoutLoader::XmlDataVertexLayoutLoader(IDataLoader& dataLoader) noexcept
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
    }

    Data&& VertexDataWriter::release() noexcept
    {
        for (auto i = 0; i < bgfx::Attrib::Count; i++)
        {
            const auto attr = static_cast<bgfx::Attrib::Enum>(i);
            if (!_layout.has(attr) || hasBeenSet(attr))
            {
                continue;
            }
            uint8_t num;
            bgfx::AttribType::Enum type;
            bool normalize;
            bool asInt;
            _layout.decode(attr, num, type, normalize, asInt);
            
            std::array<float, 4> input;
            if (attr == bgfx::Attrib::Normal && num == 3 && type == bgfx::AttribType::Float)
            {
                input.at(2) = 1.F;
            }
            else if (attr >= bgfx::Attrib::Color0 && attr <= bgfx::Attrib::Color3)
            {
                std::fill(input.begin(), input.end(), 1.F);
            }
            for (auto j = 0; j < _size; j++)
            {
                bgfx::vertexPack(&input.front(), true, attr, _layout, _data.ptr(), j);
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
        return setIter(attr, input.begin(), input.end(), [](auto& itr) noexcept { return itr->ptr(); });
    }

    bool VertexDataWriter::hasBeenSet(bgfx::Attrib::Enum attr) const noexcept
    {
        if (_markedAll.find(attr) != _markedAll.end())
        {
            return true;
        }
        auto itr = _marked.find(attr);
        if (itr == _marked.end())
        {
            return false;
        }
        for (uint32_t index = 0; index < _size; index++)
        {
            if (itr->second.find(index) == itr->second.end())
            {
                return false;
            }
        }
        return true;
    }

    bool VertexDataWriter::hasBeenSet(bgfx::Attrib::Enum attr, uint32_t index) const noexcept
    {
        if (_markedAll.find(attr) != _markedAll.end())
        {
            return true;
        }
        auto itr = _marked.find(attr);
        if (itr == _marked.end())
        {
            return false;
        }
        return itr->second.find(index) != itr->second.end();
    }

    void VertexDataWriter::mark(bgfx::Attrib::Enum attr, uint32_t index) noexcept
    {
        auto itr = _marked.find(attr);
        if (itr == _marked.end())
        {
            _marked.emplace(std::make_pair(attr, std::set<uint32_t>{ index }));
        }
        else
        {
            itr->second.emplace(index);
        }
    }

    void VertexDataWriter::markAll(bgfx::Attrib::Enum attr) noexcept
    {
        _markedAll.emplace(attr);
    }
}