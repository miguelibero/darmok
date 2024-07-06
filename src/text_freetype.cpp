#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>
#include <darmok/data.hpp>
#include "text_freetype.hpp"
#include <stdexcept>

namespace darmok
{
	struct FreetypeUtils final
	{
		static const char* getErrorMessage(FT_Error err) noexcept
		{
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
			return "(Unknown error)";
		}

		static void checkError(FT_Error err)
		{
			if (err)
			{
				throw std::runtime_error(getErrorMessage(err));
			}
		}
	};

	FreetypeFontLoaderImpl::FreetypeFontLoaderImpl(IDataLoader& dataLoader)
		: _dataLoader(dataLoader)
	{
		auto err = FT_Init_FreeType(&_library);
		FreetypeUtils::checkError(err);
	}

	FreetypeFontLoaderImpl::~FreetypeFontLoaderImpl()
	{
		FT_Done_FreeType(_library);
	}

	std::shared_ptr<Font> FreetypeFontLoaderImpl::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		FT_Face face;
		auto err = FT_New_Memory_Face(_library, (const FT_Byte*)data.ptr(), data.size(), 0, &face);
		FreetypeUtils::checkError(err);

		return std::make_shared<Font>(std::make_unique<FontImpl>(face));
	}

	FreetypeFontLoader::FreetypeFontLoader(IDataLoader& dataLoader)
		: _impl(std::make_unique<FreetypeFontLoaderImpl>(dataLoader))
	{
	}

	FreetypeFontLoader::~FreetypeFontLoader() noexcept
	{
		// intentionally left blank
	}

	std::shared_ptr<Font> FreetypeFontLoader::operator()(std::string_view name)
	{
		return (*_impl)(name);
	}

	FontImpl::FontImpl(FT_Face face) noexcept
		: _face(face)
	{
	}

	FontImpl::~FontImpl() noexcept
	{
		FT_Done_Face(_face);
	}

	Font::Font(std::unique_ptr<FontImpl>&& impl) noexcept
		: _impl(std::move(impl))
	{
	}

	Font::~Font()
	{
		// intentionally left blank
	}

	TextImpl::TextImpl(const std::shared_ptr<Font>& font, const std::string& content) noexcept
		: _font(font)
		, _content(content)
	{
	}

	std::shared_ptr<Font> TextImpl::getFont() noexcept
	{
		return _font;
	}

	void TextImpl::setFont(const std::shared_ptr<Font>& font) noexcept
	{
		_font = font;
	}

	const std::string& TextImpl::getContent() noexcept
	{
		return _content;
	}

	void TextImpl::setContent(const std::string& str) noexcept
	{
		_content = str;
	}

	Text::Text(const std::shared_ptr<Font>& font, const std::string& content) noexcept
		: _impl(std::make_unique<TextImpl>(font, content))
	{
	}

	Text::~Text()
	{
		// intentionally left blank
	}
}