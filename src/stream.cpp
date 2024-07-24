#include <darmok/stream.hpp>
#include <bx/debug.h>
#include <bx/string.h>
#include <bx/platform.h>
#include <fstream>

namespace darmok
{
    void StreamUtils::copy(std::istream& input, std::ostream& output, size_t bufferSize)
    {
        std::vector<char> buffer(bufferSize);
        while (input.read(&buffer.front(), bufferSize) || input.gcount() > 0)
        {
            output.write(&buffer.front(), input.gcount());
        }
    }

    void StreamUtils::logDebug(const std::string& msg, bool error) noexcept
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

    void StreamUtils::writeUtf8Bom(std::ostream& out)
    {
        out << _utf8Bom;
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