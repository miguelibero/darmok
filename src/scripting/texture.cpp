#include "texture.hpp"
#include "image.hpp"
#include "mesh.hpp"
#include "math.hpp"
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>

namespace darmok
{
    LuaTexture::LuaTexture(const std::shared_ptr<Texture>& texture) noexcept
	: _texture(texture)
	{
	}

	LuaTexture::LuaTexture(const LuaImage& img, uint64_t flags) noexcept
		: _texture(std::make_shared<Texture>(*img.getReal(), flags))
	{
	}

	LuaTexture::LuaTexture(const Config& cfg, uint64_t flags) noexcept
		: _texture(std::make_shared<Texture>(cfg, flags))
	{
	}

	LuaTexture::LuaTexture(const VarUvec2& size, bgfx::TextureFormat::Enum format, uint64_t flags) noexcept
		: LuaTexture(Config{ LuaMath::tableToGlm(size), format }, flags)
	{
	}

	LuaTexture::LuaTexture(const VarUvec2& size, uint64_t flags) noexcept
		: LuaTexture(Config{ LuaMath::tableToGlm(size)}, flags)
	{
	}

	const std::shared_ptr<Texture>& LuaTexture::getReal() const noexcept
	{
		return _texture;
	}

	TextureType LuaTexture::getType() const noexcept
	{
		return _texture->getType();
	}

	LuaTexture& LuaTexture::setName(const std::string& name) noexcept
	{
		_texture->setName(name);
		return *this;
	}

	const glm::uvec2& LuaTexture::getSize() noexcept
	{
		return _texture->getSize();
	}

	bgfx::TextureFormat::Enum LuaTexture::getFormat() const noexcept
	{
		return _texture->getFormat();
	}

	uint16_t LuaTexture::getLayerCount() const noexcept
	{
		return _texture->getLayerCount();
	}

	uint16_t LuaTexture::getDepth() const noexcept
	{
		return _texture->getDepth();
	}

	bool LuaTexture::hasMips() const noexcept
	{
		return _texture->hasMips();
	}

	std::string LuaTexture::to_string() const noexcept
	{
		return _texture->to_string();
	}

	void LuaTexture::configure(sol::state_view& lua) noexcept
	{
		lua.create_named_table("TextureFlag",
			"NONE", BGFX_TEXTURE_NONE,
			"MSAA_SAMPLE", BGFX_TEXTURE_MSAA_SAMPLE,
			"RT", BGFX_TEXTURE_RT,
			"COMPUTE_WRITE", BGFX_TEXTURE_COMPUTE_WRITE,
			"SRG", BGFX_TEXTURE_SRGB,
			"BLIT_DST", BGFX_TEXTURE_BLIT_DST,
			"READ_BACK", BGFX_TEXTURE_READ_BACK,
			"RT_MSAA_X2", BGFX_TEXTURE_RT_MSAA_X2,
			"RT_MSAA_X4", BGFX_TEXTURE_RT_MSAA_X4,
			"RT_MSAA_X8", BGFX_TEXTURE_RT_MSAA_X8,
			"RT_MSAA_X16", BGFX_TEXTURE_RT_MSAA_X16,
			"RT_MSAA_SHIFT", BGFX_TEXTURE_RT_MSAA_SHIFT,
			"RT_MSAA_MASK", BGFX_TEXTURE_RT_MSAA_MASK,
			"RT_WRITE_ONLY", BGFX_TEXTURE_RT_WRITE_ONLY,
			"RT_SHIFT", BGFX_TEXTURE_RT_SHIFT,
			"RT_MASK", BGFX_TEXTURE_RT_MASK
		);

		lua.create_named_table("SamplerFlag",
			"NONE", BGFX_SAMPLER_NONE,
			"U_MIRROR", BGFX_SAMPLER_U_MIRROR,
			"U_CLAMP", BGFX_SAMPLER_U_CLAMP,
			"U_BORDER", BGFX_SAMPLER_U_BORDER,
			"U_SHIFT", BGFX_SAMPLER_U_SHIFT,
			"U_MASK", BGFX_SAMPLER_U_MASK,
			"V_MIRROR", BGFX_SAMPLER_V_MIRROR,
			"V_CLAMP", BGFX_SAMPLER_V_CLAMP,
			"V_BORDER", BGFX_SAMPLER_V_BORDER,
			"V_SHIFT", BGFX_SAMPLER_V_SHIFT,
			"V_MASK", BGFX_SAMPLER_V_MASK,
			"W_MIRROR", BGFX_SAMPLER_W_MIRROR,
			"W_CLAMP", BGFX_SAMPLER_W_CLAMP,
			"W_BORDER", BGFX_SAMPLER_W_BORDER,
			"W_SHIFT", BGFX_SAMPLER_W_SHIFT,
			"W_MASK", BGFX_SAMPLER_W_MASK,
			"MIN_POINT", BGFX_SAMPLER_MIN_POINT,
			"MIN_ANISOTROPIC", BGFX_SAMPLER_MIN_ANISOTROPIC,
			"MIN_SHIFT", BGFX_SAMPLER_MIN_SHIFT,
			"MIN_MASK", BGFX_SAMPLER_MIN_MASK,
			"MAG_POINT", BGFX_SAMPLER_MAG_POINT,
			"MAG_ANISOTROPIC", BGFX_SAMPLER_MAG_ANISOTROPIC,
			"MAG_SHIFT", BGFX_SAMPLER_MAG_SHIFT,
			"MAG_MASK", BGFX_SAMPLER_MAG_MASK,
			"MIP_POINT", BGFX_SAMPLER_MIP_POINT,
			"MIP_SHIFT", BGFX_SAMPLER_MIP_SHIFT,
			"MIP_MASK", BGFX_SAMPLER_MIP_MASK,
			"COMPARE_LESS", BGFX_SAMPLER_COMPARE_LESS,
			"COMPARE_LEQUAL", BGFX_SAMPLER_COMPARE_LEQUAL,
			"COMPARE_EQUAL", BGFX_SAMPLER_COMPARE_EQUAL,
			"COMPARE_GEQUAL", BGFX_SAMPLER_COMPARE_GEQUAL,
			"COMPARE_GREATER", BGFX_SAMPLER_COMPARE_GREATER,
			"COMPARE_NOTEQUAL", BGFX_SAMPLER_COMPARE_NOTEQUAL,
			"COMPARE_NEVER", BGFX_SAMPLER_COMPARE_NEVER,
			"COMPARE_ALWAYS", BGFX_SAMPLER_COMPARE_ALWAYS,
			"COMPARE_SHIFT", BGFX_SAMPLER_COMPARE_SHIFT,
			"COMPARE_MASK", BGFX_SAMPLER_COMPARE_MASK,
			"BORDER_COLOR_SHIFT", BGFX_SAMPLER_BORDER_COLOR_SHIFT,
			"BORDER_COLOR_MASK", BGFX_SAMPLER_BORDER_COLOR_MASK,
			"RESERVED_SHIFT", BGFX_SAMPLER_RESERVED_SHIFT,
			"RESERVED_MASK", BGFX_SAMPLER_RESERVED_MASK,
			"SAMPLE_STENCIL", BGFX_SAMPLER_SAMPLE_STENCIL,
			"POINT", BGFX_SAMPLER_POINT,
			"UVW_MIRROR", BGFX_SAMPLER_UVW_MIRROR,
			"UVW_CLAMP", BGFX_SAMPLER_UVW_CLAMP,
			"UVW_BORDER", BGFX_SAMPLER_UVW_BORDER,
			"BITS_MASK", BGFX_SAMPLER_BITS_MASK,
			"BORDER_COLOR", [](uint32_t v) { return BGFX_SAMPLER_BORDER_COLOR(v); }
		);

		lua.new_enum<TextureType>("TextureType", {
			{ "Unknown", TextureType::Unknown },
			{ "CubeMap", TextureType::CubeMap },
			{ "Texture2D", TextureType::Texture2D },
			{ "Texture3D", TextureType::Texture3D },
		});

		lua.new_enum<bgfx::TextureFormat::Enum>("TextureFormat", {
			{ "BC1", bgfx::TextureFormat::BC1 },
			{ "BC2", bgfx::TextureFormat::BC2 },
			{ "BC3", bgfx::TextureFormat::BC3 },
			{ "BC4", bgfx::TextureFormat::BC4 },
			{ "BC5", bgfx::TextureFormat::BC5 },
			{ "BC6H", bgfx::TextureFormat::BC6H },
			{ "BC7", bgfx::TextureFormat::BC7 },
			{ "ETC1", bgfx::TextureFormat::ETC1 },
			{ "ETC2", bgfx::TextureFormat::ETC2 },
			{ "ETC2A", bgfx::TextureFormat::ETC2A },
			{ "ETC2A1", bgfx::TextureFormat::ETC2A1 },
			{ "PTC12", bgfx::TextureFormat::PTC12 },
			{ "PTC14", bgfx::TextureFormat::PTC14 },
			{ "PTC12A", bgfx::TextureFormat::PTC12A },
			{ "PTC14A", bgfx::TextureFormat::PTC14A },
			{ "PTC22", bgfx::TextureFormat::PTC22 },
			{ "PTC24", bgfx::TextureFormat::PTC24 },
			{ "ATC", bgfx::TextureFormat::ATC },
			{ "ATCE", bgfx::TextureFormat::ATCE },
			{ "ATCI", bgfx::TextureFormat::ATCI },
			{ "ASTC4x4", bgfx::TextureFormat::ASTC4x4 },
			{ "ASTC5x4", bgfx::TextureFormat::ASTC5x4 },
			{ "ASTC5x5", bgfx::TextureFormat::ASTC5x5 },
			{ "ASTC6x5", bgfx::TextureFormat::ASTC6x5 },
			{ "ASTC6x6", bgfx::TextureFormat::ASTC6x6 },
			{ "ASTC8x5", bgfx::TextureFormat::ASTC8x5 },
			{ "ASTC8x6", bgfx::TextureFormat::ASTC8x6 },
			{ "ASTC8x8", bgfx::TextureFormat::ASTC8x8 },
			{ "ASTC10x5", bgfx::TextureFormat::ASTC10x5 },
			{ "ASTC10x6", bgfx::TextureFormat::ASTC10x6 },
			{ "ASTC10x8", bgfx::TextureFormat::ASTC10x8 },
			{ "ASTC10x10", bgfx::TextureFormat::ASTC10x10 },
			{ "ASTC12x10", bgfx::TextureFormat::ASTC12x10 },
			{ "ASTC12x12", bgfx::TextureFormat::ASTC12x12 },
			{ "Unknown", bgfx::TextureFormat::Unknown },
			{ "R1", bgfx::TextureFormat::R1 },
			{ "A8", bgfx::TextureFormat::A8 },
			{ "R8", bgfx::TextureFormat::R8 },
			{ "R8I", bgfx::TextureFormat::R8I },
			{ "R8U", bgfx::TextureFormat::R8U },
			{ "R8S", bgfx::TextureFormat::R8S },
			{ "R16", bgfx::TextureFormat::R16 },
			{ "R16I", bgfx::TextureFormat::R16I },
			{ "R16U", bgfx::TextureFormat::R16U },
			{ "R16F", bgfx::TextureFormat::R16F },
			{ "R16S", bgfx::TextureFormat::R16S },
			{ "R32I", bgfx::TextureFormat::R32I },
			{ "R32U", bgfx::TextureFormat::R32U },
			{ "R32F", bgfx::TextureFormat::R32F },
			{ "RG8", bgfx::TextureFormat::RG8 },
			{ "RG8I", bgfx::TextureFormat::RG8I },
			{ "RG8U", bgfx::TextureFormat::RG8U },
			{ "RG8S", bgfx::TextureFormat::RG8S },
			{ "RG16", bgfx::TextureFormat::RG16 },
			{ "RG16I", bgfx::TextureFormat::RG16I },
			{ "RG16U", bgfx::TextureFormat::RG16U },
			{ "RG16F", bgfx::TextureFormat::RG16F },
			{ "RG16S", bgfx::TextureFormat::RG16S },
			{ "RG32I", bgfx::TextureFormat::RG32I },
			{ "RG32U", bgfx::TextureFormat::RG32U },
			{ "RG32F", bgfx::TextureFormat::RG32F },
			{ "RGB8", bgfx::TextureFormat::RGB8 },
			{ "RGB8I", bgfx::TextureFormat::RGB8I },
			{ "RGB8U", bgfx::TextureFormat::RGB8U },
			{ "RGB8S", bgfx::TextureFormat::RGB8S },
			{ "RGB9E5F", bgfx::TextureFormat::RGB9E5F },
			{ "BGRA8", bgfx::TextureFormat::BGRA8 },
			{ "RGBA8", bgfx::TextureFormat::RGBA8 },
			{ "RGBA8I", bgfx::TextureFormat::RGBA8I },
			{ "RGBA8U", bgfx::TextureFormat::RGBA8U },
			{ "RGBA8S", bgfx::TextureFormat::RGBA8S },
			{ "RGBA16", bgfx::TextureFormat::RGBA16 },
			{ "RGBA16I", bgfx::TextureFormat::RGBA16I },
			{ "RGBA16U", bgfx::TextureFormat::RGBA16U },
			{ "RGBA16F", bgfx::TextureFormat::RGBA16F },
			{ "RGBA16S", bgfx::TextureFormat::RGBA16S },
			{ "RGBA32I", bgfx::TextureFormat::RGBA32I },
			{ "RGBA32U", bgfx::TextureFormat::RGBA32U },
			{ "RGBA32F", bgfx::TextureFormat::RGBA32F },
			{ "B5G6R5", bgfx::TextureFormat::B5G6R5 },
			{ "R5G6B5", bgfx::TextureFormat::R5G6B5 },
			{ "BGRA4", bgfx::TextureFormat::BGRA4 },
			{ "RGBA4", bgfx::TextureFormat::RGBA4 },
			{ "BGR5A1", bgfx::TextureFormat::BGR5A1 },
			{ "RGB5A1", bgfx::TextureFormat::RGB5A1 },
			{ "RGB10A2", bgfx::TextureFormat::RGB10A2 },
			{ "RG11B10F", bgfx::TextureFormat::RG11B10F },
			{ "UnknownDepth", bgfx::TextureFormat::UnknownDepth },
			{ "D16", bgfx::TextureFormat::D16 },
			{ "D24", bgfx::TextureFormat::D24 },
			{ "D24S8", bgfx::TextureFormat::D24S8 },
			{ "D32", bgfx::TextureFormat::D32 },
			{ "D16F", bgfx::TextureFormat::D16F },
			{ "D24F", bgfx::TextureFormat::D24F },
			{ "D32F", bgfx::TextureFormat::D32F },
			{ "D0S8", bgfx::TextureFormat::D0S8 }
		});

		lua.new_usertype<TextureConfig>("TextureConfig",
			sol::constructors<TextureConfig(), TextureConfig(const glm::uvec2&)>(),
			"size", &TextureConfig::size,
			"depth", &TextureConfig::depth,
			"mips", &TextureConfig::mips,
			"layers", &TextureConfig::layers,
			"type", &TextureConfig::type,
			"format", &TextureConfig::format
		);

		lua.new_usertype<LuaTexture>("Texture",
			sol::constructors<
				LuaTexture(const LuaImage&),
				LuaTexture(const LuaImage&, uint64_t),
				LuaTexture(const TextureConfig&),
				LuaTexture(const TextureConfig&, uint64_t),
				LuaTexture(const VarUvec2&),
				LuaTexture(const VarUvec2&, uint64_t),
				LuaTexture(const VarUvec2&, const bgfx::TextureFormat::Enum&),
				LuaTexture(const VarUvec2&, const bgfx::TextureFormat::Enum&, uint64_t)
			>(),
			"type", sol::property(&LuaTexture::getType),
			"size", sol::property(&LuaTexture::getSize),
			"format", sol::property(&LuaTexture::getFormat),
			"layers", sol::property(&LuaTexture::getLayerCount),
			"depth", sol::property(&LuaTexture::getDepth),
			"mips", sol::property(&LuaTexture::hasMips),
			"name", sol::property(&LuaTexture::setName)
		);
	}

	
	LuaTextureAtlas::LuaTextureAtlas(const std::shared_ptr<TextureAtlas>& atlas) noexcept
		: _atlas(atlas)
	{
	}

	const std::shared_ptr<TextureAtlas>& LuaTextureAtlas::LuaTextureAtlas::getReal() const noexcept
	{
		return _atlas;
	}

	void LuaTextureAtlas::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaTextureAtlas>("TextureAtlas",
			sol::constructors<>()
		);
	}

	LuaTextureAtlasMeshCreator::LuaTextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const LuaTextureAtlas& atlas) noexcept
		: _creator(std::make_shared<TextureAtlasMeshCreator>(layout, *atlas.getReal()))
		, _atlas(atlas)
	{
	}

	LuaTextureAtlasMeshCreator::~LuaTextureAtlasMeshCreator()
	{
	}

	LuaTextureAtlasMeshCreator::Config& LuaTextureAtlasMeshCreator::getConfig() noexcept
	{
		return _creator->config;
	}

	void LuaTextureAtlasMeshCreator::setConfig(const Config& config) noexcept
	{
		_creator->config = config;
	}

	bgfx::VertexLayout& LuaTextureAtlasMeshCreator::getVertexLayout()  noexcept
	{
		return _creator->layout;
	}

	const LuaTextureAtlas& LuaTextureAtlasMeshCreator::getTextureAtlas() noexcept
	{
		return _atlas;
	}

	LuaMesh LuaTextureAtlasMeshCreator::createSprite(const std::string& name) const noexcept
	{
		return LuaMesh(_creator->createSprite(name));
	}

	std::vector<AnimationFrame> LuaTextureAtlasMeshCreator::createAnimation1(const std::string& namePrefix) const noexcept
	{
		return _creator->createAnimation(namePrefix);
	}

	std::vector<AnimationFrame> LuaTextureAtlasMeshCreator::createAnimation2(const std::string& namePrefix, float frameDuration) const noexcept
	{
		return _creator->createAnimation(namePrefix, frameDuration);
	}

	void LuaTextureAtlasMeshCreator::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<TextureAtlasMeshCreationConfig>("TextureAtlasMeshCreationConfig",
			sol::constructors<
				MeshCreationConfig(const glm::vec3&, const glm::vec3&, const Color&, const glm::uvec2& amount),
				MeshCreationConfig(const glm::vec3&, const glm::vec3&, const Color&),
				MeshCreationConfig(const glm::vec3&, const glm::vec3&),
				MeshCreationConfig(const glm::vec3&),
				MeshCreationConfig()
			>(),
			"scale", &TextureAtlasMeshCreationConfig::scale,
			"offset", &TextureAtlasMeshCreationConfig::offset,
			"color", &TextureAtlasMeshCreationConfig::color,
			"amount", &TextureAtlasMeshCreationConfig::amount
		);

		lua.new_usertype<LuaTextureAtlasMeshCreator>("TextureAtlasMeshCreator",
			sol::constructors<
				LuaTextureAtlasMeshCreator(const bgfx::VertexLayout&, const LuaTextureAtlas&)>(),
			"config", sol::property(&LuaTextureAtlasMeshCreator::getConfig, &LuaTextureAtlasMeshCreator::setConfig),
			"vertex_layout", sol::property(&LuaTextureAtlasMeshCreator::getVertexLayout),
			"atlas", sol::property(&LuaTextureAtlasMeshCreator::getTextureAtlas),
			"create_sprite", &LuaTextureAtlasMeshCreator::createSprite,
			"create_animation", sol::overload(
				&LuaTextureAtlasMeshCreator::createAnimation1,
				&LuaTextureAtlasMeshCreator::createAnimation2)
		);
	}

}