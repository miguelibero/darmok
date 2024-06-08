#include <darmok/data_stream.hpp>

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

    DataStreamBuffer::int_type DataStreamBuffer::overflow(int_type ch)
    {
        if (ch != EOF)
        {
            std::size_t oldSize = pptr() - pbase();
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
}