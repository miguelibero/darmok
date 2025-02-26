#pragma once

#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
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
	class AssetContextImpl final
	{
	public:
		AssetContextImpl();
		IDataLoader& getDataLoader() noexcept;
		IImageLoader& getImageLoader() noexcept;
		IProgramLoader& getProgramLoader() noexcept;
		ITextureLoader& getTextureLoader() noexcept;
		ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		IMaterialLoader& getMaterialLoader() noexcept;
		IMeshLoader& getMeshLoader() noexcept;
		IModelLoader& getModelLoader() noexcept;
		IFontLoader& getFontLoader() noexcept;

		bx::AllocatorI& getAllocator() noexcept;

#ifdef DARMOK_OZZ
		ISkeletonLoader& getSkeletonLoader() noexcept;
		ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		ISkeletalAnimatorDefinitionLoader& getSkeletalAnimatorDefinitionLoader() noexcept;
#endif

#ifdef DARMOK_ASSIMP
		AssimpModelLoader& getAssimpModelLoader() noexcept;
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
		ProgramDefinitionLoader _cerealProgDefLoader;
		ProgramLoader _progLoader;
		CerealTextureDefinitionLoader _cerealTexDefLoader;
		ImageTextureDefinitionLoader _imgTexDefLoader;
		ContainerLoader<ITextureDefinitionLoader> _texDefLoader;
		TextureLoader _texLoader;
		CerealMaterialDefinitionLoader _cerealMatDefLoader;
		MaterialLoader _materialLoader;
		CerealMeshDefinitionLoader _cerealMeshDefLoader;
		MeshLoader _meshLoader;
		CerealTextureAtlasDefinitionLoader _cerealTexAtlasDefLoader;
		TexturePackerDefinitionLoader _texPackerDefLoader;
		ContainerLoader<ITextureAtlasDefinitionLoader> _texAtlasDefLoader;
		TextureAtlasLoader _texAtlasLoader;
		ContainerLoader<IModelLoader> _modelLoader;
		CerealModelLoader _cerealModelLoader;
		TextureAtlasFontLoader _texAtlasFontLoader;
		ContainerLoader<IFontLoader> _fontLoader;

#ifdef DARMOK_OZZ
		ContainerLoader<ISkeletonLoader> _skelLoader;
		ContainerLoader<ISkeletalAnimationLoader> _skelAnimLoader;
		SkeletalAnimatorDefinitionLoader _skelAnimDefLoader;

		OzzSkeletonLoader _ozzSkeletonLoader;
		OzzSkeletalAnimationLoader _ozzSkeletalAnimationLoader;
#endif

#ifdef DARMOK_ASSIMP
		AssimpModelLoader _assimpModelLoader;
#endif

#ifdef DARMOK_FREETYPE
		DataFreetypeFontDefinitionLoader _dataFreetypeFontDefLoader;
		CerealFreetypeFontDefinitionLoader _cerealFreetypeFontDefLoader;
		ContainerLoader<IFreetypeFontDefinitionLoader> _freetypeFontDefLoader;
		FreetypeFontLoader _freetypeFontLoader;
#endif

#ifdef DARMOK_MINIAUDIO
		MiniaudioSoundLoader _miniaudioSoundLoader;
		MiniaudioMusicLoader _miniaudioMusicLoader;
#endif
	};
}
