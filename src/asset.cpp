#include "asset.hpp"

#include <filesystem>
#include <bx/filepath.h>

namespace darmok
{
	static std::string addBasePath(const std::string& path, const std::string& basePath) noexcept
	{
		std::filesystem::path p(path);

		if (p.is_absolute())
		{
			return path;
		}
		return (std::filesystem::path(basePath) / p).string();
	}

	void FileReader::setBasePath(const std::string& basePath) noexcept
	{
		_basePath = basePath;
	}

	bool FileReader::open(const bx::FilePath& filePath, bx::Error* err)
	{
		auto absFilePath = addBasePath(filePath.getCPtr(), _basePath);
		return super::open(absFilePath.c_str(), err);
	}

	void FileWriter::setBasePath(const std::string& basePath) noexcept
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
		, _vertexLayoutLoader(_dataLoader)
		, _programLoader(_dataLoader, _vertexLayoutLoader)
		, _textureLoader(_imageLoader)
		, _textureAtlasLoader(_dataLoader, _textureLoader)
		, _modelLoader(_dataLoader, _textureLoader, &_allocator)
		, _colorTextureLoader(&_allocator)
	{
	}

	bx::AllocatorI* AssetContextImpl::getAllocator() noexcept
	{
		return &_allocator;
	}

	[[nodiscard]] IDataLoader& AssetContextImpl::getDataLoader() noexcept
	{
		return _dataLoader;
	}
	
	IImageLoader& AssetContextImpl::getImageLoader() noexcept
	{
		return _imageLoader;
	}

	IProgramLoader& AssetContextImpl::getProgramLoader() noexcept
	{
		return _programLoader;
	}

	StandardProgramLoader& AssetContextImpl::getStandardProgramLoader() noexcept
	{
		return _standardProgramLoader;
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

	ColorTextureLoader& AssetContextImpl::getColorTextureLoader() noexcept
	{
		return _colorTextureLoader;
	}

	void AssetContextImpl::setBasePath(const std::string& path) noexcept
	{
		_fileReader.setBasePath(path);
		_fileWriter.setBasePath(path);
	}

	AssetContext::AssetContext() noexcept
		: _impl(std::make_unique<AssetContextImpl>())
	{
	}

	AssetContext::~AssetContext() noexcept
	{
	}

	IDataLoader& AssetContext::getDataLoader() noexcept
	{
		return _impl->getDataLoader();
	}

	IImageLoader& AssetContext::getImageLoader() noexcept
	{
		return _impl->getImageLoader();
	}

	IProgramLoader& AssetContext::getProgramLoader() noexcept
	{
		return _impl->getProgramLoader();
	}

	StandardProgramLoader& AssetContext::getStandardProgramLoader() noexcept
	{
		return _impl->getStandardProgramLoader();
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

	ColorTextureLoader& AssetContext::getColorTextureLoader() noexcept
	{
		return _impl->getColorTextureLoader();
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
