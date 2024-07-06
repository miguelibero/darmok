#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>
#include <darmok/data.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/string.hpp>
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

		static void tokenize(std::string_view str, std::unordered_set<FT_ULong>& chars, bool remove = false)
		{
			while(!str.empty())
			{
				auto chr = StringUtils::getUtf8Char(str);
				if (remove)
				{
					chars.erase(chr);
				}
				else
				{
					chars.emplace(chr);
				}
			}
		}
	};

	FreetypeFontLoaderImpl::FreetypeFontLoaderImpl(IDataLoader& dataLoader, const glm::uvec2& defaultFontSize)
		: _dataLoader(dataLoader)
		, _defaultFontSize(defaultFontSize)
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
		FT_Face face = nullptr;
		auto err = FT_New_Memory_Face(_library, (const FT_Byte*)data.ptr(), data.size(), 0, &face);
		FT_Set_Pixel_Sizes(face, _defaultFontSize.x, _defaultFontSize.y);
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

	void TextRendererImpl::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_scene = scene;
		_alloc = app.getAssets().getAllocator();
	}

	void TextRendererImpl::shutdown() noexcept
	{
		_scene.reset();
		_alloc.reset();
	}

	bool TextRendererImpl::update()
	{
		if (!_scene || !_alloc)
		{
			return false;
		}
		auto texts = _scene->getComponentView<Text>();
		std::unordered_set<std::shared_ptr<Font>> fonts;
		for (auto [entity, text] : texts.each())
		{
			fonts.insert(text.getImpl().getFont());
		}
		bool changed = false;
		for (auto& font : fonts)
		{
			if (font->getImpl().update(_alloc.value()))
			{
				changed = true;
			}
		}
		return changed;
	}

	bgfx::ViewId TextRendererImpl::afterRender(bgfx::ViewId viewId)
	{
		return viewId;
	}

	TextRenderer::TextRenderer() noexcept
		: _impl(std::make_unique<TextRendererImpl>())
	{
	}

	TextRenderer::~TextRenderer() noexcept
	{
		// intentionally left empty
	}

	void TextRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_impl->init(cam, scene, app);
	}

	void TextRenderer::shutdown() noexcept
	{
		_impl->shutdown();
	}

	void TextRenderer::update(float deltaTime)
	{
		_impl->update();
	}

	bgfx::ViewId TextRenderer::afterRender(bgfx::ViewId viewId)
	{
		return _impl->afterRender(viewId);
	}

	FontImpl::FontImpl(FT_Face face) noexcept
		: _face(face)
	{
	}

	FontImpl::~FontImpl() noexcept
	{
		FT_Done_Face(_face);
	}

	void FontImpl::removeContent(std::string_view content)
	{
		FreetypeUtils::tokenize(content, _chars, true);
	}

	void FontImpl::addContent(std::string_view content)
	{
		FreetypeUtils::tokenize(content, _chars);
	}

	FT_Face FontImpl::getFace() const  noexcept
	{
		return _face;
	}

	bool FontImpl::update(bx::AllocatorI& alloc)
	{
		if (_renderedChars == _chars)
		{
			return false;
		}
		FontAtlasGeneratorImpl generator(_face, alloc);
		Image img = generator(_chars);

		if (_tex && _tex->getSize() == img.getSize())
		{
			_tex->update(img.getData());
		}
		else
		{
			_tex.emplace(img);
		}
		
		_renderedChars = _chars;
		return true;
	}

	Font::Font(std::unique_ptr<FontImpl>&& impl) noexcept
		: _impl(std::move(impl))
	{
	}

	Font::~Font()
	{
		// intentionally left blank
	}

	FontImpl& Font::getImpl()
	{
		return *_impl;
	}

	const FontImpl& Font::getImpl() const
	{
		return *_impl;
	}

	TextImpl::TextImpl(const std::shared_ptr<Font>& font, const std::string& content) noexcept
		: _font(font)
	{
		setContent(content);
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
		_font->getImpl().removeContent(_content);
		_font->getImpl().addContent(str);
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

	TextImpl& Text::getImpl()
	{
		return *_impl;
	}

	const TextImpl& Text::getImpl() const
	{
		return *_impl;
	}

	FontAtlasGeneratorImpl::FontAtlasGeneratorImpl(FT_Face face, bx::AllocatorI& alloc) noexcept
		: _face(face)
		, _alloc(alloc)
	{
	}

	Image FontAtlasGeneratorImpl::operator()(const std::unordered_set<FT_ULong>& chars)
	{
		glm::uvec2 size(1024);
		glm::uint fontHeight = 48;
		glm::uvec2 pos(0);
		Image img(Colors::black(), _alloc, size);
		for (auto chr : chars)
		{
			auto glyphIdx = FT_Get_Char_Index(_face, chr);
			if (glyphIdx == 0)
			{
				continue;
			}

			auto err = FT_Load_Glyph(_face, glyphIdx, FT_LOAD_DEFAULT);
			FreetypeUtils::checkError(err);

			auto slot = _face->glyph;
			err = FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);
			FreetypeUtils::checkError(err);

			auto bitmap = slot->bitmap;

			if (pos.x + bitmap.width >= size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
				if (pos.y >= size.y)
				{
					throw std::runtime_error("Texture overflow\n");
				}
			}
			img.update(pos, DataView(bitmap.buffer, bitmap.width * bitmap.pitch));
			pos.x += bitmap.width;
		}
		return img;
	}

	FontAtlasGenerator::FontAtlasGenerator(const Font& font, bx::AllocatorI& alloc) noexcept
		: _impl(std::make_unique<FontAtlasGeneratorImpl>(font.getImpl().getFace(), alloc))
	{
	}

	FontAtlasGenerator::~FontAtlasGenerator() noexcept
	{
		// intentionally left blank
	}

	Image FontAtlasGenerator::operator()(std::string_view chars)
	{
		std::unordered_set<FT_ULong> ftchars;
		FreetypeUtils::tokenize(chars, ftchars);
		return (*_impl)(ftchars);
	}
}