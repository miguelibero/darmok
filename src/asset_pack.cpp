#include <darmok/asset_pack.hpp>

namespace darmok
{
	void AssetPack::clear() noexcept
	{
		programs.clear();
		textures.clear();
		meshes.clear();
		models.clear();
		scenes.clear();
	}
}