#include "lua/asset_pack.hpp"
#include "lua/utils.hpp"
#include <darmok/stream.hpp>
#include <darmok/asset_pack.hpp>

namespace darmok
{
	void LuaAssetPack::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<AssetPack>(
			"AssetPack", sol::no_constructor,
			sol::base_classes, sol::bases<IAssetContext>(),
			"reload_asset", &LuaAssetPack::reloadAsset,
			"remove_asset", &LuaAssetPack::removeAsset
		);
	}

	void LuaAssetPack::reloadAsset(AssetPack& assetPack, const std::filesystem::path& path)
	{
		auto result = assetPack.reloadAsset(path);
		if(!result)
		{
			throw sol::error{ "reloading asset pack asset: " + result.error() };
		}
	}

	void LuaAssetPack::removeAsset(AssetPack& assetPack, const std::filesystem::path& path)
	{
		auto result = assetPack.removeAsset(path);
		if (!result)
		{
			throw sol::error{ "removing asset pack asset: " + result.error() };
		}
	}
}