#include <darmok/asset_pack.hpp>

namespace darmok
{
	AssetPack::AssetPack(const Definition& def, const AssetPackFallbacks& fallbacks)
		: _def{ def }
		, _fallbacks{ fallbacks }
		, _progDefLoader{ _def }
		, _texDefLoader{ _def }
		, _meshDefLoader{ _def }
		, _matDefLoader{ _def }
		, _armDefLoader{ _def }
		, _programLoader{ _progDefLoader }
		, _textureLoader{ _texDefLoader }
		, _meshLoader{ _meshDefLoader }
		, _materialLoader{ _matDefLoader, _multiProgramLoader, _multiTextureLoader }
		, _armatureLoader{ _armDefLoader }
		, _multiProgramLoader{ _programLoader }
		, _multiTextureLoader{ _textureLoader }
		, _multiMeshLoader{ _meshLoader }
		, _multiMaterialLoader{ _materialLoader }
		, _multiArmatureLoader{ _armatureLoader }
	{
		if (fallbacks.programLoader)
		{
			_multiProgramLoader.addBack(*fallbacks.programLoader);
		}
		if (fallbacks.textureLoader)
		{
			_multiTextureLoader.addBack(*fallbacks.textureLoader);
		}
		if (fallbacks.meshLoader)
		{
			_multiMeshLoader.addBack(*fallbacks.meshLoader);
		}
		if (fallbacks.materialLoader)
		{
			_multiMaterialLoader.addBack(*fallbacks.materialLoader);
		}
		if (fallbacks.armatureLoader)
		{
			_multiArmatureLoader.addBack(*fallbacks.armatureLoader);
		}
	}

	template<>
	expected<std::shared_ptr<Program>, std::string> AssetPack::load(const std::filesystem::path& path) noexcept
	{ 
		return _multiProgramLoader(path);
	}

	template<>
	expected<std::shared_ptr<Texture>, std::string> AssetPack::load(const std::filesystem::path& path) noexcept
	{
		return _multiTextureLoader(path);
	}

	template<>
	expected<std::shared_ptr<IMesh>, std::string> AssetPack::load(const std::filesystem::path& path) noexcept
	{
		return _multiMeshLoader(path);
	}

	template<>
	expected<std::shared_ptr<Material>, std::string> AssetPack::load(const std::filesystem::path& path) noexcept
	{
		return _multiMaterialLoader(path);
	}

	template<>
	expected<std::shared_ptr<Armature>, std::string> AssetPack::load(const std::filesystem::path& path) noexcept
	{
		return _multiArmatureLoader(path);
	}
}