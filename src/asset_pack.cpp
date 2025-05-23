#include <darmok/asset_pack.hpp>

namespace darmok
{
	AssetPack::AssetPack(const Definition& def)
		: _def{ def }
		, _progDefLoader{ _def }
		, _texDefLoader{ _def }
		, _meshDefLoader{ _def }
		, _matDefLoader{ _def }
		, _programLoader{ _progDefLoader }
		, _textureLoader{ _texDefLoader }
		, _meshLoader{ _meshDefLoader }
		, _materialLoader{ _matDefLoader, _programLoader, _textureLoader }
	{
	}

	IProgramLoader& AssetPack::getProgramLoader() noexcept
	{
		return _programLoader;
	}		

	ITextureLoader& AssetPack::getTextureLoader() noexcept
	{
		return _textureLoader;
	}

	IMeshLoader& AssetPack::getMeshLoader() noexcept
	{
		return _meshLoader;
	}

	IMaterialLoader& AssetPack::getMaterialLoader() noexcept
	{
		return _materialLoader;
	}

}