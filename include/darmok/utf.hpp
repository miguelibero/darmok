#pragma once

#include <darmok/export.h>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdint>

namespace darmok
{
    struct UtfChar;

    using UtfVector = std::vector<UtfChar>;

    struct DARMOK_EXPORT UtfChar final
    {
        uint32_t code;

        static const uint32_t invalidCode;

        UtfChar(uint32_t code = invalidCode) noexcept;

        UtfChar(std::string_view str);
        UtfChar(std::u8string_view str);

        [[nodiscard]] static UtfChar read(std::string_view& str);
        static size_t read(std::string_view str, UtfVector& chars);
        [[nodiscard]] static UtfChar read(std::u8string_view& str);
        static size_t read(std::u8string_view str, UtfVector& chars);
        static std::string toString(const UtfVector& chars);
        static std::u8string toUtfString(const UtfVector& chars);

        operator bool() const noexcept;
        operator std::string() const;
        operator std::u8string() const;
        bool valid() const noexcept;
        bool empty() const noexcept;
        uint8_t length() const noexcept;
        std::string toString() const;
        std::u8string toUtf8String() const;
        bool operator==(const UtfChar& other) const noexcept;
        bool operator!=(const UtfChar& other) const noexcept;
        bool operator<(const UtfChar& other) const noexcept;

    private:

        template<typename T>
        static std::basic_string<T> vectorToString(const UtfVector& chars)
        {
            std::basic_string<T> str;
            for (auto& chr : chars)
            {
                chr.appendToString(str);
            }
            return str;
        }

        template<typename T>
        static size_t doRead(std::basic_string_view<T> str, UtfVector& chars)
        {
            size_t count = 0;
            while (!str.empty())
            {
                auto code = readCode(str);
                chars.emplace_back(code);
                count++;
            }
            return count;
        }

        template<typename T>
        void appendToString(std::basic_string<T>& str) const
        {
            if (code <= 0x7F)
            {
                str.push_back(static_cast<char>(code));
            }
            else if (code <= 0x7FF)
            {
                str.push_back(static_cast<char>((code >> 6) | 0xC0));
                str.push_back(static_cast<char>((code & 0x3F) | 0x80));
            }
            else if (code <= 0xFFFF)
            {
                str.push_back(static_cast<char>((code >> 12) | 0xE0));
                str.push_back(static_cast<char>(((code >> 6) & 0x3F) | 0x80));
                str.push_back(static_cast<char>((code & 0x3F) | 0x80));
            }
            else if (code <= 0x10FFFF)
            {
                str.push_back(static_cast<char>((code >> 18) | 0xF0));
                str.push_back(static_cast<char>(((code >> 12) & 0x3F) | 0x80));
                str.push_back(static_cast<char>(((code >> 6) & 0x3F) | 0x80));
                str.push_back(static_cast<char>((code & 0x3F) | 0x80));
            }
            else
            {
                throw std::runtime_error("invalid Unicode code point");
            }
        }

        template<typename T>
        static uint32_t readCode(std::basic_string_view<T>& str)
        {
            if (str.empty())
            {
                throw std::out_of_range("Index out of range");
            }

            uint32_t code = 0;
            int len = 0;
            auto c = str.front();
            if (c <= 0x7F)
            {
                code = c;
                len = 1;
            }
            else if (c <= 0xBF)
            {
                throw std::invalid_argument("unexpected continuation byte encountered");
            }
            else if (c <= 0xDF)
            {
                code = c & 0x1F;
                len = 2;
            }
            else if (c <= 0xEF)
            {
                code = c & 0x0F;
                len = 3;
            }
            else if (c <= 0xF7)
            {
                code = c & 0x07;
                len = 4;
            }
            else
            {
                throw std::invalid_argument("invalid UTF-8 character encountered");
            }

            for (int i = 1; i < len; ++i)
            {
                if (i >= str.size() || (str[i] & 0xC0) != 0x80)
                {
                    throw std::invalid_argument("invalid continuation byte");
                }
                code = (code << 6) | (str[i] & 0x3F);
            }

            str = std::basic_string_view<T>(str.begin() + len, str.end());
            return code;
        }
    };
}

namespace std
{
    template<typename T>
    struct hash;

    template<> struct hash<darmok::UtfChar>
    {
        std::size_t operator()(const darmok::UtfChar& key) const noexcept;
    };
}