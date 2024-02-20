#pragma once

#include <darmok/asset.hpp>

#include <string>

#include <bx/file.h>

#include "image.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "model.hpp"
#include "data.hpp"

namespace darmok
{
	class FileReader final : public bx::FileReader
	{
		typedef bx::FileReader super;
	public:
		void setBasePath(const std::string& basePath);
		bool open(const bx::FilePath& filePath, bx::Error* err) override;
	private:
		std::string _basePath;
	};

	class FileWriter final : public bx::FileWriter
	{
		typedef bx::FileWriter super;
	public:
		void setBasePath(const std::string& basePath);
		virtual bool open(const bx::FilePath& filePath, bool append, bx::Error* err) override;
	private:
		std::string _basePath;
	};

	class AssetContextImpl final
	{
	public:
		AssetContextImpl();
		IImageLoader& getImageLoader() noexcept;
		IProgramLoader& getProgramLoader() noexcept;
		EmbeddedProgramLoader& getEmbeddedProgramLoader() noexcept;
		ITextureLoader& getTextureLoader() noexcept;
		ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		IModelLoader& getModelLoader() noexcept;
		bx::AllocatorI* getAllocator() noexcept;
		void setBasePath(const std::string& path) noexcept;
	private:
		FileReader _fileReader;
		FileWriter _fileWriter;
		bx::DefaultAllocator _allocator;
		FileDataLoader _dataLoader;
		DataImageLoader _imageLoader;
		DataProgramLoader _programLoader;
		EmbeddedProgramLoader _embeddedProgramLoader;
		ImageTextureLoader _textureLoader;
		TexturePackerTextureAtlasLoader _textureAtlasLoader;
		AssimpModelLoader _modelLoader;
	};

}
