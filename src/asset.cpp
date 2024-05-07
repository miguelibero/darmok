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
#ifdef DARMOK_OZZ
		, _skeletonLoader(_dataLoader)
		, _skeletalAnimationLoader(_dataLoader)
#endif
#ifdef DARMOK_ASSIMP 
		, _assimpLoader(_dataLoader, _textureLoader, &_allocator)
#endif
		, _colorTextureLoader(&_allocator)
	{
		setBasePath("assets");
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

#ifdef DARMOK_ASSIMP 
	AssimpSceneLoader& AssetContextImpl::getAssimpLoader() noexcept
	{
		return _assimpLoader;
	}
#endif

	ColorTextureLoader& AssetContextImpl::getColorTextureLoader() noexcept
	{
		return _colorTextureLoader;
	}
	
	ISkeletonLoader& AssetContextImpl::getSkeletonLoader() noexcept
	{
		return _skeletonLoader;
	}

	ISkeletalAnimationLoader& AssetContextImpl::getSkeletalAnimationLoader() noexcept
	{
		return _skeletalAnimationLoader;
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

#ifdef DARMOK_ASSIMP 
	AssimpSceneLoader& AssetContext::getAssimpLoader() noexcept
	{
		return _impl->getAssimpLoader();
	}
#endif

	ColorTextureLoader& AssetContext::getColorTextureLoader() noexcept
	{
		return _impl->getColorTextureLoader();
	}

	ISkeletonLoader& AssetContext::getSkeletonLoader() noexcept
	{
		return _impl->getSkeletonLoader();
	}

	ISkeletalAnimationLoader& AssetContext::getSkeletalAnimationLoader() noexcept
	{
		return _impl->getSkeletalAnimationLoader();
	}

	bx::AllocatorI* AssetContext::getAllocator() noexcept
	{
		return _impl->getAllocator();
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
