#pragma once

#include <iostream>
#include <darmok/optional_ref.hpp>
#include <bx/readerwriter.h>
#include <filesystem>

namespace darmok
{
    struct StreamUtils final
    {
        static void copy(std::istream& input, std::ostream& output, size_t bufferSize = 4096);
        static void logDebug(const std::string& msg, bool error = false) noexcept;
        static void writeUtf8Bom(std::ostream& out);
    };

    class PrefixBuffer final : public std::streambuf
    {
    public:
        PrefixBuffer(OptionalRef<std::streambuf> buffer, const std::string& prefix) noexcept;
    protected:
        int overflow(int ch) override;
    private:
        OptionalRef<std::streambuf> _buffer;
        std::string _prefix;
        bool _needsPrefix;
    };

    class PrefixStream final : public std::ostream
    {
    public:
        PrefixStream(std::ostream& os, const std::string& prefix) noexcept;
    private:
        PrefixBuffer _buffer;
    };

    class StreamWriter final : public bx::WriterI
    {
    public:
        StreamWriter(std::ostream& stream) noexcept;
        int32_t write(const void* data, int32_t size, bx::Error* err) override;
    private:
        std::ostream& _stream;
    };

    class StreamReader final : public bx::ReaderI
    {
    public:
        StreamReader(std::istream& stream) noexcept;
        int32_t read(void* data, int32_t size, bx::Error* err) override;
    private:
        std::istream& _stream;
    };
}