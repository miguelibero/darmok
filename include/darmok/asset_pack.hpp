#pragma once

#include <darmok/export.h>
#include <darmok/asset.hpp>
#include <darmok/protobuf/asset.pb.h>
#include <darmok/protobuf.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/audio.hpp>
#include <darmok/text.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
	template<class Interface>
	class DARMOK_EXPORT AssetPackLoader final : public Interface
	{
	private:
		OptionalRef<const protobuf::AssetPack> _assets;
		std::unordered_set<std::filesystem::path> _loadedPaths;
	public:
		using Result = Interface::Result;
		using Resource = Interface::Resource;

		AssetPackLoader(const protobuf::AssetPack& assets)
			: _assets{ assets }
		{
		}

		[[nodiscard]] Result operator()(std::filesystem::path path) noexcept override
		{
			auto& assets = _assets->assets();
			auto itr = assets.find(path.string());
			if (itr == assets.end())
			{
				return unexpected{ "asset pack path not found" };
			}
			auto def = std::make_shared<Resource>();
			if (!itr->second.UnpackTo(def.get()))
			{
				return unexpected{ "failed to unpack asset protobuf" };
			}
			return def;
		}
	};

	struct DARMOK_EXPORT AssetPackConfig final
	{
		ProgramCompilerConfig programCompilerConfig;
		OptionalRef<IAssetContext> fallback;
	};

	class DARMOK_EXPORT AssetPack final : public IAssetContext
	{
	public:
		using Definition = protobuf::AssetPack;
		AssetPack(const Definition& def, const AssetPackConfig& config);

		[[nodiscard]] bx::AllocatorI& getAllocator() noexcept override;
		[[nodiscard]] IProgramFromDefinitionLoader& getProgramLoader() noexcept override;
		[[nodiscard]] ITextureFromDefinitionLoader& getTextureLoader() noexcept override;
		[[nodiscard]] IMeshFromDefinitionLoader& getMeshLoader() noexcept override;
		[[nodiscard]] IMaterialFromDefinitionLoader& getMaterialLoader() noexcept override;
		[[nodiscard]] IArmatureFromDefinitionLoader& getArmatureLoader() noexcept override;
		[[nodiscard]] ITextureAtlasFromDefinitionLoader& getTextureAtlasLoader() noexcept override;
		[[nodiscard]] ISceneLoader& getSceneLoader() noexcept override;
		[[nodiscard]] ISceneDefinitionLoader& getSceneDefinitionLoader() noexcept override;
		[[nodiscard]] IFontLoader& getFontLoader() noexcept override;
		[[nodiscard]] ISkeletonLoader& getSkeletonLoader() noexcept override;
		[[nodiscard]] ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept override;
		[[nodiscard]] ISkeletalAnimatorDefinitionLoader& getSkeletalAnimatorDefinitionLoader() noexcept override;
		[[nodiscard]] ISoundLoader& getSoundLoader() noexcept override;
		[[nodiscard]] IMusicLoader& getMusicLoader() noexcept override;

		expected<void, std::string> reloadAsset(const std::filesystem::path& path);
		expected<void, std::string> removeAsset(const std::filesystem::path& path);
	private:
		OptionalRef<const Definition> _def;
		OptionalRef<bx::AllocatorI> _alloc;
		bx::DefaultAllocator _defaultAlloc;

		AssetPackLoader<IProgramDefinitionLoader> _progDefLoader;
		AssetPackLoader<ITextureDefinitionLoader> _texDefLoader;
		AssetPackLoader<IMeshDefinitionLoader> _meshDefLoader;
		AssetPackLoader<IMaterialDefinitionLoader> _matDefLoader;
		AssetPackLoader<IArmatureDefinitionLoader> _armDefLoader;
		AssetPackLoader<ISceneDefinitionLoader> _sceneDefLoader;

		AssetPackLoader<IProgramSourceLoader> _progSrcLoader;
		ProgramDefinitionFromSourceLoader _progDefFromSrcLoader;
		AssetPackLoader<ITextureSourceLoader> _texSrcLoader;
		TextureDefinitionFromSourceLoader _texDefFromSrcLoader;
		AssetPackLoader<IMeshSourceLoader> _meshSrcLoader;
		MeshDefinitionFromSourceLoader _meshDefFromSrcLoader;

		MultiLoader<IProgramDefinitionLoader> _multiProgramDefLoader;
		MultiLoader<ITextureDefinitionLoader> _multiTextureDefLoader;
		MultiLoader<IMeshDefinitionLoader> _multiMeshDefLoader;

		MultiFromDefinitionLoader<IProgramFromDefinitionLoader> _multiProgramLoader;
		MultiFromDefinitionLoader<ITextureFromDefinitionLoader> _multiTextureLoader;
		MultiFromDefinitionLoader<IMeshFromDefinitionLoader> _multiMeshLoader;
		MultiFromDefinitionLoader<IMaterialFromDefinitionLoader> _multiMaterialLoader;
		MultiFromDefinitionLoader<IArmatureFromDefinitionLoader> _multiArmatureLoader;
		MultiFromDefinitionLoader<ITextureAtlasFromDefinitionLoader> _multiTexAtlasLoader;
		MultiLoader<IFontLoader> _multiFontLoader;
		MultiLoader<ISkeletonLoader> _multiSkelLoader;
		MultiLoader<ISkeletalAnimationLoader> _multiSkelAnimLoader;
		MultiLoader<ISkeletalAnimatorDefinitionLoader> _multiSkelAnimDefLoader;
		MultiLoader<ISoundLoader> _multiSoundLoader;
		MultiLoader<IMusicLoader> _multiMusicLoader;

		ProgramLoader _programLoader;
		TextureLoader _textureLoader;
		MeshLoader _meshLoader;
		MaterialLoader _materialLoader;
		ArmatureLoader _armatureLoader;
		SceneLoader _sceneLoader;
	};
}