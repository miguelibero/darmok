#include "vertex.hpp"
#include <darmok/color.hpp>
#include <darmok/program.hpp>

namespace darmok
{
    VertexDataWriter::VertexDataWriter(const bgfx::VertexLayout& layout, size_t size, bx::AllocatorI* alloc) noexcept
        : _layout(layout)
        , _size(size)
        , _alloc(alloc)
    {
    }

    void VertexDataWriter::write(bgfx::Attrib::Enum attr, uint32_t index, const std::array<float, 4>& finput, bool mark) noexcept
    {
        auto dataPtr = prepareData();
        bgfx::vertexPack(&finput.front(), false, attr, _layout, dataPtr, index);
        if (mark)
        {
            markOne(attr, index);
        }
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

    void* VertexDataWriter::prepareData() noexcept
    {
        auto size = _layout.getSize(_size);
        if (_data.size() < size)
        {
            _data = Data(size, _alloc);
        }
        return _data.ptr();
    }

    Data&& VertexDataWriter::finish() noexcept
    {
        for (auto i = 0; i < bgfx::Attrib::Count; i++)
        {
            const auto attr = static_cast<bgfx::Attrib::Enum>(i);
            if (!_layout.has(attr) || wasWritten(attr))
            {
                continue;
            }
            uint8_t num;
            bgfx::AttribType::Enum type;
            bool normalized;
            bool asInt;
            _layout.decode(attr, num, type, normalized, asInt);
            
            std::array<float, 4> input{};
            if (attr == bgfx::Attrib::Normal && num == 3 && type == bgfx::AttribType::Float)
            {
                input.at(2) = 1.F;
            }
            else if (attr >= bgfx::Attrib::Color0 && attr <= bgfx::Attrib::Color3)
            {
                std::fill(input.begin(), input.end(), Colors::maxValue);
            }
            for (auto j = 0; j < _size; j++)
            {
                write(attr, j, input);
            }
        }
        return std::move(_data);
    }

    bool VertexDataWriter::wasWritten(bgfx::Attrib::Enum attr) const noexcept
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

    bool VertexDataWriter::wasWritten(bgfx::Attrib::Enum attr, uint32_t index) const noexcept
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

    void VertexDataWriter::markOne(bgfx::Attrib::Enum attr, uint32_t index) noexcept
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

    JsonDataVertexLayoutLoader::JsonDataVertexLayoutLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    bgfx::VertexLayout JsonDataVertexLayoutLoader::operator()(std::string_view name)
    {
        auto data = _dataLoader(name);
        std::string_view json(static_cast<char*>(data->ptr()), data->size());
        bgfx::VertexLayout layout;
        Program::readVertexLayoutJson(json, layout);
        return layout;
    }
}