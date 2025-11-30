#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/expected.hpp>

#include <iostream>
#include <filesystem>

#include <bx/readerwriter.h>
#include <nlohmann/json.hpp>

namespace darmok
{
    namespace StreamUtils
    {
        expected<std::string, std::string> readString(std::istream& input) noexcept;
        expected<std::string, std::string> readString(std::filesystem::path& path) noexcept;
        void copy(std::istream& input, std::ostream& output, size_t bufferSize = 4096);
        void log(std::string_view msg, bool error = false) noexcept;
        void writeUtf8Bom(std::ostream& out);

        expected<nlohmann::json, std::string> parseJson(std::istream&& input) noexcept;
        expected<nlohmann::ordered_json, std::string> parseOrderedJson(std::istream&& input) noexcept;
        expected<nlohmann::json, std::string> parseJson(const std::filesystem::path& path) noexcept;
        expected<nlohmann::ordered_json, std::string> parseOrderedJson(const std::filesystem::path& path) noexcept;
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