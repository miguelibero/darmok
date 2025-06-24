#include <darmok/asset_pack.hpp>

namespace darmok
{
	AssetPack::AssetPack(const Definition& def, const AssetPackConfig& config)
		: _def{ def }
		, _progDefLoader{ _def }
		, _texDefLoader{ _def }
		, _meshDefLoader{ _def }
		, _matDefLoader{ _def }
		, _armDefLoader{ _def }
		, _progSrcLoader{ _def }
		, _progDefFromSrcLoader{ _progSrcLoader, config.programCompilerConfig}
		, _texSrcLoader{ _def }
		, _texDefFromSrcLoader{ _texSrcLoader, config.alloc ? *config.alloc : _defaultAlloc }
		, _meshSrcLoader{ _def }
		, _meshDefFromSrcLoader{ _meshSrcLoader, _multiProgramDefLoader }
		, _multiProgramDefLoader{ _progDefLoader, _progDefFromSrcLoader }
		, _multiTextureDefLoader{ _texDefLoader, _texDefFromSrcLoader }
		, _multiMeshDefLoader{ _meshDefLoader, _meshDefFromSrcLoader }
		, _programLoader{ _multiProgramDefLoader }
		, _textureLoader{ _multiTextureDefLoader }
		, _meshLoader{ _multiMeshDefLoader }
		, _materialLoader{ _matDefLoader, _multiProgramLoader, _multiTextureLoader }
		, _armatureLoader{ _armDefLoader }
		, _multiProgramLoader{ _programLoader }
		, _multiTextureLoader{ _textureLoader }
		, _multiMeshLoader{ _meshLoader }
		, _multiMaterialLoader{ _materialLoader }
		, _multiArmatureLoader{ _armatureLoader }
	{
		
		if (config.fallbacks.program)
		{
			_multiProgramLoader.addBack(*config.fallbacks.program);
		}
		if (config.fallbacks.texture)
		{
			_multiTextureLoader.addBack(*config.fallbacks.texture);
		}
		if (config.fallbacks.mesh)
		{
			_multiMeshLoader.addBack(*config.fallbacks.mesh);
		}
		if (config.fallbacks.material)
		{
			_multiMaterialLoader.addBack(*config.fallbacks.material);
		}
		if (config.fallbacks.armature)
		{
			_multiArmatureLoader.addBack(*config.fallbacks.armature);
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