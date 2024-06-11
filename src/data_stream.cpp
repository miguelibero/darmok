#include <darmok/data_stream.hpp>
#include <darmok/data.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

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
            int oldSize = pptr() - pbase();
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

    void save(cereal::BinaryOutputArchive& archive, const DataView& data)
    {
        archive << data.size();
        archive.saveBinary(data.ptr(), data.size());
    }

    void save(cereal::BinaryOutputArchive& archive, const Data& data)
    {
        save(archive, data.view());
    }

    void load(cereal::BinaryInputArchive& archive, Data& data)
    {
        size_t size;
        archive >> size;
        data.resize(size);
        archive.loadBinary(data.ptr(), size);
    }

    void save(cereal::XMLOutputArchive& archive, const DataView& data)
    {
        archive(data.view().to_string());
    }

    void save(cereal::XMLOutputArchive& archive, const Data& data)
    {
        save(archive, data.view());
    }

    void save(cereal::JSONOutputArchive& archive, const DataView& data)
    {
        archive(data.view().to_string());
    }

    void save(cereal::JSONOutputArchive& archive, const Data& data)
    {
        save(archive, data.view());
    }

    void load(cereal::XMLInputArchive& archive, Data& data)
    {
        std::string hex;
        archive >> hex;
        data = Data::fromHex(hex);
    }

    void load(cereal::JSONInputArchive& archive, Data& data)
    {
        std::string hex;
        archive >> hex;
        data = Data::fromHex(hex);
    }
}