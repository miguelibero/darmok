#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>
#include <darmok/data.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/stream.hpp>
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

		static void tokenize(std::string_view str, std::unordered_set<Utf8Char>& chars, bool remove = false)
		{
			while(!str.empty())
			{
				auto chr = Utf8Char::read(str);
				if (!chr)
				{
					continue;
				}
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
		, _library(nullptr)
	{
	}

	FreetypeFontLoaderImpl::~FreetypeFontLoaderImpl()
	{
		shutdown();
	}

	void FreetypeFontLoaderImpl::init(App& app)
	{
		shutdown();

		auto err = FT_Init_FreeType(&_library);
		FreetypeUtils::checkError(err);
	}

	void FreetypeFontLoaderImpl::shutdown()
	{
		if (_library)
		{
			auto err = FT_Done_FreeType(_library);
			_library = nullptr;
			FreetypeUtils::checkError(err);
		}
	}

	std::shared_ptr<Font> FreetypeFontLoaderImpl::operator()(std::string_view name)
	{
		if (!_library)
		{
			throw std::runtime_error("freetype library not initialized");
		}
		auto data = _dataLoader(name);
		FT_Face face = nullptr;

		auto err = FT_New_Memory_Face(_library, (const FT_Byte*)data.ptr(), data.size(), 0, &face);
		FreetypeUtils::checkError(err);
		err = FT_Set_Pixel_Sizes(face, _defaultFontSize.x, _defaultFontSize.y);
		FreetypeUtils::checkError(err);
		err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		FreetypeUtils::checkError(err);

		return std::make_shared<Font>(std::make_unique<FontImpl>(face, _library, std::move(data)));
	}

	FreetypeFontLoader::FreetypeFontLoader(IDataLoader& dataLoader)
		: _impl(std::make_unique<FreetypeFontLoaderImpl>(dataLoader))
	{
	}

	FreetypeFontLoader::~FreetypeFontLoader() noexcept
	{
		// intentionally left blank
	}

	void FreetypeFontLoader::init(App& app)
	{
		_impl->init(app);
	}

	void FreetypeFontLoader::shutdown()
	{
		_impl->shutdown();
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

	FontImpl::FontImpl(FT_Face face, FT_Library library, Data&& data) noexcept
		: _face(face)
		, _data(std::move(data))
		, _library(library)
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
		FreetypeFontAtlasGenerator generator(_face, _library, alloc);
		auto atlas = generator(_chars);

		if (_tex && _tex->getSize() == atlas.image.getSize())
		{
			_tex->update(atlas.image.getData());
		}
		else
		{
			_tex.emplace(atlas.image);
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

	FreetypeFontAtlasGenerator::FreetypeFontAtlasGenerator(FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept
		: _face(face)
		, _library(library)
		, _alloc(alloc)
		, _size(1024)
	{
	}

	FreetypeFontAtlasGenerator& FreetypeFontAtlasGenerator::setSize(const glm::uvec2& size) noexcept
	{
		_size = size;
		return *this;
	}

	glm::uvec2 FreetypeFontAtlasGenerator::calcSpace(const std::unordered_set<Utf8Char>& chars) noexcept
	{
		glm::uvec2 pos(0);
		FT_UInt fontHeight = _face->size->metrics.y_ppem;
		for (auto& [chr, idx] : getIndices(chars))
		{
			auto& bitmap = renderBitmap(idx);
			if (pos.x + bitmap.width >= _size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
			}
			pos.x += bitmap.width;
		}
		return pos;
	}

	std::unordered_map<Utf8Char, FT_UInt> FreetypeFontAtlasGenerator::getIndices(const std::unordered_set<Utf8Char>& chars) const
	{
		std::unordered_map<Utf8Char, FT_UInt> indices;
		if (chars.empty())
		{
			FT_UInt idx;
			auto code = FT_Get_First_Char(_face, &idx);
			while (idx != 0)
			{
				auto chr = Utf8Char(code);
				if (chr)
				{
					indices.emplace(chr, idx);
				}
				code = FT_Get_Next_Char(_face, code, &idx);
			}
		}
		else
		{
			for (auto& chr : chars)
			{
				auto idx = FT_Get_Char_Index(_face, chr.data);
				if (idx != 0)
				{
					indices.emplace(chr, idx);
				}
			}
		}
		return indices;
	}

	const FT_Bitmap& FreetypeFontAtlasGenerator::renderBitmap(FT_UInt index)
	{
		auto err = FT_Load_Glyph(_face, index, FT_LOAD_DEFAULT);
		FreetypeUtils::checkError(err);
		err = FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_NORMAL);
		FreetypeUtils::checkError(err);
		return _face->glyph->bitmap;
	}

	FontAtlas FreetypeFontAtlasGenerator::operator()(const std::unordered_set<Utf8Char>& chars)
	{
		glm::uvec2 pos(0);
		FT_UInt fontHeight = _face->size->metrics.y_ppem;
		FontAtlas atlas(Image(_size, _alloc, bimg::TextureFormat::R8));
		for(auto& [chr, idx] : getIndices(chars))
		{
			auto& bitmap = renderBitmap(idx);
			if (pos.x + bitmap.width >= _size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
				if (pos.y >= _size.y)
				{
					throw std::runtime_error("Texture overflow\n");
				}
			}
			auto& elm = atlas.elements.emplace_back();
			elm.size = glm::uvec2(bitmap.width, bitmap.rows);
			elm.texturePosition = pos;
			elm.name = chr;
			atlas.image.update(pos, elm.size, DataView(bitmap.buffer, bitmap.rows * bitmap.pitch));
			pos.x += bitmap.width;
		}
		return atlas;
	}

	FontAtlas FreetypeFontAtlasGenerator::operator()(std::string_view str)
	{
		auto chars = Utf8Char::tokenize(str);
		return (*this)(std::unordered_set<Utf8Char>(chars.begin(), chars.end()));
	}

	FreetypeFontAtlasImporterImpl::FreetypeFontAtlasImporterImpl()
		: _library(nullptr)
		, _face(nullptr)
	{
		auto err = FT_Init_FreeType(&_library);
		FreetypeUtils::checkError(err);
	}

	FreetypeFontAtlasImporterImpl::~FreetypeFontAtlasImporterImpl()
	{
		auto err = FT_Done_FreeType(_library);
		FreetypeUtils::checkError(err);
	}

	bool FreetypeFontAtlasImporterImpl::startImport(const Input& input, bool dry)
	{
		if (input.config.is_null())
		{
			return false;
		}
		if (dry)
		{
			return true;
		}

		glm::uvec2 size(100);
		if (input.config.contains("fontSize"))
		{
			auto& sizeConfig = input.config["fontSize"];
			if (sizeConfig.is_array())
			{
				std::vector<glm::uint> s = sizeConfig;
				size.x = s[0]; size.y = s[1];
			}
			else
			{
				size.x = size.y = sizeConfig;
			}
		}

		auto err = FT_New_Face(_library, input.path.string().c_str(), 0, &_face);
		FreetypeUtils::checkError(err);
		err = FT_Set_Pixel_Sizes(_face, size.x, size.y);
		FreetypeUtils::checkError(err);
		err = FT_Select_Charmap(_face, FT_ENCODING_UNICODE);
		FreetypeUtils::checkError(err);

		FreetypeFontAtlasGenerator generator(_face, _library, _alloc);
		std::string chars;
		if (input.config.contains("characters"))
		{
			chars = input.config["characters"];
		}

		_atlas = generator(chars);

		auto basePath = input.getRelativePath().parent_path();
		auto stem = StringUtils::getFileStem(input.path.filename().string());

		_imagePath = basePath;
		_atlasPath = basePath;

		if (input.config.contains("imagePath"))
		{
			_imagePath /= input.config["imagePath"];
		}
		else
		{
			_imagePath /= std::filesystem::path(stem + ".tga");

		}

		if (input.config.contains("atlasPath"))
		{
			_atlasPath /= input.config["atlasPath"];
		}
		else
		{
			_atlasPath /= std::filesystem::path(stem + ".xml");
		}

		return true;
	}

	void FreetypeFontAtlasImporterImpl::endImport(const Input& input)
	{
		FT_Done_Face(_face);
		_face = nullptr;
		_atlas.reset();
	}

	std::vector<std::filesystem::path> FreetypeFontAtlasImporterImpl::getOutputs(const Input& input)
	{
		return { _imagePath, _atlasPath };
	}

	std::ofstream FreetypeFontAtlasImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
	{
		if (outputIndex == 0)
		{
			return std::ofstream(path, std::ios::binary);
		}
		return std::ofstream(path);
	}

	void FreetypeFontAtlasImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		if (outputIndex == 0)
		{
			_atlas->image.write(ImageEncoding::Tga, out);
			return;
		}
		auto relImagePath = std::filesystem::relative(_imagePath, _atlasPath.parent_path());
		TextureAtlasData atlasData
		{
			.imagePath = relImagePath,
			.elements = _atlas->elements,
			.size = _atlas->image.getSize()
		};
		pugi::xml_document doc;
		atlasData.write(doc);
		doc.save(out, PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
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

	bool FreetypeFontAtlasImporter::startImport(const Input& input, bool dry)
	{
		return _impl->startImport(input, dry);
	}

	void FreetypeFontAtlasImporter::endImport(const Input& input)
	{
		_impl->endImport(input);
	}
}