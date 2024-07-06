#pragma once

#include <darmok/text.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H


struct FT_FaceRec_;
typedef struct FT_FaceRec_* FT_Face;

namespace darmok
{
    class IDataLoader;

    class FreetypeFontLoaderImpl final
    {
    public:
        FreetypeFontLoaderImpl(IDataLoader& dataLoader);
        ~FreetypeFontLoaderImpl();
        std::shared_ptr<Font> operator()(std::string_view name);
    private:
        IDataLoader& _dataLoader;
        FT_Library _library;
    };


    class FontImpl final
    {
    public:
        FontImpl(FT_Face face) noexcept;
        ~FontImpl() noexcept;
    private:
        FT_Face _face;
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