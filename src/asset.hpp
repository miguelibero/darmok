#pragma once

#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program.hpp>
#include <darmok/data.hpp>
#include <darmok/loader.hpp>
#include <darmok/text.hpp>

#ifdef DARMOK_OZZ
#include <darmok/skeleton.hpp>
#include <darmok/skeleton_ozz.hpp>
#endif

#ifdef DARMOK_ASSIMP
#include <darmok/model_assimp.hpp>
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
	using ModelExtLoader = ExtensionLoader<IModelLoader>;
	using SkeletonExtLoader = ExtensionLoader<ISkeletonLoader>;
	using SkeletalAnimationExtLoader = ExtensionLoader<ISkeletalAnimationLoader>;
	using FontExtLoader = ExtensionLoader<IFontLoader>;

	class AssetContextImpl final
	{
	public:
		AssetContextImpl();
		IDataLoader& getDataLoader() noexcept;
		IImageLoader& getImageLoader() noexcept;
		IProgramLoader& getProgramLoader() noexcept;
		ITextureLoader& getTextureLoader() noexcept;
		ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		IMeshLoader& getMeshLoader() noexcept;
		IModelLoader& getModelLoader() noexcept;
		bx::AllocatorI& getAllocator() noexcept;

#ifdef DARMOK_OZZ
		ISkeletonLoader& getSkeletonLoader() noexcept;
		ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		ISkeletalAnimatorConfigLoader& getSkeletalAnimatorConfigLoader() noexcept;
#endif

#ifdef DARMOK_ASSIMP
		AssimpModelLoader& getAssimpModelLoader() noexcept;
#endif

#ifdef DARMOK_FREETYPE
		IFontLoader& getFontLoader() noexcept;
#endif

#ifdef DARMOK_MINIAUDIO
		ISoundLoader& getSoundLoader() noexcept;
		IMusicLoader& getMusicLoader() noexcept;
#endif

		void addBasePath(const std::filesystem::path& path) noexcept;
		bool removeBasePath(const std::filesystem::path& path) noexcept;
		void init(App& app);
		void update();
		void shutdown();
	private:
		bx::DefaultAllocator _allocator;
		DataLoader _dataLoader;
		ImageLoader _imageLoader;
		ProgramDefinitionLoader _programDefLoader;
		ProgramLoader _programLoader;
		TextureDefinitionLoader _textureDefLoader;
		TextureLoader _textureLoader;
		MeshDefinitionLoader _meshDefLoader;
		MeshLoader _meshLoader;
		TexturePackerTextureAtlasLoader _textureAtlasLoader;
		ModelExtLoader _modelLoader;
		ModelLoader _binModelLoader;
		TextureAtlasFontLoader _textureAtlasFontLoader;
		FontExtLoader _fontLoader;

#ifdef DARMOK_OZZ
		SkeletonExtLoader _skeletonLoader;
		SkeletalAnimationExtLoader _skeletalAnimationLoader;
		SkeletalAnimatorConfigLoader _skeletalAnimatorConfigLoader;

		OzzSkeletonLoader _ozzSkeletonLoader;
		OzzSkeletalAnimationLoader _ozzSkeletalAnimationLoader;
#endif

#ifdef DARMOK_ASSIMP
		AssimpModelLoader _assimpModelLoader;
#endif

#ifdef DARMOK_FREETYPE
		FreetypeFontLoader _freetypeFontLoader;
#endif

#ifdef DARMOK_MINIAUDIO
		MiniaudioSoundLoader _miniaudioSoundLoader;
		MiniaudioMusicLoader _miniaudioMusicLoader;
#endif
	};
}
