#include <darmok/texture.hpp>

#include <glm/gtx/string_cast.hpp>
#include <darmok/glm_serialize.hpp>
#include <magic_enum/magic_enum.hpp>

namespace darmok
{
	void TextureUtils::loadImage(protobuf::Texture& def, const Image& img) noexcept
	{
		def.set_data(img.getData().toString());
		*def.mutable_config() = img.getTextureConfig();
	}
	 
	bgfx::TextureInfo TextureUtils::getInfo(const protobuf::TextureConfig& config) noexcept
	{
		bgfx::TextureInfo info;
		auto cubeMap = config.type() == protobuf::TextureType::CubeMap;
		auto format = bgfx::TextureFormat::Enum(config.format());
		bgfx::calcTextureSize(info, config.size().x(), config.size().y(),
			config.depth(), cubeMap, config.mips(), config.layers(), format);
		return info;
	}

	const protobuf::TextureConfig& TextureUtils::getEmptyConfig() noexcept
	{
		static const protobuf::TextureConfig config;
		return config;
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

	std::shared_ptr<Texture::Definition> ImageTextureDefinitionLoader::operator()(std::filesystem::path path)
	{
		if (!supports(path))
		{
			return nullptr;
		}
		if (auto img = _imgLoader(path))
		{
			auto def = std::make_shared<Texture::Definition>();
			def->set_flags(_loadFlags);
			TextureUtils::loadImage(*def, *img);
			return def;
		}
		return nullptr;
	}

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

	std::string_view Texture::getFormatName(bgfx::TextureFormat::Enum format) noexcept
	{
		return magic_enum::enum_name(format);
	}

	std::optional<bgfx::TextureFormat::Enum> Texture::readFormat(std::string_view name) noexcept
	{
		return magic_enum::enum_cast<bgfx::TextureFormat::Enum>(name);
	}

	std::string_view Texture::getTypeName(TextureType::Enum type) noexcept
	{
		return magic_enum::enum_name(type);
	}

	std::optional<Texture::TextureType::Enum> Texture::readType(std::string_view name) noexcept
	{
		return magic_enum::enum_cast<TextureType::Enum>(name);
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
		if (getType() == TextureType::Unknown)
		{
			// TODO: maybe throw here
			return;
		}

		// copying the memory of the image becauyse bgfx needs to maintain the memory for some frames
		// since the texture creation can b e async, and it could happen that the std::shared_ptr<Image>
		// is destroyed before (for example if a texture is created and replaced in the same frame
		const auto mem = data.copyMem();
		auto w = uint16_t(_config.size().x());
		auto h = uint16_t(_config.size().y());
		auto format = bgfx::TextureFormat::Enum(_config.format());
		switch (getType())
		{
		case TextureType::CubeMap:
			_handle = bgfx::createTextureCube(w, _config.mips(), _config.layers(), format, flags, mem);
			break;
		case TextureType::Texture3D:
			_handle = bgfx::createTexture3D(w, h, _config.depth(), _config.mips(), format, flags, mem);
			break;
		case TextureType::Texture2D:
			_handle = bgfx::createTexture2D(w, h, _config.mips(), _config.layers(), format, flags, mem);
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
		auto format = bgfx::TextureFormat::Enum(cfg.format());
		switch (getType())
		{
		case TextureType::CubeMap:
			_handle = bgfx::createTextureCube(cfg.size().x(), cfg.mips(), cfg.layers(), format, flags);
			break;
		case TextureType::Texture2D:
			_handle = bgfx::createTexture2D(cfg.size().x(), cfg.size().y(), cfg.mips(), cfg.layers(), format, flags);
			break;
		case TextureType::Texture3D:
			_handle = bgfx::createTexture3D(cfg.size().x(), cfg.size().y(), cfg.depth(), cfg.mips(), format, flags);
			break;
		default:
			break;
		}
	}

	Texture::Texture(const Definition& def) noexcept
		: Texture(DataView{ def.data() }, def.config(), def.flags())
	{
	}

	Texture::Texture(Texture&& other) noexcept
		: _handle(other._handle)
		, _config(other._config)
	{
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = TextureUtils::getEmptyConfig();
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		_handle = other._handle;
		_config = other._config;
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = TextureUtils::getEmptyConfig();
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
		return TextureUtils::getInfo(_config).storageSize;
	}

	uint8_t Texture::getMipsCount() const noexcept
	{
		return TextureUtils::getInfo(_config).numMips;
	}

	uint8_t Texture::getBitsPerPixel() const noexcept
	{
		return TextureUtils::getInfo(_config).bitsPerPixel;
	}

	void Texture::update(const DataView& data, uint8_t mip)
	{
		if (getType() == TextureType::Texture3D)
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
		if (getType() == TextureType::Texture3D)
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
		if (getType() == TextureType::Texture2D)
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
		if (getType() != TextureType::Texture3D)
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
			+ " " + _config.DebugString() + ")";
	}

	const bgfx::TextureHandle& Texture::getHandle() const noexcept
	{
		return _handle;
	}

	glm::uvec2 Texture::getSize() const noexcept
	{
		return GlmSerializationUtils::convert(_config.size());
	}

	Texture::TextureType::Enum Texture::getType() const noexcept
	{
		return _config.type();
	}

	bgfx::TextureFormat::Enum Texture::getFormat() const noexcept
	{
		return bgfx::TextureFormat::Enum(_config.format());
	}

	uint16_t Texture::getLayerCount() const noexcept
	{
		return _config.layers();
	}

	uint16_t Texture::getDepth() const noexcept
	{
		return _config.depth();
	}

	bool Texture::hasMips() const noexcept
	{
		return _config.mips();
	}

	Texture& Texture::setName(std::string_view name) noexcept
	{
		if (isValid(_handle))
		{
			bgfx::setName(_handle, name.data(), int32_t(name.size()));
		}
		return *this;
	}

	TextureFileImporter::TextureFileImporter()
		: _dataLoader(_alloc)
		, _imgLoader(_dataLoader, _alloc)
		, _defLoader(_imgLoader)
		, ProtobufFileImporter<ImageTextureDefinitionLoader>(_defLoader, "texture")
	{
	}

	void TextureFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		auto def = _defLoader(input.path);
		auto& config = *def->mutable_config();
		ProtobufUtils::read(config, input.dirConfig);
		ProtobufUtils::read(config, input.config);
		ProtobufFileImporter::writeOutput(*def, out);
	}
}