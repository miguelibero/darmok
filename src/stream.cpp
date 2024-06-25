#include <darmok/stream.hpp>
#include <bx/platform.h>

#if BX_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace darmok
{
    void StreamUtils::copyStream(std::istream& input, std::ostream& output, size_t bufferSize)
    {
        std::vector<char> buffer(bufferSize);
        while (input.read(&buffer.front(), bufferSize) || input.gcount() > 0)
        {
            output.write(&buffer.front(), input.gcount());
        }
    }

    void StreamUtils::logDebug(const std::string& msg) noexcept
    {
        std::cerr << msg << std::endl;
#if BX_PLATFORM_WINDOWS
        OutputDebugString(msg.c_str());
#endif
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
}