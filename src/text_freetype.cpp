#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>
#include "text_freetype.hpp"

namespace darmok
{
	std::shared_ptr<Font> FreetypeFontLoaderImpl::operator()(std::string_view name)
	{
		auto impl = std::make_unique<FontImpl>();
		return std::make_shared<Font>(std::move(impl));
	}

	FreetypeFontLoader::FreetypeFontLoader() noexcept
		: _impl(std::make_unique< FreetypeFontLoaderImpl>())
	{
	}

	FreetypeFontLoader::~FreetypeFontLoader() noexcept
	{
	}

	std::shared_ptr<Font> FreetypeFontLoader::operator()(std::string_view name)
	{
		return (*_impl)(name);
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