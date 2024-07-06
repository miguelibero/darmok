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
		FreetypeUtils::checkError(err);
		err = FT_Set_Pixel_Sizes(face, _defaultFontSize.x, _defaultFontSize.y);
		FreetypeUtils::checkError(err);
		err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		FreetypeUtils::checkError(err);

		return std::make_shared<Font>(std::make_unique<FontImpl>(face, std::move(data)));
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

	FontImpl::FontImpl(FT_Face face, Data&& data) noexcept
		: _face(face)
		, _data(std::move(data))
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
		FreetypeFontAtlasGenerator generator(_face, alloc);
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

	FreetypeFontAtlasGenerator::FreetypeFontAtlasGenerator(FT_Face face, bx::AllocatorI& alloc) noexcept
		: _face(face)
		, _alloc(alloc)
		, _size(1024)
	{
	}

	FreetypeFontAtlasGenerator& FreetypeFontAtlasGenerator::setSize(const glm::uvec2& size) noexcept
	{
		_size = size;
		return *this;
	}

	glm::uvec2 FreetypeFontAtlasGenerator::calcSpace(const std::unordered_set<FT_ULong>& chars) noexcept
	{
		glm::uvec2 pos(0);
		FT_UInt fontHeight = _face->size->metrics.y_ppem;
		for (auto bitmap : getBitmaps(chars))
		{
			if (pos.x + bitmap.width >= _size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
			}
			pos.x += bitmap.width;
		}
		return pos;
	}

	std::vector<FT_Bitmap> FreetypeFontAtlasGenerator::getBitmaps(const std::unordered_set<FT_ULong>& chars)
	{
		std::vector<FT_Bitmap> bitmaps;
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

			bitmaps.emplace_back(slot->bitmap);
		}

		return bitmaps;
	}

	Image FreetypeFontAtlasGenerator::operator()(const std::unordered_set<FT_ULong>& chars)
	{
		glm::uvec2 pos(0);
		FT_UInt fontHeight = _face->size->metrics.y_ppem;
		Image img(_size, _alloc, bimg::TextureFormat::R8);
		for(auto bitmap : getBitmaps(chars))
		{
			if (pos.x + bitmap.width >= _size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
				if (pos.y >= _size.y)
				{
					throw std::runtime_error("Texture overflow\n");
				}
			}
			glm::uvec2 size(bitmap.width, bitmap.rows);
			img.update(pos, size, DataView(bitmap.buffer, bitmap.rows * bitmap.pitch));
			pos.x += bitmap.width;
		}
		return img;
	}

	std::vector<std::filesystem::path> FreetypeFontAtlasImporterImpl::getOutputs(const Input& input)
	{
		std::vector<std::filesystem::path> outputs;
		return outputs;
	}

	std::ofstream FreetypeFontAtlasImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
	{
		if (outputIndex == 0)
		{
			return std::ofstream(path);
		}
		return std::ofstream(path, std::ios::binary);
	}

	void FreetypeFontAtlasImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{

	}

	const std::string& FreetypeFontAtlasImporterImpl::getName() const noexcept
	{
		static const std::string name("font_atlas");
		return name;
	}

	FreetypeFontAtlasImporter::FreetypeFontAtlasImporter() noexcept
		: _impl(std::make_unique<FreetypeFontAtlasImporterImpl>())
	{
	}

	FreetypeFontAtlasImporter::~FreetypeFontAtlasImporter() noexcept
	{
		// intentionally left blank
	}

	std::vector<std::filesystem::path> FreetypeFontAtlasImporter::getOutputs(const Input& input)
	{
		return _impl->getOutputs(input);
	}

	std::ofstream FreetypeFontAtlasImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
	{
		return _impl->createOutputStream(input, outputIndex, path);
	}

	void FreetypeFontAtlasImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		_impl->writeOutput(input, outputIndex, out);
	}

	const std::string& FreetypeFontAtlasImporter::getName() const noexcept
	{
		return _impl->getName();
	}
}