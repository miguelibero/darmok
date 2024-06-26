#pragma once

#include <iostream>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    struct StreamUtils final
    {
        static void copyStream(std::istream& input, std::ostream& output, size_t bufferSize = 4096);
        static void logDebug(const std::string& msg, bool error = false) noexcept;
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
}