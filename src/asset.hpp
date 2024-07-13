#pragma once

#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/vertex_layout.hpp>
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

#include <string>
#include <bx/file.h>

namespace darmok
{
	class FileReader final : public bx::FileReader
	{
	public:
		void setBasePath(const std::filesystem::path& basePath) noexcept;
		bool open(const bx::FilePath& filePath, bx::Error* err) override;
	private:
		std::filesystem::path _basePath;
	};

	class FileWriter final : public bx::FileWriter
	{
	public:
		void setBasePath(const std::filesystem::path& basePath) noexcept;
		virtual bool open(const bx::FilePath& filePath, bool append, bx::Error* err) override;
	private:
		std::filesystem::path _basePath;
	};


	using ModelExtLoader = ExtensionLoader<IModelLoader>;
	using SkeletonExtLoader = ExtensionLoader<ISkeletonLoader>;
	using SkeletalAnimationExtLoader = ExtensionLoader<ISkeletalAnimationLoader>;
	using FontExtLoader = ExtensionLoader<IFontLoader>;

	class AssetContextImpl final
	{
	public:
		AssetContextImpl();
		[[nodiscard]] IDataLoader& getDataLoader() noexcept;
		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] IProgramGroupLoader& getProgramGroupLoader() noexcept;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] ColorTextureLoader& getColorTextureLoader() noexcept;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] IModelLoader& getModelLoader() noexcept;
		[[nodiscard]] bx::AllocatorI& getAllocator() noexcept;

#ifdef DARMOK_OZZ
		[[nodiscard]] ISkeletonLoader& getSkeletonLoader() noexcept;
		[[nodiscard]] ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		[[nodiscard]] ISkeletalAnimatorConfigLoader& getSkeletalAnimatorConfigLoader() noexcept;
#endif

#ifdef DARMOK_ASSIMP
		[[nodiscard]] AssimpModelLoader& getAssimpModelLoader() noexcept;
#endif

#ifdef DARMOK_FREETYPE
		[[nodiscard]] IFontLoader& getFontLoader() noexcept;
#endif

		void setBasePath(const std::filesystem::path& path) noexcept;
		void init(App& app);
		void shutdown();
	private:
		FileReader _fileReader;
		FileWriter _fileWriter;
		bx::DefaultAllocator _allocator;
		FileDataLoader _dataLoader;
		DataImageLoader _imageLoader;
		DataProgramLoader _programLoader;
		JsonProgramGroupLoader _programGroupLoader;
		ImageTextureLoader _textureLoader;
		DataVertexLayoutLoader _vertexLayoutLoader;
		TexturePackerTextureAtlasLoader _textureAtlasLoader;
		ColorTextureLoader _colorTextureLoader;
		ModelExtLoader _modelLoader;
		BinaryModelLoader _binModelLoader;
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
	};
}
