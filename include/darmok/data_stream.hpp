#pragma once

#include <darmok/export.h>
#include <iostream>
#include <cereal/archives/binary.hpp>
#include <bx/readerwriter.h>

namespace cereal
{
    class XMLOutputArchive;
    class XMLInputArchive;
    class JSONOutputArchive;
    class JSONInputArchive;
}

namespace darmok
{
    class DataView;
    class Data;

    class DARMOK_EXPORT DataViewStreamBuffer final : public std::streambuf
    {
    public:
        DataViewStreamBuffer(const DataView& data) noexcept;
    };

    class DARMOK_EXPORT DataInputStream final : public std::istream
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

    class DARMOK_EXPORT DataStreamBuffer final : public std::streambuf
    {
    public:
        DataStreamBuffer(Data& data, size_t overflowSizeIncrease = 64) noexcept;
    protected:
        int_type overflow(int_type ch) override;
    private:
        Data& _data;
        size_t _overflowSizeIncrease;
    };

    class DARMOK_EXPORT DataOutputStream final : public std::ostream
    {
    public:
        DataOutputStream(Data& data) noexcept;

        template<typename T>
        static void write(Data& data, const T& val)
        {
            DataOutputStream stream(data);
            cereal::BinaryOutputArchive archive(stream);
            archive(val);
        }
    private:
        DataStreamBuffer _buffer;
    };

    class DARMOK_EXPORT DataMemoryBlock final : public bx::MemoryBlockI
    {
    public:
        DataMemoryBlock(Data& data)noexcept;
        void* more(uint32_t size = 0) override;
        uint32_t getSize() override;
    private:
        Data& _data;
    };

    DARMOK_EXPORT void save(cereal::BinaryOutputArchive& archive, const DataView& data);
    DARMOK_EXPORT void save(cereal::BinaryOutputArchive& archive, const Data& data);
    DARMOK_EXPORT void save(cereal::XMLOutputArchive& archive, const DataView& data);
    DARMOK_EXPORT void save(cereal::XMLOutputArchive& archive, const Data& data);
    DARMOK_EXPORT void save(cereal::JSONOutputArchive& archive, const DataView& data);
    DARMOK_EXPORT void save(cereal::JSONOutputArchive& archive, const Data& data);
    DARMOK_EXPORT void load(cereal::BinaryInputArchive& archive, Data& data);
    DARMOK_EXPORT void load(cereal::XMLInputArchive& archive, Data& data);
    DARMOK_EXPORT void load(cereal::JSONInputArchive& archive, Data& data);
}