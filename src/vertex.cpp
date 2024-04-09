#include "vertex.hpp"
#include <darmok/color.hpp>

namespace darmok
{
    VertexDataWriter::VertexDataWriter(const bgfx::VertexLayout& layout, size_t size, bx::AllocatorI* alloc) noexcept
        : _layout(layout)
        , _size(size)
        , _alloc(alloc)
    {
    }

    bool VertexDataWriter::load(Data&& data) noexcept
    {
        auto size = _layout.getSize(_size);
        if (data.size() >= size)
        {
            _data = std::move(data);
            return true;
        }
        return false;
    }

    Data&& VertexDataWriter::finish() noexcept
    {
        auto data = const_cast<void*>(_data.ptr());
        for (auto i = 0; i < bgfx::Attrib::Count; i++)
        {
            const auto attr = static_cast<bgfx::Attrib::Enum>(i);
            if (!_layout.has(attr) || hasBeenSet(attr))
            {
                continue;
            }
            uint8_t num;
            bgfx::AttribType::Enum type;
            bool normalized;
            bool asInt;
            _layout.decode(attr, num, type, normalized, asInt);
            
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
                bgfx::vertexPack(&input.front(), normalized, attr, _layout, data, j);
            }
        }
        return std::move(_data);
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

    void* VertexDataWriter::prepareData()
    {
        auto size = _layout.getSize(_size);
        if (_data.size() < size)
        {
            _data = Data(size, _alloc);
        }
        return _data.ptr();
    }

    void VertexDataWriter::mark(bgfx::Attrib::Enum attr, uint32_t index) noexcept
    {
        auto itr = _marked.find(attr);
        if (itr == _marked.end())
        {
            _marked.emplace(attr, std::unordered_set<uint32_t>{ index });
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