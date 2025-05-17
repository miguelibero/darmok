#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/data.hpp>
#include <darmok/stream.hpp>
#include <darmok/program.hpp>
#include "text_freetype.hpp"

#include <stdexcept>
#include <pugixml.hpp>

namespace darmok
{
	namespace FreetypeUtils
	{
		const char* getErrorMessage(FT_Error err) noexcept
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

		void checkError(FT_Error err)
		{
			if (err)
			{
				auto msg = getErrorMessage(err);
				throw std::runtime_error(msg);
			}
		}
	};

	FreetypeFontDefinitionLoader::FreetypeFontDefinitionLoader(IDataLoader& dataLoader) noexcept
		: _dataLoader{ dataLoader }
	{
	}

	std::string FreetypeFontDefinitionLoader::checkFontData(const DataView& data) noexcept
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
		return {};
	}

	FreetypeFontDefinitionLoader::Result FreetypeFontDefinitionLoader::operator()(std::filesystem::path path)
	{
		auto dataResult = _dataLoader(path);
		if (!dataResult)
		{
			return unexpected{ dataResult.error() };
		}
		auto& data = dataResult.value();
		auto error = checkFontData(data);
		if (!error.empty())
		{
			return unexpected{ error };
		}
		auto res = std::make_shared<Resource>();
		res->set_data(data.toString());
		return res;
	}

	FreetypeFontLoaderImpl::FreetypeFontLoaderImpl(bx::AllocatorI& alloc)
		: _library{ nullptr }
		, _alloc{ alloc }
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

	FreetypeFontLoaderImpl::Result FreetypeFontLoaderImpl::create(const std::shared_ptr<Definition>& def)
	{
		if (!def)
		{
			return unexpected<std::string>{ "empty definition" };
		}
		if (!_library)
		{
			return unexpected<std::string>{ "freetype library not initialized" };
		}
		FT_Face face = nullptr;
		auto& data = def->data();
		auto err = FT_New_Memory_Face(_library, (const FT_Byte*)data.data(), data.size(), 0, &face);
		FreetypeUtils::checkError(err);
		auto& fontSize = def->font_size();
		err = FT_Set_Pixel_Sizes(face, fontSize.x(), fontSize.y());
		FreetypeUtils::checkError(err);
		err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
		FreetypeUtils::checkError(err);

		return std::make_shared<FreetypeFont>(def, face, _library, _alloc);
	}

	FreetypeFontLoader::FreetypeFontLoader(IFreetypeFontDefinitionLoader& defLoader, bx::AllocatorI& alloc)
		: FromDefinitionLoader(defLoader)
		, _impl{ std::make_unique<FreetypeFontLoaderImpl>(alloc) }
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

	FreetypeFontLoader::Result FreetypeFontLoader::create(const std::shared_ptr<Definition>& def)
	{
		return _impl->create(def);
	}

	FreetypeFont::FreetypeFont(const std::shared_ptr<Definition>& def, FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept
		: _def{ def }
		, _face{ face }
		, _library{ library }
		, _alloc{ alloc }
	{
	}

	FreetypeFont::~FreetypeFont() noexcept
	{
		FT_Done_Face(_face);
	}

	std::optional<IFont::Glyph> FreetypeFont::getGlyph(char32_t chr) const noexcept
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

	expected<void, std::string> FreetypeFont::update(const std::unordered_set<char32_t>& chars)
	{
		if (_renderedChars == chars || chars.empty())
		{
			return {};
		}
		FreetypeFontAtlasGenerator generator{ _face, _library, _alloc };
		generator.setImageFormat(bimg::TextureFormat::RGBA8);
		auto result = generator(std::u32string{ chars.begin(), chars.end() });
		if (!result)
		{
			return unexpected{ result.error() };
		}

		if (!_texture || _texture->getSize() != result->image.getSize())
		{
			_texture = std::make_shared<Texture>(result->image.getTextureConfig());
		}
		auto updateResult = _texture->update(result->image.getData());
		if (!updateResult)
		{
			return unexpected{ updateResult.error() };
		}

		_glyphs.clear();
		for (auto& glyph : result->atlas.elements())
		{
			auto chr = StringUtils::toUtf32Char(glyph.name());			
			_glyphs.emplace(chr, glyph);
		}
		_renderedChars = chars;
		return {};
	}

	FreetypeFontAtlasGenerator::FreetypeFontAtlasGenerator(FT_Face face, FT_Library library, bx::AllocatorI& alloc) noexcept
		: _face{ face }
		, _library{ library }
		, _alloc{ alloc }
		, _size{ 1024 }
		, _renderMode{ FT_RENDER_MODE_NORMAL }
		, _imageFormat{ bimg::TextureFormat::A8 }
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

	glm::uvec2 FreetypeFontAtlasGenerator::calcSpace(std::u32string_view chars) noexcept
	{
		glm::uvec2 pos{ 0 };
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

	std::map<char32_t, FT_UInt> FreetypeFontAtlasGenerator::getIndices(std::u32string_view chars) const
	{
		std::map<char32_t, FT_UInt> indices;
		if (chars.empty())
		{
			FT_UInt idx;
			char32_t chr = FT_Get_First_Char(_face, &idx);
			while (idx != 0)
			{
				if (chr)
				{
					indices.emplace(chr, idx);
				}
				chr = FT_Get_Next_Char(_face, chr, &idx);
			}
		}
		else
		{
			for (auto& chr : chars)
			{
				auto idx = FT_Get_Char_Index(_face, chr);
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

	FreetypeFontAtlasGenerator::Result FreetypeFontAtlasGenerator::operator()(std::u32string_view chars)
	{
		glm::uvec2 pos{ 0 };
		FT_UInt fontHeight = _face->size->metrics.y_ppem;
		ResultData data
		{
			.image = { _size, _alloc, _imageFormat }
		};
		auto indices = getIndices(chars);

		auto bytesPerPixel = data.image.getTextureInfo().bitsPerPixel / 8;
		for(auto& [chr, idx] : indices)
		{
			auto& bitmap = renderBitmap(idx);
			if (pos.x + bitmap.width >= _size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
				if (pos.y >= _size.y)
				{
					return unexpected<Error>{"Texture overflow"};
				}
			}

			auto& metrics = _face->glyph->metrics;
			// https://freetype.org/freetype2/docs/glyphs/glyphs-3.html
			glm::uvec2 glyphSize{ metrics.width >> 6, metrics.height >> 6 };
			glm::vec2 glpyhOffset{ metrics.horiBearingX >> 6, metrics.horiBearingY >> 6 - glyphSize.y };
			glm::uvec2 glyphOriginalSize{ metrics.horiAdvance >> 6, fontHeight };

			glm::uvec2 bsize{ bitmap.width, bitmap.rows };
			DataView bitmapData{ bitmap.buffer, bsize.y * bitmap.pitch };
			for (size_t pixelOffset = 0; pixelOffset < bytesPerPixel; pixelOffset++)
			{
				auto result = data.image.update(pos, bsize, bitmapData, pixelOffset, 1);
				if (!result)
				{
					return unexpected{ result.error() };
				}
			}
			pos.x += glyphSize.x;

			auto& glyph = *data.atlas.add_elements();
			*glyph.mutable_size() = protobuf::convert(glyphSize);
			*glyph.mutable_texture_position() = protobuf::convert(pos);
			*glyph.mutable_offset() = protobuf::convert(glpyhOffset);
			*glyph.mutable_original_size() = protobuf::convert(glyphOriginalSize);
		}
		return data;
	}

	FreetypeFontAtlasGenerator::Result FreetypeFontAtlasGenerator::operator()(std::string_view str)
	{
		return (*this)(StringUtils::toUtf32(str));
	}

	FreetypeFontFileImporterImpl::FreetypeFontFileImporterImpl()
		: _library(nullptr)
		, _face(nullptr)
		, _dataLoader{_alloc}
		, _texDefLoader{_dataLoader}
	{
		auto err = FT_Init_FreeType(&_library);
		FreetypeUtils::checkError(err);
	}

	FreetypeFontFileImporterImpl::~FreetypeFontFileImporterImpl()
	{
		auto err = FT_Done_FreeType(_library);
		FreetypeUtils::checkError(err);
	}

	bool FreetypeFontFileImporterImpl::startImport(const Input& input, bool dry)
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

		auto result = generator(chars);
		if (!result)
		{
			throw std::runtime_error(result.error());
		}

		_atlas = std::move(result->atlas);
		_image = std::move(result->image);

		auto basePath = input.getRelativePath().parent_path();
		auto stem = input.path.stem().string();

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

	void FreetypeFontFileImporterImpl::endImport(const Input& input)
	{
		FT_Done_Face(_face);
		_face = nullptr;
		_atlas.reset();
	}

	std::vector<std::filesystem::path> FreetypeFontFileImporterImpl::getOutputs(const Input& input)
	{
		return { _imagePath, _atlasPath };
	}

	std::ofstream FreetypeFontFileImporterImpl::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
	{
		if (outputIndex == 0)
		{
			return std::ofstream(path, std::ios::binary);
		}
		return std::ofstream(path);
	}

	void FreetypeFontFileImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		if (!_atlas)
		{
			return;
		}
		if (outputIndex == 0)
		{
			auto encoding = Image::getEncodingForPath(_imagePath);
			_image->write(encoding, out);
			return;
		}
		auto relImagePath = std::filesystem::relative(_imagePath, _atlasPath.parent_path());
		
		pugi::xml_document doc;
		TextureAtlasUtils::writeTexturePacker(*_atlas, doc, _alloc, _texDefLoader, relImagePath);
		doc.save(out, PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
	}

	const std::string& FreetypeFontFileImporterImpl::getName() const noexcept
	{
		static const std::string name("font_atlas");
		return name;
	}

	FreetypeFontFileImporter::FreetypeFontFileImporter() noexcept
		: _impl(std::make_unique<FreetypeFontFileImporterImpl>())
	{
	}

	FreetypeFontFileImporter::~FreetypeFontFileImporter() noexcept
	{
		// intentionally left blank
	}

	std::vector<std::filesystem::path> FreetypeFontFileImporter::getOutputs(const Input& input)
	{
		return _impl->getOutputs(input);
	}

	std::ofstream FreetypeFontFileImporter::createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path)
	{
		return _impl->createOutputStream(input, outputIndex, path);
	}

	void FreetypeFontFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		_impl->writeOutput(input, outputIndex, out);
	}

	const std::string& FreetypeFontFileImporter::getName() const noexcept
	{
		return _impl->getName();
	}

	bool FreetypeFontFileImporter::startImport(const Input& input, bool dry)
	{
		return _impl->startImport(input, dry);
	}

	void FreetypeFontFileImporter::endImport(const Input& input)
	{
		_impl->endImport(input);
	}
}