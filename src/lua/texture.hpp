#pragma once

#include <sol/sol.hpp>
#include "glm.hpp"

namespace darmok
{
	struct TextureConfig;

	class LuaTexture final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static TextureConfig createSizeConfig(const VarLuaTable<glm::uvec2>& size) noexcept;
	};	

	class LuaTextureAtlas final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	};
}