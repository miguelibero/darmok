#include <darmok/stream.hpp>
#include <bx/debug.h>
#include <bx/string.h>
#include <bx/platform.h>
#include <fstream>

namespace darmok
{
    namespace StreamUtils
    {
        expected<std::string, std::string> readString(std::istream& input) noexcept
        {
            if (input.fail())
            {
                return unexpected{ "could not open input stream" };
            }

            if (input.tellg() < 0)
            {
                std::ostringstream oss;
                oss << input.rdbuf();
                return oss.str();
            }

            input.seekg(0, std::ios::end);
            auto size = input.tellg();
            input.seekg(0, std::ios::beg);

            std::string str(size, '\0');
            if (!input.read(&str.front(), size))
            {
                return unexpected{ "could not read input stream" };
            }

            return str;
        }

        expected<std::string, std::string> readString(std::filesystem::path& path) noexcept
        {
            if (!std::filesystem::exists(path))
            {
                return unexpected{ "vertex shader not found" };
            }
            std::ifstream input{ path, std::ios::binary };
            return readString(input);
        }

        void copy(std::istream& input, std::ostream& output, size_t bufferSize)
        {
            std::vector<char> buffer(bufferSize);
            while (input.read(&buffer.front(), bufferSize) || input.gcount() > 0)
            {
                output.write(&buffer.front(), input.gcount());
            }
        }

        void logDebug(const std::string& msg, bool error) noexcept
        {
            if (error)
            {
                std::cerr << msg << std::endl;
            }
            else
            {
                std::cout << msg << std::endl;
            }
            bx::debugOutput(bx::StringView(msg.data(), msg.size()));
        }

        static const unsigned char _utf8Bom[3] = { 0xEF, 0xBB, 0xBF };

        void writeUtf8Bom(std::ostream& out)
        {
            out << _utf8Bom;
        }

        expected<nlohmann::json, std::string> parseJson(std::istream&& input) noexcept
        {
            try
            {
                return nlohmann::json::parse(input);
            }
            catch (const std::exception& ex)
            {
                return unexpected{ ex.what() };
            }
        }

        expected<nlohmann::ordered_json, std::string> parseOrderedJson(std::istream&& input) noexcept
        {
            try
            {
                return nlohmann::ordered_json::parse(input);
            }
            catch (const std::exception& ex)
            {
                return unexpected{ ex.what() };
            }
        }

        expected<nlohmann::json, std::string> parseJson(const std::filesystem::path& path) noexcept
        {
            return parseJson(std::ifstream{ path });
        }

        expected<nlohmann::ordered_json, std::string> parseOrderedJson(const std::filesystem::path& path) noexcept
        {
            return parseOrderedJson(std::ifstream{ path });
        }
    }

    PrefixBuffer::PrefixBuffer(OptionalRef<std::streambuf> buffer, const std::string& prefix) noexcept
        : _buffer(buffer), _prefix(prefix), _needsPrefix(true)
    {
    }

    int PrefixBuffer::overflow(int ch)
    {
        if (_needsPrefix && ch != EOF)
        {
            _buffer->sputn(_prefix.c_str(), _prefix.size());
            _needsPrefix = false;
        }
        if (ch == '\n')
        {
            _needsPrefix = true;
        }
        return _buffer->sputc(ch);
    }

    PrefixStream::PrefixStream(std::ostream& os, const std::string& prefix) noexcept
      : std::ostream(&_buffer)
      , _buffer(os.rdbuf(), prefix)
    {
    }

    StreamWriter::StreamWriter(std::ostream& stream) noexcept
        : _stream(stream)
    {
    }

    int32_t StreamWriter::write(const void* data, int32_t size, bx::Error* err)
    {
        _stream.write((const char*)data, size);
        return size;
    }

    StreamReader::StreamReader(std::istream& stream) noexcept
        : _stream(stream)
    {
    }

    int32_t StreamReader::read(void* data, int32_t size, bx::Error* err)
    {
        _stream.read((char*)data, size);
        return size;
    }
}