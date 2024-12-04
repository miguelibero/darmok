#include <darmok/data_stream.hpp>
#include <darmok/data.hpp>

namespace darmok
{
    DataViewStreamBuffer::DataViewStreamBuffer(const DataView& data) noexcept
    {
        auto base = (char*)data.ptr();
        auto size = data.size();
        setg(base, base, base + size);
        setp(base, base + size);
    }

    DataInputStream::DataInputStream(const DataView& data) noexcept
    : std::istream(&_buffer)
    , _buffer(data)
    {
        rdbuf(&_buffer);
    }

    DataStreamBuffer::DataStreamBuffer(Data& data, size_t overflowSizeIncrease) noexcept
        : _data(data)
        , _overflowSizeIncrease(overflowSizeIncrease)
    {
        auto base = (char*)data.ptr();
        auto size = data.size();
        setg(base, base, base + size);
        setp(base, base + size);
    }

    size_t DataStreamBuffer::size() const noexcept
    {
        return _data.size();
    }

    std::streampos DataStreamBuffer::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
    {
        if (which & std::ios_base::out)
        {
            std::streamoff newpos;
            if (way == std::ios_base::beg)
            {
                newpos = off;
            }
            else if (way == std::ios_base::cur)
            {
                newpos = (pptr() - pbase()) + off;
            }
            else if (way == std::ios_base::end)
            {
                newpos = size() + off;
            }
            else {
                return -1;
            }
            if (newpos < 0 || newpos > size())
            {
                return -1;
            }
            setp(pbase(), epptr());
            pbump(newpos);
            return newpos;
        }
        return -1;
    }

    std::streampos DataStreamBuffer::seekpos(std::streampos pos, std::ios_base::openmode which)
    {
        if (which & std::ios_base::out)
        {
            if (pos < 0 || pos > size())
            {
                return -1;
            }
            setp(pbase(), epptr());
            pbump(pos);
            return pos;
        }
        return -1;
    }

    DataStreamBuffer::int_type DataStreamBuffer::overflow(int_type ch)
    {
        if (ch != EOF)
        {
            auto oldSize = int(pptr() - pbase());
            _data.resize(_data.size() + _overflowSizeIncrease);
            auto base = (char*)_data.ptr();
            auto size = _data.size();
            setp(base, base + size);
            pbump(oldSize);
            *pptr() = static_cast<char>(ch);
            pbump(1);
        }
        return ch;
    }

    DataOutputStream::DataOutputStream(Data& data) noexcept
        : std::ostream(&_buffer)
        , _buffer(data)
    {
        rdbuf(&_buffer);
    }

    DataMemoryBlock::DataMemoryBlock(Data& data) noexcept
        : _data(data)
    {
    }

    void* DataMemoryBlock::more(uint32_t size)
    {
        _data.resize(size);
        return _data.ptr();
    }

    uint32_t DataMemoryBlock::getSize()
    {
        return _data.size();
    }
}