#include <darmok/asset_pack.hpp>

namespace darmok
{
	AssetPack::AssetPack(const Definition& def, const AssetPackConfig& config)
		: _def{ def }
		, _alloc{ config.fallback ? config.fallback->getAllocator() : _defaultAlloc }
		, _progDefLoader{ _def }
		, _texDefLoader{ _def }
		, _meshDefLoader{ _def }
		, _matDefLoader{ _def }
		, _armDefLoader{ _def }
		, _sceneDefLoader{ _def }

		, _progSrcLoader{ _def }
		, _progDefFromSrcLoader{ _progSrcLoader, config.programCompilerConfig }
		, _texSrcLoader{ _def }
		, _texDefFromSrcLoader{ _texSrcLoader, _alloc }
		, _meshSrcLoader{ _def }
		, _meshDefFromSrcLoader{ _meshSrcLoader, _multiProgramDefLoader, _alloc }

		, _programLoader{ _multiProgramDefLoader }
		, _textureLoader{ _multiTextureDefLoader }
		, _meshLoader{ _multiMeshDefLoader }
		, _materialLoader{ _matDefLoader, _multiProgramLoader, _multiTextureLoader }
		, _armatureLoader{ _armDefLoader }
		, _sceneLoader{ _sceneDefLoader, config }
		, _multiProgramDefLoader{ _progDefLoader, _progDefFromSrcLoader }
		, _multiTextureDefLoader{ _texDefLoader, _texDefFromSrcLoader }
		, _multiMeshDefLoader{ _meshDefLoader, _meshDefFromSrcLoader }
		, _multiProgramLoader{ _programLoader }
		, _multiTextureLoader{ _textureLoader }
		, _multiMeshLoader{ _meshLoader }
		, _multiMaterialLoader{ _materialLoader }
		, _multiArmatureLoader{ _armatureLoader }
	{
		if (config.fallback)
		{
			_multiProgramLoader.addBack(config.fallback->getProgramLoader());
			_multiTextureLoader.addBack(config.fallback->getTextureLoader());
			_multiMeshLoader.addBack(config.fallback->getMeshLoader());
			_multiMaterialLoader.addBack(config.fallback->getMaterialLoader());
			_multiArmatureLoader.addBack(config.fallback->getArmatureLoader());
			_multiTexAtlasLoader.addBack(config.fallback->getTextureAtlasLoader());
			_multiFontLoader.addBack(config.fallback->getFontLoader());
			_multiSkelLoader.addBack(config.fallback->getSkeletonLoader());	
			_multiSkelAnimLoader.addBack(config.fallback->getSkeletalAnimationLoader());
			_multiSkelAnimDefLoader.addBack(config.fallback->getSkeletalAnimatorDefinitionLoader());
			_multiSoundLoader.addBack(config.fallback->getSoundLoader());
			_multiMusicLoader.addBack(config.fallback->getMusicLoader());
		}
	}

	bx::AllocatorI& AssetPack::getAllocator() noexcept
	{
		return _alloc;
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

	IArmatureLoader& AssetPack::getArmatureLoader() noexcept
	{
		return _armatureLoader;
	}

	ITextureAtlasLoader& AssetPack::getTextureAtlasLoader() noexcept
	{
		return _multiTexAtlasLoader;
	}

	ISceneLoader& AssetPack::getSceneLoader() noexcept
	{
		return _sceneLoader;
	}

	IFontLoader& AssetPack::getFontLoader() noexcept
	{
		return _multiFontLoader;
	}

	ISkeletonLoader& AssetPack::getSkeletonLoader() noexcept
	{
		return _multiSkelLoader;
	}

	ISkeletalAnimationLoader& AssetPack::getSkeletalAnimationLoader() noexcept
	{
		return _multiSkelAnimLoader;
	}

	ISkeletalAnimatorDefinitionLoader& AssetPack::getSkeletalAnimatorDefinitionLoader() noexcept
	{
		return _multiSkelAnimDefLoader;
	}

	ISoundLoader& AssetPack::getSoundLoader() noexcept
	{
		return _multiSoundLoader;
	}

	IMusicLoader& AssetPack::getMusicLoader() noexcept
	{
		return _multiMusicLoader;
	}

	namespace
	{
		template<typename T, typename E>
		expected<void, E> expectedVoid(expected<T, E>&& v)
		{
			if (!v)
			{
				return unexpected{ std::move(v).error() };
			}
			return {};
		}
	}

	expected<void, std::string> AssetPack::removeAsset(const std::filesystem::path& path)
	{
		auto& assets = _def.assets();
		auto itr = assets.find(path.string());
		if (itr == assets.end())
		{
			return unexpected{ "path not found" };
		}
		auto& asset = itr->second;
		auto typeId = protobuf::getTypeId(asset);
		if (typeId == protobuf::getTypeId<Mesh::Source>())
		{
			_meshDefFromSrcLoader.clearCache(path);
			_meshLoader.clearCache(path);
			return {};
		}
		if (typeId == protobuf::getTypeId<Material::Definition>())
		{
			_materialLoader.clearCache(path);
			return {};
		}
		return unexpected{ "type loader not found" };
	}

	expected<void, std::string> AssetPack::reloadAsset(const std::filesystem::path& path)
	{
		auto& assets = _def.assets();
		auto itr = assets.find(path.string());
		if (itr == assets.end())
		{
			return unexpected{ "path not found" };
		}
		auto& asset = itr->second;
		auto typeId = protobuf::getTypeId(asset);
		if (typeId == protobuf::getTypeId<Texture::Source>())
		{
			auto result = _texDefFromSrcLoader.reload(path);
			if (!result)
			{
				return unexpected{ result.error() };
			}
			typeId = protobuf::getTypeId<Texture::Definition>();
		}
		if (typeId == protobuf::getTypeId<Texture::Definition>())
		{
			return expectedVoid(_textureLoader.reload(path));
		}
		if (typeId == protobuf::getTypeId<Mesh::Source>())
		{
			auto result = _meshDefFromSrcLoader.reload(path);
			if(!result)
			{
				return unexpected{ result.error() };
			}
			typeId = protobuf::getTypeId<Mesh::Definition>();
		}
		if (typeId == protobuf::getTypeId<Mesh::Definition>())
		{
			return expectedVoid(_meshLoader.reload(path));
		}
		if (typeId == protobuf::getTypeId<Program::Source>())
		{
			auto result = _progDefFromSrcLoader.reload(path);
			if (!result)
			{
				return unexpected{ result.error() };
			}
			typeId = protobuf::getTypeId<Program::Definition>();
		}
		if (typeId == protobuf::getTypeId<Program::Definition>())
		{
			return expectedVoid(_programLoader.reload(path));
		}
		if (typeId == protobuf::getTypeId<Material::Definition>())
		{
			return expectedVoid(_materialLoader.reload(path));
		}
		return unexpected{ "type loader not found" };
	}
}