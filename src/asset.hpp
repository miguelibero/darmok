#pragma once

#include <darmok/asset.hpp>

#include <string>

#include <bx/file.h>

#include "image.hpp"
#include "texture.hpp"
#include "program.hpp"
#include "model.hpp"
#include "data.hpp"
#include "vertex.hpp"

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
		[[nodiscard]] IImageLoader& getImageLoader() noexcept;
		[[nodiscard]] IProgramLoader& getProgramLoader() noexcept;
		[[nodiscard]] ITextureLoader& getTextureLoader() noexcept;
		[[nodiscard]] ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		[[nodiscard]] IModelLoader& getModelLoader() noexcept;
		[[nodiscard]] IVertexLayoutLoader& getVertexLayoutLoader() noexcept;

		bx::AllocatorI* getAllocator() noexcept;
		void setBasePath(const std::string& path) noexcept;
	private:
		FileReader _fileReader;
		FileWriter _fileWriter;
		bx::DefaultAllocator _allocator;
		FileDataLoader _dataLoader;
		DataImageLoader _imageLoader;
		DataProgramLoader _programLoader;
		ImageTextureLoader _textureLoader;
		TexturePackerTextureAtlasLoader _textureAtlasLoader;
		AssimpModelLoader _modelLoader;
		XmlDataVertexLayoutLoader _vertexLayoutLoader;
	};

}
