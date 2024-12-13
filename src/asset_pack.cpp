#include <darmok/asset_pack.hpp>

namespace darmok
{
	void AssetPack::clear() noexcept
	{
		materials.clear();
		meshes.clear();
		models.clear();
		scenes.clear();
	}
}