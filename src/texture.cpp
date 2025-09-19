#include <darmok/texture.hpp>
#include <darmok/glm_serialize.hpp>

#include <glm/gtx/string_cast.hpp>
#include <magic_enum/magic_enum.hpp>

namespace darmok
{
	ConstTextureSourceWrapper::ConstTextureSourceWrapper(const Source& src) noexcept
		: _src{ src }
	{
	}

	expected<Image, std::string> ConstTextureSourceWrapper::createImage(bx::AllocatorI& alloc, bimg::TextureFormat::Enum format) noexcept
	{
		try
		{
			return Image{ DataView{ _src.data() }, alloc, format };
		}
		catch (const std::exception& ex)
		{
			return unexpected<std::string>{ ex.what() };
		}
	}

	TextureSourceWrapper::TextureSourceWrapper(Source& src) noexcept
		: ConstTextureSourceWrapper( src )
		, _src{ src }
	{
	}

	expected<void, std::string> TextureSourceWrapper::loadData(DataView data, ImageEncoding encoding) noexcept
	{
		_src.set_data(data.toString());
		_src.set_encoding(static_cast<Source::Encoding>(encoding));
		return {};
	}

	ConstTextureDefinitionWrapper::ConstTextureDefinitionWrapper(const Definition& def) noexcept
		: _def{ def }
	{
	}

	expected<Image, std::string> ConstTextureDefinitionWrapper::createImage(bx::AllocatorI& alloc) noexcept
	{
		auto size = protobuf::convert(_def.config().size());
		Image img{ size, alloc, static_cast<bimg::TextureFormat::Enum>(_def.config().format()) };
		auto result = img.setData(DataView{ _def.data() });
		if (!result)
		{
			return unexpected{ result.error() };
		}
		return img;
	}

	expected<void, std::string> ConstTextureDefinitionWrapper::writeImage(bx::AllocatorI& alloc, const std::filesystem::path& path) noexcept
	{
		auto result = createImage(alloc);
		if (!result)
		{
			return unexpected{ result.error() };
		}
		auto& img = result.value();
		auto encoding = Image::getEncodingForPath(path);
		std::ofstream out{ path, std::ios::binary };
		return img.write(encoding, out);
	}

	TextureDefinitionWrapper::TextureDefinitionWrapper(Definition& def) noexcept
		: ConstTextureDefinitionWrapper{ def }
		, _def { def }
	{
	}

	expected<void, std::string> TextureDefinitionWrapper::loadSource(const protobuf::TextureSource& src, bx::AllocatorI& alloc) noexcept
	{
		auto createResult = ConstTextureSourceWrapper{ src }.createImage(alloc);
		if (!createResult)
		{
			return unexpected{ createResult.error() };
		}
		auto loadResult = loadImage(createResult.value());
		if(loadResult && src.mips())
		{
			_def.mutable_config()->set_mips(true);
		}
		return loadResult;
	}

	expected<void, std::string> TextureDefinitionWrapper::loadImage(const Image& img) noexcept
	{
		_def.set_data(img.getData().toString());
		*_def.mutable_config() = img.getTextureConfig();
		return {};
	}	

	ImageTextureSourceLoader::ImageTextureSourceLoader(IDataLoader& dataLoader) noexcept
		: _dataLoader{ dataLoader }
		, _loadFlags{ defaultTextureLoadFlags }
	{
	}

	bool ImageTextureSourceLoader::supports(const std::filesystem::path& path) const noexcept
	{
		return Image::getEncodingForPath(path) != ImageEncoding::Count;
	}

	ImageTextureSourceLoader& ImageTextureSourceLoader::setLoadFlags(uint64_t flags) noexcept
	{
		_loadFlags = flags;
		return *this;
	}

	ImageTextureSourceLoader::Result ImageTextureSourceLoader::operator()(std::filesystem::path path)
	{
		auto encoding = Image::getEncodingForPath(path);
		if (encoding == ImageEncoding::Count)
		{
			return unexpected{ "format not supported" };
		}
		auto dataResult = _dataLoader(path);
		if (!dataResult)
		{
			return unexpected{ dataResult.error() };
		}

		auto src = std::make_shared<Texture::Source>();
		src->set_flags(_loadFlags);
		auto loadResult = TextureSourceWrapper{ *src }.loadData(dataResult.value(), encoding);
		if (!loadResult)
		{
			return unexpected{ loadResult.error() };
		}
		return src;
	}

	ImageTextureDefinitionLoader::ImageTextureDefinitionLoader(IImageLoader& imgLoader) noexcept
		: _imgLoader{ imgLoader }
		, _loadFlags{ defaultTextureLoadFlags }
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

	ImageTextureDefinitionLoader::Result ImageTextureDefinitionLoader::operator()(std::filesystem::path path)
	{
		if (!supports(path))
		{
			return unexpected{ "format not supported" };
		}
		auto imgResult = _imgLoader(path);
		if (!imgResult)
		{
			return unexpected{ imgResult.error() };
		}
		auto img = *imgResult;
		if (!img)
		{
			return unexpected{ "empty image" };
		}

		auto def = std::make_shared<Texture::Definition>();
		def->set_flags(_loadFlags);
		auto loadResult = TextureDefinitionWrapper{ *def }.loadImage(*img);
		if (!loadResult)
		{
			return unexpected{ loadResult.error() };
		}
		return def;
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

	std::string_view Texture::getTypeName(Texture::Type type) noexcept
	{
		return magic_enum::enum_name(type);
	}

	std::optional<Texture::Type> Texture::readType(std::string_view name) noexcept
	{
		return magic_enum::enum_cast<Texture::Type>(name);
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

	Texture::Source Texture::createSource() noexcept
	{
		Source src;
		src.set_encoding(Texture::Source::Tga);
		return src;
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
		: _handle{ handle }
		, _config{ cfg }
	{
	}

	Texture::Texture(const Image& img, uint64_t flags)
		: Texture(img.getData(), img.getTextureConfig(), flags)
	{
	}

	Texture::Texture(const DataView& data, const Config& cfg, uint64_t flags)
		: _handle{ bgfx::kInvalidHandle }
		, _config{ cfg }
	{
		// copying the memory of the image becauyse bgfx needs to maintain the memory for some frames
		const auto mem = data.copyMem();
		auto w = static_cast<uint16_t>(_config.size().x());
		auto h = static_cast<uint16_t>(_config.size().y());
		auto format = static_cast<bgfx::TextureFormat::Enum>(_config.format());
		switch (getType())
		{
		case Definition::CubeMap:
			_handle = bgfx::createTextureCube(w, _config.mips(), _config.layers(), format, flags, mem);
			break;
		case Definition::Texture3D:
			_handle = bgfx::createTexture3D(w, h, _config.depth(), _config.mips(), format, flags, mem);
			break;
		case Definition::Texture2D:
			_handle = bgfx::createTexture2D(w, h, _config.mips(), _config.layers(), format, flags, mem);
			break;
		}
	}

	Texture::Texture(const Config& cfg, uint64_t flags)
		: _handle{ bgfx::kInvalidHandle }
		, _config{ cfg }
	{
		bgfx::TextureHandle handle{ bgfx::kInvalidHandle };
		auto format = static_cast<bgfx::TextureFormat::Enum>(cfg.format());
		switch (getType())
		{
		case Definition::CubeMap:
			_handle = bgfx::createTextureCube(cfg.size().x(), cfg.mips(), cfg.layers(), format, flags);
			break;
		case Definition::Texture2D:
			_handle = bgfx::createTexture2D(cfg.size().x(), cfg.size().y(), cfg.mips(), cfg.layers(), format, flags);
			break;
		case Definition::Texture3D:
			_handle = bgfx::createTexture3D(cfg.size().x(), cfg.size().y(), cfg.depth(), cfg.mips(), format, flags);
			break;
		}
	}

	Texture::Texture(const Definition& def) noexcept
		: Texture(DataView{ def.data() }, def.config(), def.flags())
	{
	}

	Texture::Texture(Texture&& other) noexcept
		: _handle{ other._handle }
		, _config{ other._config }
	{
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = getEmptyConfig();
	}

	Texture& Texture::operator=(Texture&& other) noexcept
	{
		_handle = other._handle;
		_config = other._config;
		other._handle.idx = bgfx::kInvalidHandle;
		other._config = getEmptyConfig();
		return *this;
	}

	Texture::~Texture() noexcept
	{
		if (isValid(_handle))
		{
			bgfx::destroy(_handle);
		}
	}

	bgfx::TextureInfo Texture::getInfo() const noexcept
	{
		bgfx::TextureInfo info;
		auto cubeMap = _config.type() == Texture::Definition::CubeMap;
		auto format = static_cast<bgfx::TextureFormat::Enum>(_config.format());
		bgfx::calcTextureSize(info, _config.size().x(), _config.size().y(),
			_config.depth(), cubeMap, _config.mips(), _config.layers(), format);
		return info;
	}

	const protobuf::TextureConfig& Texture::getEmptyConfig() noexcept
	{
		static const protobuf::TextureConfig config;
		return config;
	}

	uint32_t Texture::getStorageSize() const noexcept
	{
		return getInfo().storageSize;
	}

	uint8_t Texture::getMipsCount() const noexcept
	{
		return getInfo().numMips;
	}

	uint8_t Texture::getBitsPerPixel() const noexcept
	{
		return getInfo().bitsPerPixel;
	}

	expected<void, std::string> Texture::update(const DataView& data, uint8_t mip)
	{
		if (getType() == Definition::Texture3D)
		{
			return update(data, glm::uvec3(getSize(), getDepth()), glm::uvec3(0), mip);
		}
		else
		{
			return update(data, getSize(), glm::uvec2(0), mip);
		}
	}

	expected<void, std::string> Texture::update(const DataView& data, const glm::uvec2& size, const glm::uvec2& origin, uint8_t mip, uint16_t layer, uint8_t side)
	{
		if (getType() == Definition::Texture3D)
		{
			return unexpected{ "does not work on 3D textures" };
		}
		auto memSize = size.x * size.y * getBitsPerPixel() / 8;
		if (memSize == 0)
		{
			return {};
		}
		if (data.size() < memSize)
		{
			return unexpected{ "data is smaller that expected size" };
		}
		if (getType() == Definition::Texture2D)
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
		return {};
	}

	expected<void, std::string> Texture::update(const DataView& data, const glm::uvec3& size, const glm::uvec3& origin, uint8_t mip)
	{
		if (getType() != Definition::Texture3D)
		{
			return unexpected{ "does only work on 3D textures" };
		}
		auto memSize = size.x * size.y * size.z * getBitsPerPixel() / 8;
		if (memSize == 0)
		{
			return {};
		}
		if (data.size() < memSize)
		{
			return unexpected{ "data is smaller that expected size" };
		}
		bgfx::updateTexture3D(_handle, mip,
			origin.x, origin.y, origin.y, size.x, size.y, size.z,
			data.copyMem(0, memSize));
		return {};
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
		return protobuf::convert(_config.size());
	}

	Texture::Type Texture::getType() const noexcept
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

	Texture::UniformKey Texture::createUniformKey(const std::string & name, uint8_t stage) noexcept
	{
		UniformKey key;
		key.set_name(name);
		key.set_stage(stage);
		return key;
	}

	TextureDefinitionFromSourceLoader::TextureDefinitionFromSourceLoader(ITextureSourceLoader& srcLoader, bx::AllocatorI& alloc) noexcept
		: FromDefinitionLoader(srcLoader)
		, _alloc{ alloc }
	{
	}

	TextureDefinitionFromSourceLoader::Result TextureDefinitionFromSourceLoader::create(const std::shared_ptr<protobuf::TextureSource>& src)
	{
		auto def = std::make_shared<protobuf::Texture>();
		auto result = TextureDefinitionWrapper{ *def }.loadSource(*src, *_alloc);
		if(!result)
		{
			return unexpected{ result.error() };
		}
		return def;
	}

	TextureFileImporter::TextureFileImporter()
		: _dataLoader{ _alloc }
		, _imgLoader{ _dataLoader, _alloc }
		, _defLoader{ _imgLoader }
		, ProtobufFileImporter<ImageTextureDefinitionLoader>(_defLoader, "texture")
	{
	}

	void TextureFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
	{
		auto defResult = _defLoader(input.path);
		if (!defResult)
		{
			throw std::runtime_error(defResult.error());
		}
		auto& def = **defResult;
		auto& config = *def.mutable_config();
		auto result = protobuf::read(config, input.dirConfig);
		if (!result)
		{
			throw std::runtime_error(result.error());
		}
		result = protobuf::read(config, input.config);
		if (!result)
		{
			throw std::runtime_error(result.error());
		}
		ProtobufFileImporter::writeOutput(def, out);
	}
}