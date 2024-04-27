#pragma once

#include <string>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program.hpp>
#include "sol.hpp"

namespace darmok
{
	class AssetContext;
	class LuaProgram;
	class LuaTexture;
	class LuaTextureAtlas;
	class LuaAssimpScene;

	class LuaAssets final
	{
	public:
		LuaAssets(AssetContext& assets) noexcept;
		LuaProgram loadProgram(const std::string& name);
		LuaProgram loadStandardProgram(StandardProgramType type);
		LuaTexture loadTexture1(const std::string& name);
		LuaTexture loadTexture2(const std::string& name, uint64_t flags);
		LuaTexture loadColorTexture(const Color& color);
		LuaTextureAtlas loadTextureAtlas1(const std::string& name);
		LuaTextureAtlas loadTextureAtlas2(const std::string& name, uint64_t textureFlags);
#ifdef DARMOK_ASSIMP
		LuaAssimpScene loadAssimp(const std::string& name);
#endif

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<AssetContext> _assets;
	};
}