#include "lua/texture.hpp"
#include "lua/utils.hpp"
#include "lua/protobuf.hpp"
#include <darmok/texture.hpp>
#include <darmok/image.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/anim.hpp>
#include <darmok/glm_serialize.hpp>

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

		LuaUtils::newEnum<Texture::Type>(lua, "TextureType");
		LuaUtils::newEnum<Texture::Format>(lua, "TextureFormat");
		LuaProtobufBinding<Texture::Config>{lua, "TextureConfig"}
			.convertProtobufProperty<glm::uvec2, protobuf::Uvec2>("size");

			LuaProtobufBinding<Texture::Source>{lua, "TextureSource"};
		LuaProtobufBinding<Texture::Definition>{lua, "TextureDefinition"}
			.protobufProperty<Texture::Config>("config");

		lua.new_usertype<Texture>("Texture",
			sol::factories(
				[](const Image& img) {
					auto tex = LuaUtils::unwrapExpected(Texture::load(img));
					return std::make_shared<Texture>(std::move(tex));
				},
				[](const Image& img, uint64_t flags) {
					auto tex = LuaUtils::unwrapExpected(Texture::load(img, flags));
					return std::make_shared<Texture>(std::move(tex));
				},
				[](const Texture::Config& config) {
					auto tex = LuaUtils::unwrapExpected(Texture::load(config));
					return std::make_shared<Texture>(std::move(tex));
				},
				[](const Texture::Config& config, uint64_t flags) {
					auto tex = LuaUtils::unwrapExpected(Texture::load(config, flags));
					return std::make_shared<Texture>(std::move(tex));
				},
				[](const VarLuaTable<glm::uvec2>& size) {
					auto tex = LuaUtils::unwrapExpected(Texture::load(createSizeConfig(size)));
					return std::make_shared<Texture>(std::move(tex));
				},
				[](const VarLuaTable<glm::uvec2>& size, uint64_t flags)
				{
					auto tex = LuaUtils::unwrapExpected(Texture::load(createSizeConfig(size), flags));
					return std::make_shared<Texture>(std::move(tex));
				},
				[](const VarLuaTable<glm::uvec2>& size, Texture::Format format)
				{
					auto config = createSizeConfig(size);
					config.set_format(format);
					auto tex = LuaUtils::unwrapExpected(Texture::load(config));
					return std::make_shared<Texture>(std::move(tex));
				},
				[](const VarLuaTable<glm::uvec2>& size, Texture::Format format, uint64_t flags)
				{
					auto config = createSizeConfig(size);
					config.set_format(format);
					auto tex = LuaUtils::unwrapExpected(Texture::load(config, flags));
					return std::make_shared<Texture>(std::move(tex));
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

	Texture::Config LuaTexture::createSizeConfig(const VarLuaTable<glm::uvec2>& size) noexcept
	{
		Texture::Config config;		
		*config.mutable_size() = convert<protobuf::Uvec2>(LuaGlm::tableGet(size));
		return config;
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