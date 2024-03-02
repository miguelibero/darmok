#include "asset.hpp"

#include <filesystem>
#include <bx/filepath.h>

namespace darmok
{
	static std::string addBasePath(const std::string& path, const std::string& basePath)
	{
		std::filesystem::path p(path);

		if (p.is_absolute())
		{
			return path;
		}
		return (std::filesystem::path(basePath) / p).string();
	}

	void FileReader::setBasePath(const std::string& basePath)
	{
		_basePath = basePath;
	}

	bool FileReader::open(const bx::FilePath& filePath, bx::Error* err)
	{
		auto absFilePath = addBasePath(filePath.getCPtr(), _basePath);
		return super::open(absFilePath.c_str(), err);
	}

	void FileWriter::setBasePath(const std::string& basePath)
	{
		_basePath = basePath;
	}

	bool FileWriter::open(const bx::FilePath& filePath, bool append, bx::Error* err)
	{
		auto absFilePath = addBasePath(filePath.getCPtr(), _basePath);
		return super::open(absFilePath.c_str(), append, err);
	}

	AssetContextImpl::AssetContextImpl()
		: _dataLoader(&_fileReader, &_allocator)
		, _imageLoader(_dataLoader, &_allocator)
		, _programLoader(_dataLoader)
		, _textureLoader(_imageLoader)
		, _textureAtlasLoader(_dataLoader, _textureLoader)
		, _modelLoader(_dataLoader, _textureLoader)
		, _vertexLayoutLoader(_dataLoader)
	{
	}

	bx::AllocatorI* AssetContextImpl::getAllocator() noexcept
	{
		return &_allocator;
	}
	
	IImageLoader& AssetContextImpl::getImageLoader() noexcept
	{
		return _imageLoader;
	}

	IProgramLoader& AssetContextImpl::getProgramLoader() noexcept
	{
		return _programLoader;
	}

	ITextureLoader& AssetContextImpl::getTextureLoader() noexcept
	{
		return _textureLoader;
	}

	ITextureAtlasLoader& AssetContextImpl::getTextureAtlasLoader() noexcept
	{
		return _textureAtlasLoader;
	}

	IModelLoader& AssetContextImpl::getModelLoader() noexcept
	{
		return _modelLoader;
	}

	IVertexLayoutLoader& AssetContextImpl::getVertexLayoutLoader() noexcept
	{
		return _vertexLayoutLoader;
	}

	void AssetContextImpl::setBasePath(const std::string& path) noexcept
	{
		_fileReader.setBasePath(path);
		_fileWriter.setBasePath(path);
	}

	AssetContext& AssetContext::get() noexcept
	{
		static AssetContext instance;
		return instance;
	}

	AssetContext::AssetContext() noexcept
		: _impl(std::make_unique<AssetContextImpl>())
	{
	}

	IImageLoader& AssetContext::getImageLoader() noexcept
	{
		return _impl->getImageLoader();
	}

	IProgramLoader& AssetContext::getProgramLoader() noexcept
	{
		return _impl->getProgramLoader();
	}

	ITextureLoader& AssetContext::getTextureLoader() noexcept
	{
		return _impl->getTextureLoader();
	}

	ITextureAtlasLoader& AssetContext::getTextureAtlasLoader() noexcept
	{
		return _impl->getTextureAtlasLoader();
	}

	IModelLoader& AssetContext::getModelLoader() noexcept
	{
		return _impl->getModelLoader();
	}

	IVertexLayoutLoader& AssetContext::getVertexLayoutLoader() noexcept
	{
		return _impl->getVertexLayoutLoader();
	}

	AssetContextImpl& AssetContext::getImpl() noexcept
	{
		return *_impl;
	}

	const AssetContextImpl& AssetContext::getImpl() const noexcept
	{
		return *_impl;
	}
}
