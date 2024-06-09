#pragma once

#include <darmok/asset.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/model.hpp>
#include <darmok/vertex_layout.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
#include <darmok/program.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/data.hpp>

#include <string>
#include <bx/file.h>

#ifdef DARMOK_OZZ
#include "skeleton_ozz.hpp"
#endif

namespace darmok
{
	class FileReader final : public bx::FileReader
	{
		typedef bx::FileReader super;
	public:
		void setBasePath(const std::string& basePath) noexcept;
		bool open(const bx::FilePath& filePath, bx::Error* err) override;
	private:
		std::string _basePath;
	};

	class FileWriter final : public bx::FileWriter
	{
		typedef bx::FileWriter super;
	public:
		void setBasePath(const std::string& basePath) noexcept;
		virtual bool open(const bx::FilePath& filePath, bool append, bx::Error* err) override;
	private:
		std::string _basePath;
	};

	class AssetContextImpl final
	{
	public:
		AssetContextImpl();
		[[nodiscard]] IDataLoader& getDataLoader() noexcept;
		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] StandardProgramLoader& getStandardProgramLoader() noexcept;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] ColorTextureLoader& getColorTextureLoader() noexcept;
		[[nodiscard]] ISkeletonLoader& getSkeletonLoader() noexcept;
		[[nodiscard]] IModelLoader& getModelLoader() noexcept;
		[[nodiscard]] bx::AllocatorI& getAllocator() noexcept;

#ifdef DARMOK_OZZ
		[[nodiscard]] ISkeletalAnimationLoader& getSkeletalAnimationLoader() noexcept;
		[[nodiscard]] ISkeletalAnimatorConfigLoader& getSkeletalAnimatorConfigLoader() noexcept;
#endif
		void setBasePath(const std::string& path) noexcept;
	private:
		FileReader _fileReader;
		FileWriter _fileWriter;
		bx::DefaultAllocator _allocator;
		FileDataLoader _dataLoader;
		DataImageLoader _imageLoader;
		DataProgramLoader _programLoader;
		StandardProgramLoader _standardProgramLoader;
		ImageTextureLoader _textureLoader;
		BinaryVertexLayoutLoader _vertexLayoutLoader;
		TexturePackerTextureAtlasLoader _textureAtlasLoader;
		ColorTextureLoader _colorTextureLoader;
#ifdef DARMOK_OZZ
		OzzSkeletonLoader _skeletonLoader;
		OzzSkeletalAnimationLoader _skeletalAnimationLoader;
#endif
		JsonSkeletalAnimatorConfigLoader _skeletalAnimatorConfigLoader;
		BinaryModelLoader _modelLoader;
	};
}
