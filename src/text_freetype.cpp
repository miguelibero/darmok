#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/data.hpp>
#include <darmok/stream.hpp>
#include <darmok/program.hpp>
#include <darmok/utf.hpp>
#include "text_freetype.hpp"
#include <stdexcept>
#include <pugixml.hpp>

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
			if (!err)
			{
				return "";
			}
			return "(Unknown error)";
		}

		static void checkError(FT_Error err)
		{
			if (err)
			{
				auto msg = getErrorMessage(err);
				throw std::runtime_error(msg);
			}
		}
	};

	DataFreetypeFontDefinitionLoader::DataFreetypeFontDefinitionLoader(IDataLoader& dataLoader, bool skipInvalid) noexcept
		: _dataLoader(dataLoader)
		, _skipInvalid(skipInvalid)
	{
	}

	std::string DataFreetypeFontDefinitionLoader::checkFontData(const DataView& data) noexcept
	{
		FT_Library library;
		auto err = FT_Init_FreeType(&library);
		if (err)
		{
			return FreetypeUtils::getErrorMessage(err);
		}

		FT_Face face;
		FT_Open_Args args;
		FT_StreamRec stream;

		stream.base = (unsigned char*)data.ptr();
		stream.size = data.size();
		stream.pos = 0;
		stream.read = NULL;
		stream.close = NULL;
		stream.descriptor.pointer = NULL;

		args.flags = FT_OPEN_STREAM;
		args.stream = &stream;
		err = FT_Open_Face(library, &args, 0, &face);
		if (!err)
		{
			FT_Done_Face(face);
		}
		FT_Done_FreeType(library);
		if (err)
		{
			return FreetypeUtils::getErrorMessage(err);
		}
		return "";
	}

	std::shared_ptr<FreetypeFontDefinition> DataFreetypeFontDefinitionLoader::operator()(std::filesystem::path path)
	{
		auto data = _dataLoader(path);
		if (_skipInvalid && !checkFontData(data).empty())
		{
			return nullptr;
		}
		return std::make_shared<FreetypeFontDefinition>(std::move(data));
	}

	FreetypeFontLoaderImpl::FreetypeFontLoaderImpl(bx::AllocatorI& alloc)
		: _library(nullptr)
		, _alloc(alloc)
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

	std::shared_ptr<IFont> FreetypeFontLoaderImpl::create(const std::shared_ptr<FreetypeFontDefinition>& def)
	{
		if (!def)
		{
			return nullptr;
		}
		if (!_library)
		{
			throw std::runtime_error("freetype library not initialized");
		}
		FT_Face face = nullptr;

		auto err = FT_New_Memory_Face(_library, (const FT_Byte*)def->data.ptr(), def->data.size(), 0, &face);
		FreetypeUtils::checkError(err);
		err = FT_Set_Pixel_Sizes(face, def->fontSize.x, def->fontSize.y);
		FreetypeUtils::checkError(err);
		err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		FreetypeUtils::checkError(err);

		return std::make_shared<FreetypeFont>(def, face, _library, _alloc);
	}

	FreetypeFontLoader::FreetypeFontLoader(IFreetypeFontDefinitionLoader& defLoader, bx::AllocatorI& alloc)
		: BasicFromDefinitionLoader(defLoader)
		, _impl(std::make_unique<FreetypeFontLoaderImpl>(alloc))
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

	std::shared_ptr<IFont> FreetypeFontLoader::create(const std::shared_ptr<FreetypeFontDefinition>& def)
	{
		return _impl->create(def);
	}

	FreetypeFont::FreetypeFont(const std::shared_ptr<Definition>& def, FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept
		: _def(def)
		, _face(face)
		, _library(library)
		, _alloc(alloc)
	{
	}

	FreetypeFont::~FreetypeFont() noexcept
	{
		FT_Done_Face(_face);
	}

	std::optional<Glyph> FreetypeFont::getGlyph(const UtfChar& chr) const noexcept
	{
		auto itr = _glyphs.find(chr);
		if (itr == _glyphs.end())
		{
			return std::nullopt;
		}
		return itr->second;
	}

	float FreetypeFont::getLineSize() const noexcept
	{
		return _face->size->metrics.height >> 6;
	}

	FT_Face FreetypeFont::getFace() const  noexcept
	{
		return _face;
	}

	std::shared_ptr<Texture> FreetypeFont::getTexture() const
	{
		return _texture;
	}

	void FreetypeFont::update(const std::unordered_set<UtfChar>& chars)
	{
		if (_renderedChars == chars || chars.empty())
		{
			return;
		}
		FreetypeFontAtlasGenerator generator(_face, _library, _alloc);
		generator.setImageFormat(bimg::TextureFormat::RGBA8);
		auto atlas = generator(UtfVector(chars.begin(), chars.end()));

		if (!_texture || _texture->getSize() != atlas.image.getSize())
		{
			_texture = std::make_shared<Texture>(atlas.image.getTextureConfig());
		}
		_texture->update(atlas.image.getData());

		_glyphs = atlas.glyphs;
		_renderedChars = chars;
	}

	FreetypeFontAtlasGenerator::FreetypeFontAtlasGenerator(FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept
		: _face(face)
		, _library(library)
		, _alloc(alloc)
		, _size(1024)
		, _renderMode(FT_RENDER_MODE_NORMAL)
		, _imageFormat(bimg::TextureFormat::A8)
	{
	}

	FreetypeFontAtlasGenerator& FreetypeFontAtlasGenerator::setSize(const glm::uvec2& size) noexcept
	{
		_size = size;
		return *this;
	}

	FreetypeFontAtlasGenerator& FreetypeFontAtlasGenerator::setImageFormat(bimg::TextureFormat::Enum format) noexcept
	{
		_imageFormat = format;
		return *this;
	}

	FreetypeFontAtlasGenerator& FreetypeFontAtlasGenerator::setRenderMode(FT_Render_Mode mode) noexcept
	{
		_renderMode = mode;
		return *this;
	}

	glm::uvec2 FreetypeFontAtlasGenerator::calcSpace(const UtfVector& chars) noexcept
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

	std::map<UtfChar, FT_UInt> FreetypeFontAtlasGenerator::getIndices(const UtfVector& chars) const
	{
		std::map<UtfChar, FT_UInt> indices;
		if (chars.empty())
		{
			FT_UInt idx;
			auto code = FT_Get_First_Char(_face, &idx);
			while (idx != 0)
			{
				auto chr = UtfChar(code);
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
				auto idx = FT_Get_Char_Index(_face, chr.code);
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
		err = FT_Render_Glyph(_face->glyph, _renderMode);
		FreetypeUtils::checkError(err);
		return _face->glyph->bitmap;
	}

	FontAtlas FreetypeFontAtlasGenerator::operator()(const UtfVector& chars)
	{
		glm::uvec2 pos(0);
		FT_UInt fontHeight = _face->size->metrics.y_ppem;
		FontAtlas atlas(Image(_size, _alloc, _imageFormat));
		auto indices = getIndices(chars);

		auto bytesPerPixel = atlas.image.getTextureInfo().bitsPerPixel / 8;
		for(auto& [chr, idx] : indices)
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

			auto& metrics = _face->glyph->metrics;
			// https://freetype.org/freetype2/docs/glyphs/glyphs-3.html
			Glyph glyph
			{
				.size = glm::vec2(metrics.width >> 6, metrics.height >> 6),
				.texturePosition = pos,
				.offset = glm::vec2(metrics.horiBearingX >> 6, metrics.horiBearingY >> 6),
				.originalSize = glm::uvec2(metrics.horiAdvance >> 6, fontHeight),
			};
			glyph.offset.y -= glyph.size.y;
			atlas.glyphs.emplace(chr, glyph);
			glm::uvec2 bsize(bitmap.width, bitmap.rows);
			DataView bitmapData(bitmap.buffer, bsize.y * bitmap.pitch);
			for (size_t pixelOffset = 0; pixelOffset < bytesPerPixel; pixelOffset++)
			{
				atlas.image.update(pos, bsize, bitmapData, pixelOffset, 1);
			}
			pos.x += glyph.size.x;
		}
		return atlas;
	}

	FontAtlas FreetypeFontAtlasGenerator::operator()(std::string_view str)
	{
		UtfVector chars;
		UtfChar::read(str, chars);
		return (*this)(chars);
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

		glm::uvec2 size(0, 48);
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
		generator.setImageFormat(bimg::TextureFormat::RGBA8);
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
			_imagePath /= std::filesystem::path(stem + ".png");
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
			auto encoding = Image::getEncodingForPath(_imagePath);
			_atlas->image.write(encoding, out);
			return;
		}
		auto relImagePath = std::filesystem::relative(_imagePath, _atlasPath.parent_path());
		TextureAtlasDefinition atlasDef
		{
			.imagePath = relImagePath,
			.size = _atlas->image.getSize()
		};
		for (auto& [name, glyph] : _atlas->glyphs)
		{
			atlasDef.elements.emplace_back(TextureAtlasElement{
				.name = name,
				.texturePosition = glyph.texturePosition,
				.size = glyph.size,
				.offset = glyph.offset,
				.originalSize = glyph.originalSize
			});
		}

		pugi::xml_document doc;
		atlasDef.writeTexturePacker(doc);
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