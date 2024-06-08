#pragma once

#include <darmok/data.hpp>
#include <iostream>
#include <cereal/archives/binary.hpp>

namespace darmok
{
    class DataViewStreamBuffer final : public std::streambuf
    {
    public:
        DataViewStreamBuffer(const DataView& data) noexcept;
    };

    class DataInputStream final : public std::istream
    {
    public:
        DataInputStream(const DataView& data) noexcept;

        template<typename T>
        static void read(const DataView& data, T& val)
        {
            DataInputStream stream(data);
            cereal::BinaryInputArchive archive(stream);
            archive(val);
        }
    private:
        DataViewStreamBuffer _buffer;
    };

    class DataStreamBuffer final : public std::streambuf
    {
    public:
        DataStreamBuffer(Data& data, size_t overflowSizeIncrease = 64) noexcept;
    protected:
        int_type overflow(int_type ch) override;
    private:
        Data& _data;
        size_t _overflowSizeIncrease;
    };

    class DataOutputStream final : public std::ostream
    {
    public:
        DataOutputStream(Data& data) noexcept;

        template<typename T>
        static void write(const DataView& data, const T& val)
        {
            DataOutputStream stream(data);
            cereal::BinaryInputArchive archive(stream);
            archive(val);
        }
    private:
        DataStreamBuffer _buffer;
    };
}