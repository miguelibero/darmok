#pragma once

#include <darmok/export.h>

#include <iostream>

#include <bx/readerwriter.h>

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
    private:
        DataViewStreamBuffer _buffer;
    };

    class DARMOK_EXPORT DataStreamBuffer final : public std::streambuf
    {
    public:
        DataStreamBuffer(Data& data, size_t overflowSizeIncrease = 64) noexcept;
        size_t size() const noexcept;
    protected:
        int_type overflow(int_type ch) override;
        std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which) override;
        std::streampos seekpos(std::streampos pos, std::ios_base::openmode which) override;
    private:
        Data& _data;
        size_t _overflowSizeIncrease;
    };

    class DARMOK_EXPORT DataOutputStream final : public std::ostream
    {
    public:
        DataOutputStream(Data& data) noexcept;
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
}