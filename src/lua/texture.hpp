#pragma once

#include "lua/lua.hpp"
#include "lua/glm.hpp"

namespace darmok
{
	namespace protobuf
	{
		class TextureConfig;
	}

	class LuaTexture final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static protobuf::TextureConfig createSizeConfig(const VarLuaTable<glm::uvec2>& size) noexcept;
	};	

	class LuaTextureAtlas final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	};
}