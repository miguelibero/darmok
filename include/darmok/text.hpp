#pragma once

#include <darmok/export.h>
#include <memory>
#include <string>
#include <bx/bx.h>

namespace darmok
{
    class FontImpl;

    class DARMOK_EXPORT Font final
    {
    public:
        Font(std::unique_ptr<FontImpl>&& impl) noexcept;
        ~Font();
    private:
        std::unique_ptr<FontImpl> _impl;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IFontLoader
    {
    public:
        using result_type = std::shared_ptr<Font>;

        virtual ~IFontLoader() = default;
        virtual result_type operator()(std::string_view name) = 0;
    };

    class TextImpl;

    class DARMOK_EXPORT Text final
    {
    public:
        Text(const std::shared_ptr<Font>& font, const std::string& content = "") noexcept;
        ~Text();
    private:
        std::unique_ptr<TextImpl> _impl;
    };
}