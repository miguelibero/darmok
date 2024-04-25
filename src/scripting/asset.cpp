#include "asset.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "program.hpp"
#include "texture.hpp"
#include "model.hpp"
#include "material.hpp"
#include <darmok/math.hpp>
#include <darmok/asset.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/model.hpp>

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

	LuaModel LuaAssets::loadModel(const std::string& name)
	{
		return LuaModel(_assets->getModelLoader()(name));
	}

	void LuaAssets::configure(sol::state_view& lua) noexcept
	{
		LuaProgram::configure(lua);
		LuaTexture::configure(lua);
		LuaTextureAtlas::configure(lua);
		LuaTextureAtlasMeshCreator::configure(lua);
		LuaMaterial::configure(lua);
		LuaMesh::configure(lua);
		LuaMeshCreator::configure(lua);
		LuaModel::configure(lua);

		lua.new_usertype<LuaAssets>("Assets",
			sol::constructors<>(),
			"load_program", &LuaAssets::loadProgram,
			"load_standard_program", &LuaAssets::loadStandardProgram,
			"load_texture", sol::overload(&LuaAssets::loadTexture1, &LuaAssets::loadTexture2),
			"load_color_texture", &LuaAssets::loadColorTexture,
			"load_texture_atlas", sol::overload(&LuaAssets::loadTextureAtlas1, &LuaAssets::loadTextureAtlas2),
			"load_model", &LuaAssets::loadModel
		);
	}
}