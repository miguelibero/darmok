#include "texture.hpp"
#include <darmok/texture.hpp>
#include <darmok/image.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/anim.hpp>

namespace darmok
{
	void LuaTexture::bind(sol::state_view& lua) noexcept
	{
		auto textureFlagTable = lua.create_named_table("TextureFlag");
		for (const auto& [name, flag] : Texture::getTextureFlags())
		{
			textureFlagTable[name] = flag;
		}
		
		auto samplerFlagTable = lua.create_named_table("SamplerFlag");
		for (const auto& [name, flag] : Texture::getSamplerFlags())
		{
			samplerFlagTable[name] = flag;
		}
		samplerFlagTable["BORDER_COLOR"] = [](uint32_t v) { return BGFX_SAMPLER_BORDER_COLOR(v); };

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
			sol::factories(
				[]() { return TextureConfig{}; },
				[](const VarLuaTable<glm::uvec2>& size) { return TextureConfig{ LuaGlm::tableGet(size) }; }
			),
			"size", &TextureConfig::size,
			"depth", &TextureConfig::depth,
			"mips", &TextureConfig::mips,
			"layers", &TextureConfig::layers,
			"type", &TextureConfig::type,
			"format", &TextureConfig::format,
			sol::meta_function::to_string, &TextureConfig::toString
		);

		lua.new_usertype<Texture>("Texture",
			sol::factories(
				[](const Image& img) { return std::make_shared<Texture>(img); },
				[](const Image& img, uint64_t flags) { return std::make_shared<Texture>(img, flags); },
				[](const TextureConfig& config) { return std::make_shared<Texture>(config); },
				[](const TextureConfig& config, uint64_t flags) { return std::make_shared<Texture>(config, flags); },
				[](const VarLuaTable<glm::uvec2>& size) { return std::make_shared<Texture>(createSizeConfig(size)); },
				[](const VarLuaTable<glm::uvec2>& size, uint64_t flags)
				{
					return std::make_shared<Texture>(createSizeConfig(size), flags);
				},
				[](const VarLuaTable<glm::uvec2>& size, bgfx::TextureFormat::Enum format)
				{
					auto config = createSizeConfig(size);
					config.format = format;
					return std::make_shared<Texture>(config);
				},
				[](const VarLuaTable<glm::uvec2>& size, bgfx::TextureFormat::Enum format, uint64_t flags)
				{
					auto config = createSizeConfig(size);
					config.format = format;
					return std::make_shared<Texture>(config, flags);
				}
			),
			"type", sol::property(&Texture::getType),
			"size", sol::property(&Texture::getSize),
			"format", sol::property(&Texture::getFormat),
			"layers", sol::property(&Texture::getLayerCount),
			"depth", sol::property(&Texture::getDepth),
			"mips", sol::property(&Texture::hasMips),
			"name", sol::property(&Texture::setName),
			sol::meta_function::to_string, &Texture::toString
		);
	}

	TextureConfig LuaTexture::createSizeConfig(const VarLuaTable<glm::uvec2>& size) noexcept
	{
		return TextureConfig(LuaGlm::tableGet(size));
	};

	void LuaTextureAtlas::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<TextureAtlas>("TextureAtlas", sol::no_constructor,
			"texture", &TextureAtlas::texture,
			"create_sprite", sol::overload(
				[](const TextureAtlas& atlas, const std::string& name, const bgfx::VertexLayout& layout)
				{
					return atlas.createSprite(name, layout);
				},
				[](const TextureAtlas& atlas, const std::string& name, const bgfx::VertexLayout& layout, const TextureAtlas::MeshConfig& config)
				{
					return atlas.createSprite(name, layout, config);
				}
			),
			"create_animation", sol::overload(
				[](const TextureAtlas& atlas, const bgfx::VertexLayout& layout)
				{
					return atlas.createAnimation(layout);
				},
				[](const TextureAtlas& atlas, const bgfx::VertexLayout& layout, const std::string& namePrefix)
				{
					return atlas.createAnimation(layout, namePrefix);
				},
				[](const TextureAtlas& atlas, const bgfx::VertexLayout& layout, const std::string& namePrefix, float frameDuration)
				{
					return atlas.createAnimation(layout, namePrefix, frameDuration);
				},
				[](const TextureAtlas& atlas, const bgfx::VertexLayout& layout, const std::string& namePrefix, float frameDuration, const TextureAtlas::MeshConfig& config)
				{
					return atlas.createAnimation(layout, namePrefix, frameDuration, config);
				}
			)
		);
	
		lua.new_usertype<TextureAtlasMeshConfig>("TextureAtlasMeshConfig",
			sol::default_constructor,
			"scale", &TextureAtlasMeshConfig::scale,
			"offset", &TextureAtlasMeshConfig::offset,
			"color", &TextureAtlasMeshConfig::color,
			"amount", &TextureAtlasMeshConfig::amount
		);
	}

}