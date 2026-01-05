#include <darmok/vertex.hpp>
#include <darmok/color.hpp>
#include <darmok/utils.hpp>

namespace darmok
{
    VertexBuffer::VertexBuffer(const bgfx::Memory* mem, const bgfx::VertexLayout& layout, uint16_t flags) noexcept
        : BaseBgfxOwnedHandle(bgfx::createVertexBuffer(mem, layout, flags))
    {
    }

    IndexBuffer::IndexBuffer(const bgfx::Memory* mem, uint16_t flags) noexcept
        : BaseBgfxOwnedHandle(bgfx::createIndexBuffer(mem, flags))
    {
    }

    DynamicVertexBuffer::DynamicVertexBuffer(uint32_t num, const bgfx::VertexLayout& layout, uint16_t flags) noexcept
        : BaseBgfxOwnedHandle(bgfx::createDynamicVertexBuffer(num, layout, flags))
    {
    }

    DynamicVertexBuffer::DynamicVertexBuffer(const bgfx::Memory* mem, const bgfx::VertexLayout& layout, uint16_t flags) noexcept
        : BaseBgfxOwnedHandle(bgfx::createDynamicVertexBuffer(mem, layout, flags))
    {
    }

    DynamicIndexBuffer::DynamicIndexBuffer(uint32_t num, uint16_t flags) noexcept
        : BaseBgfxOwnedHandle(bgfx::createDynamicIndexBuffer(num, flags))
    {
    }

    DynamicIndexBuffer::DynamicIndexBuffer(const bgfx::Memory* mem, uint16_t flags) noexcept
        : BaseBgfxOwnedHandle(bgfx::createDynamicIndexBuffer(mem, flags))
    {
    }

    TransientVertexBuffer::TransientVertexBuffer() noexcept
    {
        _bgfx.data = nullptr;
        _bgfx.size = 0;
        _bgfx.handle.idx = bgfx::kInvalidHandle;
    }

    TransientVertexBuffer::TransientVertexBuffer(bgfx::TransientVertexBuffer bgfx) noexcept
        : _bgfx{ std::move(bgfx) }
    {
    }

    const bgfx::TransientVertexBuffer& TransientVertexBuffer::get() const noexcept
    {
        return _bgfx;
    }

    uint16_t TransientVertexBuffer::idx() const noexcept
    {
        return _bgfx.handle.idx;
    }

    DataView TransientVertexBuffer::data() noexcept
    {
        return { _bgfx.data, _bgfx.size };
    }

    const DataView TransientVertexBuffer::data() const noexcept
    {
        return { _bgfx.data, _bgfx.size };
    }

    expected<TransientVertexBuffer, std::string> TransientVertexBuffer::create(uint32_t vertNum, const bgfx::VertexLayout& layout) noexcept
    {
        if (vertNum == 0)
        {
            return unexpected<std::string>{ "empty" };
        }
        if (!bgfx::getAvailTransientVertexBuffer(vertNum, layout))
        {
            return unexpected<std::string>{ "not enought transient vertex buffer space" };
        }
        bgfx::TransientVertexBuffer buffer;
        bgfx::allocTransientVertexBuffer(&buffer, vertNum, layout);
        return TransientVertexBuffer{ buffer };
    }

    expected<TransientVertexBuffer, std::string> TransientVertexBuffer::create(DataView data, const bgfx::VertexLayout& layout) noexcept
    {
        auto stride = layout.getStride();
        if (stride == 0)
        {
            return unexpected<std::string>{ "empty vertex layout" };
        }
        auto vertNum = static_cast<uint32_t>(data.size()) / stride;
        auto result = create(vertNum, layout);
        if (result)
        {
            result.value().data().fill(data);
        }
        return result;
    }

    TransientIndexBuffer::TransientIndexBuffer(bgfx::TransientIndexBuffer bgfx) noexcept
        : _bgfx{ std::move(bgfx) }
    {
    }

    TransientIndexBuffer::TransientIndexBuffer() noexcept
    {
        _bgfx.data = nullptr;
        _bgfx.size = 0;
        _bgfx.handle.idx = bgfx::kInvalidHandle;
    }

    expected<TransientIndexBuffer, std::string> TransientIndexBuffer::create(uint32_t idxNum, bool index32) noexcept
    {
        if (idxNum == 0)
        {
            return unexpected<std::string>{ "empty" };
        }
        if (!bgfx::getAvailTransientIndexBuffer(idxNum, index32))
        {
            return unexpected<std::string>{ "not enought transient index buffer space" };
        }
        bgfx::TransientIndexBuffer buffer;
        bgfx::allocTransientIndexBuffer(&buffer, idxNum, index32);
        return TransientIndexBuffer{ buffer };
    }

    expected<TransientIndexBuffer, std::string> TransientIndexBuffer::create(DataView data, bool index32) noexcept
    {
        auto idxSize = index32 ? 4 : sizeof(VertexIndex);
        auto idxNum = static_cast<uint32_t>(data.size() / idxSize);
        auto result = create(idxNum, index32);
        if (result)
        {
            result.value().data().fill(data);
        }
        return result;
    }

    const bgfx::TransientIndexBuffer& TransientIndexBuffer::get() const noexcept
    {
        return _bgfx;
    }

    uint16_t TransientIndexBuffer::idx() const noexcept
    {
        return _bgfx.handle.idx;
    }

    DataView TransientIndexBuffer::data() noexcept
    {
        return { _bgfx.data, _bgfx.size };
    }

    const DataView TransientIndexBuffer::data() const noexcept
    {
        return { _bgfx.data, _bgfx.size };
    }    

    VertexDataWriter::VertexDataWriter(const bgfx::VertexLayout& layout, uint32_t size, OptionalRef<bx::AllocatorI> alloc) noexcept
        : _layout(layout)
        , _size(size)
        , _data(alloc)
    {
    }

    bool VertexDataWriter::write(bgfx::Attrib::Enum attr, uint32_t index, const std::array<float, 4>& finput, bool mark) noexcept
    {
        if (index >= _size || !_layout.has(attr))
        {
            return false;
        }
        auto dataPtr = prepareData();
        bgfx::vertexPack(&finput.front(), false, attr, _layout, dataPtr, index);
        if (mark)
        {
            markOne(attr, index);
        }
        return true;
    }

    void VertexDataWriter::load(Data&& data) noexcept
    {
        _data = std::move(data);
    }

    const bgfx::VertexLayout& VertexDataWriter::getLayout() const noexcept
    {
        return _layout;
    }

    uint32_t VertexDataWriter::getSize() const noexcept
    {
        return _size;
    }

    void* VertexDataWriter::prepareData() noexcept
    {
        auto size = _layout.getSize(_size);
        if (_data.size() < size)
        {
            _data.resize(size);
        }
        return _data.ptr();
    }

    Data&& VertexDataWriter::finish() noexcept
    {
        for (auto i = 0; i < bgfx::Attrib::Count; i++)
        {
            const auto attr = static_cast<bgfx::Attrib::Enum>(i);
            if (!_layout.has(attr))
            {
                continue;
            }
            uint8_t num;
            bgfx::AttribType::Enum type;
            bool normalized;
            bool asInt;
            _layout.decode(attr, num, type, normalized, asInt);
            
            std::array<float, 4> input{};
            input.fill(0.F);

            if (attr == bgfx::Attrib::Normal && num == 3)
            {
                input.at(2) = 1.F;
            }
            else if (attr == bgfx::Attrib::Indices)
            {
                input.fill(-1.F); // unused index is -1
            }
            else if (attr == bgfx::Attrib::Weight)
            {
                input.at(0) = 1.F; // take only first weight
            }
            else if (attr >= bgfx::Attrib::Color0 && attr <= bgfx::Attrib::Color3)
            {
                input.fill(normalized ? Colors::getMaxValue() : 1.F);
            }
            for (uint32_t j = 0; j < _size; j++)
            {
                if (!wasWritten(attr, j))
                {
                    write(attr, j, input);
                }
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
}