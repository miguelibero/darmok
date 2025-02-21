#include "texture.hpp"

#include <glm/gtx/string_cast.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/portable_binary.hpp>
#include <darmok/glm_serialize.hpp>

namespace darmok
{
	bool TextureDefinition::empty() const noexcept
	{
		return data.empty();
	}

	void TextureDefinition::loadImage(const Image& img) noexcept
	{
		data = img.getData();
		config = img.getTextureConfig();
	}
	 
	void TextureConfig::read(const nlohmann::json& json)
	{
		if (json.contains("size"))
		{
			size = json["size"];
		}
		if (json.contains("format"))
		{
			format = json["format"];
		}
		if (json.contains("type"))
		{
			type = json["type"];
		}
		if (json.contains("depth"))
		{
			depth = json["depth"];
		}
		if (json.contains("mips"))
		{
			mips = json["mips"];
		}
		if (json.contains("layers"))
		{
			layers = json["layers"];
		}
	}

	const TextureConfig& TextureConfig::getEmpty() noexcept
	{
		static TextureConfig empty{
			glm::uvec2(0),
			bgfx::TextureFormat::Unknown,
			TextureType::Unknown,
			0, false, 0
		};
		return empty;
	}

	std::string TextureConfig::toString() const noexcept
	{
		return "TextureConfig(" + glm::to_string(size) + ")";
	}

	bgfx::TextureInfo TextureConfig::getInfo() const noexcept
	{
		bgfx::TextureInfo info;
		auto cubeMap = type == TextureType::CubeMap;
		bgfx::calcTextureSize(info, size.x, size.y, depth, cubeMap, mips, layers, format);
		return info;
	}

	ImageTextureDefinitionLoader::ImageTextureDefinitionLoader(IImageLoader& imgLoader) noexcept
		: _imgLoader(imgLoader)
		, _loadFlags(defaultTextureLoadFlags)
	{
	}

	bool ImageTextureDefinitionLoader::supports(const std::filesystem::path& path) const noexcept
	{
		return Image::getEncodingForPath(path) != ImageEncoding::Count;
	}

	ImageTextureDefinitionLoader& ImageTextureDefinitionLoader::setLoadFlags(uint64_t flags) noexcept
	{
		_loadFlags = flags;
		return *this;
	}

	std::shared_ptr<TextureDefinition> ImageTextureDefinitionLoader::operator()(std::filesystem::path path)
	{
		if (!supports(path))
		{
			return nullptr;
		}
		if (auto img = _imgLoader(path))
		{
			auto def = std::make_shared<TextureDefinition>();
			def->flags = _loadFlags;
			def->loadImage(*img);
			return def;
		}
		return nullptr;
	}

	const std::array<std::string, toUnderlying(bgfx::TextureFormat::Count)> Texture::_formatNames =
	{
		"BC1",
		"BC2",
		"BC3",
		"BC4",
		"BC5",
		"BC6H",
		"BC7",
		"ETC1",
		"ETC2",
		"ETC2A",
		"ETC2A1",
		"PTC12",
		"PTC14",
		"PTC12A",
		"PTC14A",
		"PTC22",
		"PTC24",
		"ATC",
		"ATCE",
		"ATCI",
		"ASTC4x4",
		"ASTC5x4",
		"ASTC5x5",
		"ASTC6x5",
		"ASTC6x6",
		"ASTC8x5",
		"ASTC8x6",
		"ASTC8x8",
		"ASTC10x5",
		"ASTC10x6",
		"ASTC10x8",
		"ASTC10x10",
		"ASTC12x10",
		"ASTC12x12",
		"Unknown",
		"R1",
		"A8",
		"R8",
		"R8I",
		"R8U",
		"R8S",
		"R16",
		"R16I",
		"R16U",
		"R16F",
		"R16S",
		"R32I",
		"R32U",
		"R32F",
		"RG8",
		"RG8I",
		"RG8U",
		"RG8S",
		"RG16",
		"RG16I",
		"RG16U",
		"RG16F",
		"RG16S",
		"RG32I",
		"RG32U",
		"RG32F",
		"RGB8",
		"RGB8I",
		"RGB8U",
		"RGB8S",
		"RGB9E5F",
		"BGRA8",
		"RGBA8",
		"RGBA8I",
		"RGBA8U",
		"RGBA8S",
		"RGBA16",
		"RGBA16I",
		"RGBA16U",
		"RGBA16F",
		"RGBA16S",
		"RGBA32I",
		"RGBA32U",
		"RGBA32F",
		"B5G6R5",
		"R5G6B5",
		"BGRA4",
		"RGBA4",
		"BGR5A1",
		"RGB5A1",
		"RGB10A2",
		"RG11B10F",
		"UnknownDepth",
		"D16",
		"D24",
		"D24S8",
		"D32",
		"D16F",
		"D24F",
		"D32F",
		"D0S8"
	};

	const std::array<std::string, toUnderlying(TextureType::Count)> Texture::_typeNames =
	{
		"Unknown",
		"CubeMap",
		"Texture2D",
		"Texture3D"
	};

	const Texture::FlagMap Texture::_textureFlags =
	{
		{ "NONE", BGFX_TEXTURE_NONE },
		{ "MSAA_SAMPLE", BGFX_TEXTURE_MSAA_SAMPLE },
		{ "RT", BGFX_TEXTURE_RT },
		{ "COMPUTE_WRITE", BGFX_TEXTURE_COMPUTE_WRITE },
		{ "SRG", BGFX_TEXTURE_SRGB },
		{ "BLIT_DST", BGFX_TEXTURE_BLIT_DST },
		{ "READ_BACK", BGFX_TEXTURE_READ_BACK },
		{ "RT_MSAA_X2", BGFX_TEXTURE_RT_MSAA_X2 },
		{ "RT_MSAA_X4", BGFX_TEXTURE_RT_MSAA_X4 },
		{ "RT_MSAA_X8", BGFX_TEXTURE_RT_MSAA_X8 },
		{ "RT_MSAA_X16", BGFX_TEXTURE_RT_MSAA_X16 },
		{ "RT_MSAA_SHIFT", BGFX_TEXTURE_RT_MSAA_SHIFT },
		{ "RT_MSAA_MASK", BGFX_TEXTURE_RT_MSAA_MASK },
		{ "RT_WRITE_ONLY", BGFX_TEXTURE_RT_WRITE_ONLY },
		{ "RT_SHIFT", BGFX_TEXTURE_RT_SHIFT },
		{ "RT_MASK", BGFX_TEXTURE_RT_MASK }
	};

	const Texture::FlagMap Texture::_samplerFlags
	{
		{ "NONE", BGFX_SAMPLER_NONE },
		{ "U_MIRROR", BGFX_SAMPLER_U_MIRROR },
		{ "U_CLAMP", BGFX_SAMPLER_U_CLAMP },
		{ "U_BORDER", BGFX_SAMPLER_U_BORDER },
		{ "U_SHIFT", BGFX_SAMPLER_U_SHIFT },
		{ "U_MASK", BGFX_SAMPLER_U_MASK },
		{ "V_MIRROR", BGFX_SAMPLER_V_MIRROR },
		{ "V_CLAMP", BGFX_SAMPLER_V_CLAMP },
		{ "V_BORDER", BGFX_SAMPLER_V_BORDER },
		{ "V_SHIFT", BGFX_SAMPLER_V_SHIFT },
		{ "V_MASK", BGFX_SAMPLER_V_MASK },
		{ "W_MIRROR", BGFX_SAMPLER_W_MIRROR },
		{ "W_CLAMP", BGFX_SAMPLER_W_CLAMP },
		{ "W_BORDER", BGFX_SAMPLER_W_BORDER },
		{ "W_SHIFT", BGFX_SAMPLER_W_SHIFT },
		{ "W_MASK", BGFX_SAMPLER_W_MASK },
		{ "MIN_POINT", BGFX_SAMPLER_MIN_POINT },
		{ "MIN_ANISOTROPIC", BGFX_SAMPLER_MIN_ANISOTROPIC },
		{ "MIN_SHIFT", BGFX_SAMPLER_MIN_SHIFT },
		{ "MIN_MASK", BGFX_SAMPLER_MIN_MASK },
		{ "MAG_POINT", BGFX_SAMPLER_MAG_POINT },
		{ "MAG_ANISOTROPIC", BGFX_SAMPLER_MAG_ANISOTROPIC },
		{ "MAG_SHIFT", BGFX_SAMPLER_MAG_SHIFT },
		{ "MAG_MASK", BGFX_SAMPLER_MAG_MASK },
		{ "MIP_POINT", BGFX_SAMPLER_MIP_POINT },
		{ "MIP_SHIFT", BGFX_SAMPLER_MIP_SHIFT },
		{ "MIP_MASK", BGFX_SAMPLER_MIP_MASK },
		{ "COMPARE_LESS", BGFX_SAMPLER_COMPARE_LESS },
		{ "COMPARE_LEQUAL", BGFX_SAMPLER_COMPARE_LEQUAL },
		{ "COMPARE_EQUAL", BGFX_SAMPLER_COMPARE_EQUAL },
		{ "COMPARE_GEQUAL", BGFX_SAMPLER_COMPARE_GEQUAL },
		{ "COMPARE_GREATER", BGFX_SAMPLER_COMPARE_GREATER },
		{ "COMPARE_NOTEQUAL", BGFX_SAMPLER_COMPARE_NOTEQUAL },
		{ "COMPARE_NEVER", BGFX_SAMPLER_COMPARE_NEVER },
		{ "COMPARE_ALWAYS", BGFX_SAMPLER_COMPARE_ALWAYS },
		{ "COMPARE_SHIFT", BGFX_SAMPLER_COMPARE_SHIFT },
		{ "COMPARE_MASK", BGFX_SAMPLER_COMPARE_MASK },
		{ "BORDER_COLOR_SHIFT", BGFX_SAMPLER_BORDER_COLOR_SHIFT },
		{ "BORDER_COLOR_MASK", BGFX_SAMPLER_BORDER_COLOR_MASK },
		{ "RESERVED_SHIFT", BGFX_SAMPLER_RESERVED_SHIFT },
		{ "RESERVED_MASK", BGFX_SAMPLER_RESERVED_MASK },
		{ "SAMPLE_STENCIL", BGFX_SAMPLER_SAMPLE_STENCIL },
		{ "POINT", BGFX_SAMPLER_POINT },
		{ "UVW_MIRROR", BGFX_SAMPLER_UVW_MIRROR },
		{ "UVW_CLAMP", BGFX_SAMPLER_UVW_CLAMP },
		{ "UVW_BORDER", BGFX_SAMPLER_UVW_BORDER },
		{ "BITS_MASK", BGFX_SAMPLER_BITS_MASK }
	};

	const std::string& Texture::getFormatName(bgfx::TextureFormat::Enum format) noexcept
	{
		return StringUtils::getEnumName(static_cast<size_t>(format), _formatNames);
	}

	std::optional<bgfx::TextureFormat::Enum> Texture::readFormat(std::string_view name) noexcept
	{
		return StringUtils::readEnum<bgfx::TextureFormat::Enum>(name, _formatNames);
	}

	const std::string& Texture::getTypeName(TextureType type) noexcept
	{
		return StringUtils::getEnumName(toUnderlying(type), _typeNames);
	}

	std::optional<TextureType> Texture::readType(std::string_view name) noexcept
	{
		return StringUtils::readEnum<TextureType>(name, _typeNames);
	}

	const Texture::FlagMap& Texture::getTextureFlags() noexcept
	{
		return _textureFlags;
	}

	const std::string& Texture::getTextureFlagName(uint64_t flag) noexcept
	{
		auto itr = std::find_if(_textureFlags.begin(), _textureFlags.end(),
			[flag](auto& elm) { return elm.second == flag; });
		if (itr != _textureFlags.end())
		{
			return itr->first;
		}
		static const std::string empty;
		return empty;
	}

	std::optional<uint64_t> Texture::readTextureFlag(std::string_view name) noexcept
	{
		auto itr = _textureFlags.find(std::string(name));
		if (itr != _textureFlags.end())
		{
			return itr->second;
		}
		return std::nullopt;
	}

	const Texture::FlagMap& Texture::getSamplerFlags() noexcept
	{
		return _samplerFlags;
	}

	const std::string& Texture::getSamplerFlagName(uint64_t flag) noexcept
	{
		auto itr = std::find_if(_samplerFlags.begin(), _samplerFlags.end(),
			[flag](auto& elm) { return elm.second == flag; });
		if (itr != _samplerFlags.end())
		{
			return itr->first;
		}
		static const std::string empty;
		return empty;
	}

	std::optional<uint64_t> Texture::readSamplerFlag(std::string_view name) noexcept
	{
		auto itr = _samplerFlags.find(std::string(name));
		if (itr != _samplerFlags.end())
		{
			return itr->second;
		}
		return std::nullopt;
	}

	uint64_t Texture::getBorderColorFlag(const Color& color) noexcept
	{
		return BGFX_SAMPLER_BORDER_COLOR(Colors::toNumber(color));
	}

	uint64_t Texture::readFlags(const nlohmann::json& json) noexcept
	{
		uint64_t flags = 0;

		auto readFlag = [](auto& elm) -> std::optional<uint64_t>
			{
				if (auto textureFlag = readTextureFlag(elm))
				{
					return textureFlag;
				}
				if (auto samplerFlag = readSamplerFlag(elm))
				{
					return samplerFlag;
				}
				return {};
			};

		if (json.is_array())
		{
			for (auto& elm : json)
			{
				if (auto flag = readFlag(elm))
				{
					flags |= *flag;
				}
			}
		}
		else if (json.is_object())
		{
			for (auto& [key, val] : json.items())
			{
				if (auto flag = readFlag(key))
				{
					if (val)
					{
						flags |= *flag;
					}
					else
					{
						flags &= ~(*flag);
					}
				}
			}
		}
		else if (json.is_string())
		{
			for (auto& word : StringUtils::splitWords(json))
			{
				if (auto flag = readFlag(word))
				{
					flags |= *flag;
				}
			}
		}
		else
		{
			flags = json;
		}

		return flags;
	}


	Texture::Texture(const bgfx::TextureHandle& handle, const Config& cfg) noexcept
		: _handle(handle)
		, _config(cfg)
	{
	}

	Texture::Texture(const Image& img, uint64_t flags) noexcept
		: Texture(img.getData(), img.getTextureConfig(), flags)
	{
	}

	Texture::Texture(const DataView& data, const Config& cfg, uint64_t flags) noexcept
		: _handle{ bgfx::kInvalidHandle }
		, _config(cfg)
	{
		if (_config.type == TextureType::Unknown)
		{
			// TODO: maybe throw here
			return;
		}

		// copying the memory of the image becauyse bgfx needs to maintain the memory for some frames
		// since the texture creation can b e async, and it could happen that the std::shared_ptr<Image>
		// is destroyed before (for example if a texture is created and replaced in the same frame
		const auto mem = data.copyMem();
		auto w = uint16_t(_config.size.x);
		auto h = uint16_t(_config.size.y);

		switch (_config.type)
		{
		case TextureType::CubeMap:
			_handle = bgfx::createTextureCube(w, _config.mips, _config.layers, _config.format, flags, mem);
			break;
		case TextureType::Texture3D:
			_handle = bgfx::createTexture3D(w, h, _config.depth, _config.mips, _config.format, flags, mem);
			break;
		case TextureType::Texture2D:
			_handle = bgfx::createTexture2D(w, h, _config.mips, _config.layers, _config.format, flags, mem);
			break;
		default:
			break;
		}
	}

	Texture::Texture(const Config& cfg, uint64_t flags) noexcept
		: _handle{ bgfx::kInvalidHandle }
		, _config(cfg)
	{
		bgfx::TextureHandle handle{ bgfx::kInvalidHandle };
		switch (cfg.type)
		{
		case TextureType::CubeMap:
			_handle = bgfx::createTextureCube(cfg.size.x, cfg.mips, cfg.layers, cfg.format, flags);
			break;
		case TextureType::Texture2D:
			_handle = bgfx::createTexture2D(cfg.size.x, cfg.size.y, cfg.mips, cfg.layers, cfg.format, flags);
			break;
		case TextureType::Texture3D:
			_handle = bgfx::createTexture3D(cfg.size.x, cfg.size.y, cfg.depth, cfg.mips, cfg.format, flags);
			break;
		default:
			break;
		}
	}

	Texture::Texture(const TextureDefinition& def) noexcept
		: Texture(def.data, def.config, def.flags)
	{
	}

	Texture::Texture(Texture&& other) noexcept
		: _handle(other._handle)
		, _config(other._config)
	{
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = Config::getEmpty();
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		_handle = other._handle;
		_config = other._config;
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = Config::getEmpty();
		return *this;
	}

	Texture::~Texture() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	uint32_t Texture::getStorageSize() const noexcept
	{
		return _config.getInfo().storageSize;
	}

	uint8_t Texture::getMipsCount() const noexcept
	{
		return _config.getInfo().numMips;
	}

	uint8_t Texture::getBitsPerPixel() const noexcept
	{
		return _config.getInfo().bitsPerPixel;
	}

	void Texture::update(const DataView& data, uint8_t mip)
	{
		if (_config.type == TextureType::Texture3D)
		{
			update(data, glm::uvec3(getSize(), getDepth()), glm::uvec3(0), mip);
		}
		else
		{
			update(data, getSize(), glm::uvec2(0), mip);
		}
	}

	void Texture::update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin, uint8_t mip, uint16_t layer, uint8_t side)
	{
		if (_config.type == TextureType::Texture3D)
		{
			throw std::runtime_error("does not work on 3D textures");
		}
		auto memSize = size.x * size.y * getBitsPerPixel() / 8;
		if (memSize == 0)
		{
			return;
		}
		if (data.size() < memSize)
		{
			throw std::runtime_error("data is smaller that expected size");
		}
		if (_config.type == TextureType::Texture2D)
		{
			bgfx::updateTexture2D(_handle, layer, mip,
				origin.x, origin.y, size.x, size.y,
				data.copyMem(0, memSize));
		}
		else
		{
			bgfx::updateTextureCube(_handle, layer, side, mip,
				origin.x, origin.y, size.x, size.y,
				data.copyMem(0, memSize));
		}
	}

	void Texture::update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin, uint8_t mip)
	{
		if (_config.type != TextureType::Texture3D)
		{
			throw std::runtime_error("does only work on 3D textures");
		}
		auto memSize = size.x * size.y * size.z * getBitsPerPixel() / 8;
		if (memSize == 0)
		{
			return;
		}
		if (data.size() < memSize)
		{
			throw std::runtime_error("data is smaller that expected size");
		}
		bgfx::updateTexture3D(_handle, mip,
			origin.x, origin.y, origin.y, size.x, size.y, size.z,
			data.copyMem(0, memSize));

	}

	uint32_t Texture::read(Data& data) noexcept
	{
		auto size = getStorageSize();
		if (data.size() < size)
		{
			data.resize(size);
		}
		return bgfx::readTexture(_handle, data.ptr());
	}

	std::string Texture::toString() const noexcept
	{
		return "Texture(" + std::to_string(_handle.idx)
			+ " " + _config.toString() + ")";
	}

	const bgfx::TextureHandle& Texture::getHandle() const noexcept
	{
		return _handle;
	}

	const glm::uvec2& Texture::getSize() const noexcept
	{
		return _config.size;
	}

	TextureType Texture::getType() const noexcept
	{
		return _config.type;
	}

	bgfx::TextureFormat::Enum Texture::getFormat() const noexcept
	{
		return _config.format;
	}

	uint16_t Texture::getLayerCount() const noexcept
	{
		return _config.layers;
	}

	uint16_t Texture::getDepth() const noexcept
	{
		return _config.depth;
	}

	bool Texture::hasMips() const noexcept
	{
		return _config.mips;
	}

	Texture& Texture::setName(std::string_view name) noexcept
	{
		if (isValid(_handle))
		{
			bgfx::setName(_handle, name.data(), int32_t(name.size()));
		}
		return *this;
	}

	void TextureFileImportConfig::read(const nlohmann::json& json, std::filesystem::path basePath)
	{
		if (json.contains("imageFormat"))
		{
			imageFormat = json["imageFormat"];
		}
		if (json.contains("flags"))
		{
			flags = Texture::readFlags(json["flags"]);
		}
		config.read(json);
	}

	void TextureFileImportConfig::load(const FileTypeImporterInput& input)
	{
		auto basePath = input.path.parent_path();
		auto fileName = input.path.filename();

		if (input.config.is_object())
		{
			read(input.config, input.basePath);
		}
		if (std::filesystem::exists(input.path))
		{
			std::ifstream is(input.path);
			auto fileJson = nlohmann::ordered_json::parse(is);
			read(fileJson, input.path.parent_path());
		}
	}

	bool TextureFileImporterImpl::startImport(const Input& input, bool dry)
	{
		if (input.config.is_null())
		{
			return false;
		}
		_outputPath = input.getOutputPath(".dtx");
		_importConfig.emplace().load(input);
		return true;
	}

	std::vector<std::filesystem::path> TextureFileImporterImpl::getOutputs(const Input& input)
	{
		return { _outputPath };
	}

	TextureFileImporterImpl::Dependencies TextureFileImporterImpl::getDependencies(const Input& input)
	{
		return {};
	}

	void TextureFileImporterImpl::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		TextureDefinition def;
		bimg::TextureFormat::Enum format = bimg::TextureFormat::Count;
		if (_importConfig)
		{
			format = _importConfig->imageFormat;
			def.config = _importConfig->config;
			def.flags = _importConfig->flags;
		}

		Image img(Data::fromFile(input.path), _alloc, format);
		def.loadImage(img);
		CerealUtils::save(def, _outputPath);
	}

	void TextureFileImporterImpl::endImport(const Input& input)
	{
		_importConfig.reset();
		_outputPath.clear();
	}

	const std::string& TextureFileImporterImpl::getName() const noexcept
	{
		static const std::string name = "texture";
		return name;
	}

	TextureFileImporter::TextureFileImporter()
		: _impl(std::make_unique<TextureFileImporterImpl>())
	{
	}

	TextureFileImporter::~TextureFileImporter() noexcept
	{
		// empty on purpose
	}

	bool TextureFileImporter::startImport(const Input& input, bool dry)
	{
		return _impl->startImport(input, dry);
	}

	TextureFileImporter::Outputs TextureFileImporter::getOutputs(const Input& input)
	{
		return _impl->getOutputs(input);
	}

	TextureFileImporter::Dependencies TextureFileImporter::getDependencies(const Input& input)
	{
		return _impl->getDependencies(input);
	}

	void TextureFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		_impl->writeOutput(input, outputIndex, out);
	}

	void TextureFileImporter::endImport(const Input& input)
	{
		_impl->endImport(input);
	}

	const std::string& TextureFileImporter::getName() const noexcept
	{
		return _impl->getName();
	}

}