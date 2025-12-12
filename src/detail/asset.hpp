#pragma once

#include <darmok/asset.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/data.hpp>
#include <darmok/loader.hpp>
#include <darmok/text.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/audio.hpp>

#ifdef DARMOK_OZZ
#include <darmok/skeleton_ozz.hpp>
#endif

#ifdef DARMOK_ASSIMP
#include <darmok/scene_assimp.hpp>
#ifdef DARMOK_OZZ
#include <darmok/skeleton_assimp.hpp>
#endif
#endif

#ifdef DARMOK_FREETYPE
#include <darmok/text_freetype.hpp>
#endif

#ifdef DARMOK_MINIAUDIO
#include <darmok/audio_mini.hpp>
#endif

#include <string>
#include <vector>

namespace darmok
{
	class IAssetContext;

	class AssetContextImpl final
	{
	public:
		using Config = AssetContextConfig;
		AssetContextImpl(IAssetContext& assets, Config&& config);
		IDataLoader& getDataLoader() noexcept;
		IImageLoader& getImageLoader() noexcept;
		IProgramFromDefinitionLoader& getProgramLoader() noexcept;
		ITextureFromDefinitionLoader& getTextureLoader() noexcept;
		ITextureAtlasFromDefinitionLoader& getTextureAtlasLoader() noexcept;
		IMaterialFromDefinitionLoader& getMaterialLoader() noexcept;
		IMeshFromDefinitionLoader& getMeshLoader() noexcept;
		IArmatureFromDefinitionLoader& getArmatureLoader() noexcept;
		IFontLoader& getFontLoader() noexcept;
		ISceneDefinitionLoader& getSceneDefinitionLoader() noexcept;
		bx::AllocatorI& getAllocator() noexcept;
		ISkeletonLoader& getSkeletonLoader() noexcept;
		ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		ISkeletalAnimatorDefinitionLoader& getSkeletalAnimatorDefinitionLoader() noexcept;
		ISoundLoader& getSoundLoader() noexcept;
		IMusicLoader& getMusicLoader() noexcept;

		expected<void, std::string> init(App& app) noexcept;
		expected<void, std::string> update() noexcept;
		expected<void, std::string> shutdown() noexcept;
	private:
		Config _config;
		ImageLoader _imageLoader;
		DataProgramDefinitionLoader _dataProgDefLoader;
		ProgramLoader _progLoader;
		DataTextureDefinitionLoader _dataTexDefLoader;
		ImageTextureDefinitionLoader _imgTexDefLoader;
		MultiLoader<ITextureDefinitionLoader> _texDefLoader;
		TextureLoader _texLoader;
		DataMaterialDefinitionLoader _dataMatDefLoader;
		MaterialLoader _materialLoader;
		DataMeshDefinitionLoader _dataMeshDefLoader;
		MeshLoader _meshLoader;
		DataArmatureDefinitionLoader _dataArmDefLoader;
		ArmatureLoader _armatureLoader;
		DataTextureAtlasDefinitionLoader _dataTexAtlasDefLoader;
		TexturePackerDefinitionLoader _texPackerDefLoader;
		MultiLoader<ITextureAtlasDefinitionLoader> _texAtlasDefLoader;
		TextureAtlasLoader _texAtlasLoader;
		TextureAtlasFontLoader _texAtlasFontLoader;
		MultiLoader<IFontLoader> _fontLoader;
		MultiLoader<ISceneDefinitionLoader> _sceneDefLoader;
		DataSceneDefinitionLoader _dataSceneDefLoader;

#ifdef DARMOK_ASSIMP
        AssimpSceneDefinitionLoader _assimpSceneDefLoader;
#endif

		MultiLoader<ISkeletonLoader> _skelLoader;
		MultiLoader<ISkeletalAnimationLoader> _skelAnimLoader;
		DataSkeletalAnimatorDefinitionLoader _dataSkelAnimDefLoader;

#ifdef DARMOK_OZZ
		OzzSkeletonLoader _ozzSkeletonLoader;
		OzzSkeletalAnimationLoader _ozzSkeletalAnimationLoader;
#endif

#ifdef DARMOK_FREETYPE
		FreetypeFontDefinitionLoader _freetypeFontDefLoader;
		FreetypeFontLoader _freetypeFontLoader;
#endif

#ifdef DARMOK_MINIAUDIO
		MiniaudioSoundLoader _miniaudioSoundLoader;
		MiniaudioMusicLoader _miniaudioMusicLoader;
#else
		EmptyLoader<ISoundLoader> _emptySoundLoader;
		EmptyLoader<IMusicLoader> _emptyMusicLoader;
#endif
	};
}
