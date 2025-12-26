#include <darmok/text.hpp>
#include <darmok/text_freetype.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/program.hpp>
#include "detail/text_freetype.hpp"

#include <stdexcept>
#include <pugixml.hpp>

namespace darmok
{
	namespace FreetypeUtils
	{
		using Definition = protobuf::FreetypeFont;

		std::optional<std::string> getErrorMessage(FT_Error err) noexcept
		{
			if (!err)
			{
				return std::nullopt;
			}
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
			return "(Unknown error)";
		}

		expected<FT_Face, std::string> createFace(FT_Library library, const Definition& def) noexcept
		{
			FT_Face face = nullptr;
			auto& data = def.data();
			auto err = FT_New_Memory_Face(library, (const FT_Byte*)data.data(), data.size(), 0, &face);
			if (auto msg = getErrorMessage(err))
			{
				return unexpected{ *msg };
			}
			auto& fontSize = def.font_size();
			err = FT_Set_Pixel_Sizes(face, fontSize.x(), fontSize.y());
			if (auto msg = getErrorMessage(err))
			{
				return unexpected{ *msg };
			}
			err = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
			if (auto msg = getErrorMessage(err))
			{
				return unexpected{ *msg };
			}
			return face;
		}

		void updateDefinition(Definition& def, const nlohmann::json& json) noexcept
		{
			glm::uvec2 size{ 0, 48 };
			auto itr = json.find("fontSize");
			if (itr != json.end())
			{
				auto& sizeConfig = *itr;
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
			*def.mutable_font_size() = convert<protobuf::Uvec2>(size);
		}

	};

	FreetypeFontDefinitionLoader::FreetypeFontDefinitionLoader(IDataLoader& dataLoader) noexcept
		: _dataLoader{ dataLoader }
	{
	}

	expected<void, std::string> FreetypeFontDefinitionLoader::checkFontData(const DataView& data) noexcept
	{
		FT_Library library;
		auto err = FT_Init_FreeType(&library);
		if (auto msg = FreetypeUtils::getErrorMessage(err))
		{
			return unexpected{ *msg };
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
		if (auto msg = FreetypeUtils::getErrorMessage(err))
		{
			return unexpected{ *msg };
		}
		return {};
	}

	FreetypeFontDefinitionLoader::Result FreetypeFontDefinitionLoader::operator()(std::filesystem::path path) noexcept
	{
		auto dataResult = _dataLoader(path);
		if (!dataResult)
		{
			return unexpected{ dataResult.error() };
		}
		auto& data = dataResult.value();
		auto checkResult = checkFontData(data);
		if (!checkResult)
		{
			return unexpected{ checkResult.error() };
		}
		auto def = std::make_shared<Resource>();
		def->set_data(data.toString());
		def->mutable_font_size()->set_y(48);

		auto configResult = _dataLoader(path.replace_extension(".json"));
		if (configResult)
		{
			DataInputStream input{ configResult.value() };
			auto jsonResult = StreamUtils::parseJson(std::move(input));
			if (!jsonResult)
			{
				return unexpected{ jsonResult.error() };
			}
			FreetypeUtils::updateDefinition(*def, *jsonResult);
		}

		return def;
	}

	FreetypeFontLoaderImpl::FreetypeFontLoaderImpl(bx::AllocatorI& alloc) noexcept
		: _library{ nullptr }
		, _alloc{ alloc }
	{
	}

	FreetypeFontLoaderImpl::~FreetypeFontLoaderImpl() noexcept
	{
		auto result = shutdown();
	}

	expected<void, std::string> FreetypeFontLoaderImpl::init(App& app) noexcept
	{
		auto result = shutdown();
		if (!result)
		{
			return result;
		}
		auto err = FT_Init_FreeType(&_library);
		if (auto msg = FreetypeUtils::getErrorMessage(err))
		{
			return unexpected{ std::move(*msg) };
		}
		return {};
	}

	expected<void, std::string> FreetypeFontLoaderImpl::shutdown() noexcept
	{
		if (_library)
		{
			auto err = FT_Done_FreeType(_library);
			_library = nullptr;
			if (auto msg = FreetypeUtils::getErrorMessage(err))
			{
				return unexpected{ std::move(*msg) };
			}
		}
		return {};
	}

	FreetypeFontLoaderImpl::Result FreetypeFontLoaderImpl::create(std::shared_ptr<Definition> def) noexcept
	{
		if (!def)
		{
			return unexpected<std::string>{ "empty definition" };
		}

		auto faceResult = FreetypeUtils::createFace(_library, *def);
		if (!faceResult)
		{
			return unexpected{ faceResult.error() };
		}
		return std::make_shared<FreetypeFont>(def, faceResult.value(), _library, _alloc);
	}

	FreetypeFontLoader::FreetypeFontLoader(IFreetypeFontDefinitionLoader& defLoader, bx::AllocatorI& alloc) noexcept
		: FromDefinitionLoader(defLoader)
		, _impl{ std::make_unique<FreetypeFontLoaderImpl>(alloc) }
	{
	}

	FreetypeFontLoader::~FreetypeFontLoader() noexcept = default;

	expected<void, std::string> FreetypeFontLoader::init(App& app) noexcept
	{
		return _impl->init(app);
	}

	expected<void, std::string> FreetypeFontLoader::shutdown() noexcept
	{
		return _impl->shutdown();
	}

	FreetypeFontLoader::Result FreetypeFontLoader::create(std::shared_ptr<Definition> def) noexcept
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

	std::shared_ptr<Texture> FreetypeFont::getTexture() const noexcept
	{
		return _texture;
	}

	expected<void, std::string> FreetypeFont::update(const std::unordered_set<char32_t>& chars) noexcept
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
			return unexpected{ std::move(result).error() };
		}

		if (!_texture || _texture->getSize() != result->image.getSize())
		{
			auto texResult = Texture::load(result->image.getTextureConfig());
			if(!texResult)
			{
				return unexpected{ std::move(texResult).error() };
			}
			_texture = std::make_shared<Texture>(std::move(texResult).value());
		}
		auto updateResult = _texture->update(result->image.getData());
		if (!updateResult)
		{
			return unexpected{ std::move(updateResult).error() };
		}

		_glyphs.clear();
		for (auto& glyph : result->atlas.elements())
		{
			auto chrResult = StringUtils::toUtf32Char(glyph.name());
			if (!chrResult)
			{
				return unexpected{ std::move(chrResult).error() };
			}
			_glyphs.emplace(chrResult.value(), glyph);
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
			auto bitmapResult = renderBitmap(idx);
			if (!bitmapResult)
			{
				continue;
			}
			auto& bitmap = bitmapResult.value().get();
			if (pos.x + bitmap.width >= _size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
			}
			pos.x += bitmap.width;
		}
		return pos;
	}

	std::map<char32_t, FT_UInt> FreetypeFontAtlasGenerator::getIndices(std::u32string_view chars) const noexcept
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

	expected<std::reference_wrapper<const FT_Bitmap>, std::string> FreetypeFontAtlasGenerator::renderBitmap(FT_UInt index) noexcept
	{
		auto err = FT_Load_Glyph(_face, index, FT_LOAD_DEFAULT);
		if (auto msg = FreetypeUtils::getErrorMessage(err))
		{
			return unexpected{ std::move(*msg) };
		}
		err = FT_Render_Glyph(_face->glyph, _renderMode);
		if (auto msg = FreetypeUtils::getErrorMessage(err))
		{
			return unexpected{ std::move(*msg) };
		}
		return std::ref(_face->glyph->bitmap);
	}

	FreetypeFontAtlasGenerator::Result FreetypeFontAtlasGenerator::operator()(std::u32string_view chars) noexcept
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
			auto chrResult = StringUtils::toUtf8(chr);
			if (!chrResult)
			{
				return unexpected{ std::move(chrResult).error() };
			}
			auto bitmapResult = renderBitmap(idx);
			if (!bitmapResult)
			{
				return unexpected{ std::move(bitmapResult).error() };
			}
			auto& bitmap = bitmapResult.value().get();
			if (pos.x + bitmap.width >= _size.x)
			{
				pos.x = 0;
				pos.y += fontHeight;
				if (pos.y >= _size.y)
				{
					return unexpected<std::string>{"Texture overflow"};
				}
			}

			auto& metrics = _face->glyph->metrics;
			// https://freetype.org/freetype2/docs/glyphs/glyphs-3.html
			glm::uvec2 glyphSize{ metrics.width >> 6, metrics.height >> 6 };
			glm::vec2 glpyhOffset{ metrics.horiBearingX >> 6, metrics.horiBearingY >> 6 };
			glm::uvec2 glyphOriginalSize{ metrics.horiAdvance >> 6, fontHeight };
			glpyhOffset.y -= glyphSize.y;

			glm::uvec2 bsize{ bitmap.width, bitmap.rows };
			DataView bitmapData{ bitmap.buffer, bsize.y * bitmap.pitch };
			for (size_t pixelOffset = 0; pixelOffset < bytesPerPixel; pixelOffset++)
			{
				auto updateResult = data.image.update(pos, bsize, bitmapData, pixelOffset, 1);
				if (!updateResult)
				{
					return unexpected{ std::move(updateResult).error() };
				}
			}

			auto& glyph = *data.atlas.add_elements();
			glyph.set_name(chrResult.value());
			*glyph.mutable_size() = convert<protobuf::Uvec2>(glyphSize);
			*glyph.mutable_texture_position() = convert<protobuf::Uvec2>(pos);
			*glyph.mutable_offset() = convert<protobuf::Vec2>(glpyhOffset);
			*glyph.mutable_original_size() = convert<protobuf::Uvec2>(glyphOriginalSize);

			pos.x += glyphSize.x;
		}
		return data;
	}

	FreetypeFontAtlasGenerator::Result FreetypeFontAtlasGenerator::operator()(std::string_view str) noexcept
	{
		auto result = StringUtils::toUtf32(str);
		if (!result)
		{
			return unexpected{ std::move(result).error() };
		}
		return (*this)(result.value());
	}

	FreetypeFontFileImporterImpl::FreetypeFontFileImporterImpl() noexcept
		: _library{ nullptr }
		, _face{ nullptr }
	{
	}

	expected<void, std::string> FreetypeFontFileImporterImpl::init(OptionalRef<std::ostream> lo) noexcept
	{
		auto err = FT_Init_FreeType(&_library);
		if (auto msg = FreetypeUtils::getErrorMessage(err))
		{
			return unexpected{ std::move(*msg) };
		}
		return {};
	}

	expected<void, std::string> FreetypeFontFileImporterImpl::shutdown() noexcept
	{
		if (_library)
		{
			auto err = FT_Done_FreeType(_library);
			if (auto msg = FreetypeUtils::getErrorMessage(err))
			{
				return unexpected{ std::move(*msg) };
			}
			_library = nullptr;
		}
		return {};
	}

	FreetypeFontFileImporterImpl::~FreetypeFontFileImporterImpl() noexcept
	{
		auto result = shutdown();
		if(!result)
		{
			StreamUtils::log("FreetypeFontFileImporterImpl destructor: " + result.error(), true);
		}
	}

	expected<FreetypeFontFileImporterImpl::Effect, std::string> FreetypeFontFileImporterImpl::prepare(const Input& input) noexcept
	{
		Effect effect;

		if (input.config.is_null())
		{
			return effect;
		}

		auto fileResult = Data::fromFile(input.path);
		if (!fileResult)
		{
			return unexpected{ std::move(fileResult).error() };
		}
		if (!_library)
		{
			auto initResult = init();
			if(!initResult)
			{
				return unexpected{ std::move(initResult).error() };
			}
		}

		protobuf::FreetypeFont def;
		def.set_data(fileResult.value().toString());
		FreetypeUtils::updateDefinition(def, input.config);
		auto faceResult = FreetypeUtils::createFace(_library, def);
		if (!faceResult)
		{
			return unexpected{ std::move(faceResult).error() };
		}
		_face = faceResult.value();
		FreetypeFontAtlasGenerator generator{ _face, _library, _alloc };
		generator.setImageFormat(bimg::TextureFormat::RGBA8);

		std::string chars;
		auto itr = input.config.find("characters");
		if (itr != input.config.end())
		{
			chars = *itr;
		}

		auto result = generator(chars);
		if (!result)
		{
			return unexpected{ std::move(result).error() };
		}

		_atlas = std::move(result->atlas);
		_image = std::move(result->image);

		auto basePath = input.getRelativePath().parent_path();
		auto stem = input.path.stem().string();

		_imagePath = basePath;
		_atlasPath = basePath;

		itr = input.config.find("imagePath");
		if (itr != input.config.end())
		{
			_imagePath /= *itr;
		}
		else
		{
			_imagePath /= std::filesystem::path{ stem + ".png" };
		}
		itr = input.config.find("atlasPath");
		if (itr != input.config.end())
		{
			_atlasPath /= *itr;
		}
		else
		{
			_atlasPath /= std::filesystem::path{ stem + ".xml" };
		}

		effect.outputs.emplace_back(_imagePath, true);
		effect.outputs.emplace_back(_atlasPath, false);

		return effect;
	}

	expected<void, std::string> FreetypeFontFileImporterImpl::operator()(const Input& input, Config& config) noexcept
	{
		if (!_atlas)
		{
			return {};
		}
		if (auto& out = config.outputStreams[0])
		{
			auto encoding = Image::getEncodingForPath(_imagePath);
			auto imgWriteResult = _image->write(encoding, *out);
			if (!imgWriteResult)
			{
				return unexpected{ std::move(imgWriteResult).error() };
			}
		}
		if (auto& out = config.outputStreams[1])
		{
			auto baseAtlasPath = _atlasPath.parent_path();
			auto imgPath = std::filesystem::relative(_imagePath, baseAtlasPath);
			_atlas->set_texture_path(imgPath.string());

			pugi::xml_document doc;

			auto atlasWriteResult = TextureAtlasUtils::writeTexturePacker(*_atlas, doc);
			if (!atlasWriteResult)
			{
				return unexpected{ std::move(atlasWriteResult).error() };
			}
			doc.save(*out, PUGIXML_TEXT("  "), pugi::format_default, pugi::encoding_utf8);
		}

		FT_Done_Face(_face);
		_face = nullptr;
		_atlas.reset();

		return {};
	}

	const std::string& FreetypeFontFileImporterImpl::getName() const noexcept
	{
		static const std::string name{ "font_atlas" };
		return name;
	}

	FreetypeFontFileImporter::FreetypeFontFileImporter() noexcept
		: _impl{ std::make_unique<FreetypeFontFileImporterImpl>() }
	{
	}

	expected<void, std::string> FreetypeFontFileImporter::init(OptionalRef<std::ostream> log) noexcept
	{
		return _impl->init(log);
	}

	expected<void, std::string> FreetypeFontFileImporter::shutdown() noexcept
	{
		return _impl->shutdown();
	}

	FreetypeFontFileImporter::~FreetypeFontFileImporter() noexcept = default;

	expected<FreetypeFontFileImporter::Effect, std::string> FreetypeFontFileImporter::prepare(const Input& input) noexcept
	{
		return _impl->prepare(input);
	}

	expected<void, std::string> FreetypeFontFileImporter::operator()(const Input& input, Config& config) noexcept
	{
		return (*_impl)(input, config);
	}

	const std::string& FreetypeFontFileImporter::getName() const noexcept
	{
		return _impl->getName();
	}
}