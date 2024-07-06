#pragma once

#include <darmok/text.hpp>

namespace darmok
{
    class FreetypeFontLoaderImpl final
    {
    public:
        std::shared_ptr<Font> operator()(std::string_view name);
    };

    class FontImpl final
    {

    };

    class TextImpl final
    {
    public:
        TextImpl(const std::shared_ptr<Font>& font, const std::string& content = "") noexcept;

        std::shared_ptr<Font> getFont() noexcept;
        void setFont(const std::shared_ptr<Font>& font) noexcept;
        const std::string& getContent() noexcept;
        void setContent(const std::string& str) noexcept;
    private:
        std::shared_ptr<Font> _font;
        std::string _content;
    };
}