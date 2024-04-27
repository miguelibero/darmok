#include "asset.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "texture.hpp"
#include "material.hpp"
#include <darmok/shape.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>

#ifdef DARMOK_ASSIMP
#include "assimp.hpp"
#include <darmok/assimp.hpp>
#endif

namespace darmok
{	
	LuaAssets::LuaAssets(AssetContext& assets) noexcept
		: _assets(assets)
	{
	}

	LuaProgram LuaAssets::loadProgram(const std::string& name)
	{
		return LuaProgram(_assets->getProgramLoader()(name));
	}

	LuaProgram LuaAssets::loadStandardProgram(StandardProgramType type)
	{
		return LuaProgram(_assets->getStandardProgramLoader()(type));
	}

	LuaTexture LuaAssets::loadColorTexture(const Color& color)
	{
		return LuaTexture(_assets->getColorTextureLoader()(color));
	}

	LuaTextureAtlas LuaAssets::loadTextureAtlas1(const std::string& name)
	{
		return LuaTextureAtlas(_assets->getTextureAtlasLoader()(name));
	}

	LuaTextureAtlas LuaAssets::loadTextureAtlas2(const std::string& name, uint64_t textureFlags)
	{
		return LuaTextureAtlas(_assets->getTextureAtlasLoader()(name, textureFlags));
	}

	LuaTexture LuaAssets::loadTexture1(const std::string& name)
	{
		return LuaTexture(_assets->getTextureLoader()(name));
	}

	LuaTexture LuaAssets::loadTexture2(const std::string& name, uint64_t flags)
	{
		return LuaTexture(_assets->getTextureLoader()(name, flags));
	}

#ifdef DARMOK_ASSIMP
	LuaAssimpScene LuaAssets::loadAssimp(const std::string& name)
	{
		return LuaAssimpScene(_assets->getAssimpLoader()(name));
	}
#endif

	void LuaAssets::configure(sol::state_view& lua) noexcept
	{
		LuaProgram::configure(lua);
		LuaTexture::configure(lua);
		LuaTextureAtlas::configure(lua);
		LuaTextureAtlasMeshCreator::configure(lua);
		LuaMaterial::configure(lua);
		LuaMesh::configure(lua);
		LuaMeshCreator::configure(lua);
#ifdef DARMOK_ASSIMP
		LuaAssimpScene::configure(lua);
#endif

		lua.new_usertype<LuaAssets>("Assets",
			sol::constructors<>(),
#ifdef DARMOK_ASSIMP
			"load_assimp", &LuaAssets::loadAssimp,
#endif
			"load_program", &LuaAssets::loadProgram,
			"load_standard_program", &LuaAssets::loadStandardProgram,
			"load_texture", sol::overload(&LuaAssets::loadTexture1, &LuaAssets::loadTexture2),
			"load_color_texture", &LuaAssets::loadColorTexture,
			"load_texture_atlas", sol::overload(&LuaAssets::loadTextureAtlas1, &LuaAssets::loadTextureAtlas2)
		);
	}
}