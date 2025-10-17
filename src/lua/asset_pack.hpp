#pragma once

#include "lua/lua.hpp"

#include <filesystem>

namespace darmok
{
	class AssetPack;

	class LuaAssetPack final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		static void reloadAsset(AssetPack& assetPack, const std::filesystem::path& path);
		static void removeAsset(AssetPack& assetPack, const std::filesystem::path& path);
	};
}