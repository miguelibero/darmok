#include "image.hpp"
#include <bimg/decode.h>
#include <darmok/utils.hpp>

namespace darmok
{
    Image::Image(bimg::ImageContainer* container)
		: _container(container)
	{
	}

	Image::~Image()
	{
		bimg::imageFree(_container);
	}

	const bgfx::Memory* Image::makeRef() const
	{
		return bgfx::makeRef(
			_container->m_data
			, _container->m_size
		);
	}

	bool Image::empty() const
	{
		return _container == nullptr || _container->m_size == 0;
	}

	glm::uvec2 Image::getSize() const
	{
		return glm::uvec2(_container->m_width, _container->m_height);
	}

	uint32_t Image::getDepth() const
	{
		return _container->m_depth;
	}

	bool Image::isCubeMap() const
	{
		return _container->m_cubeMap;
	}

	uint8_t Image::getMipCount() const
	{
		return _container->m_numMips;
	}

	uint16_t Image::getLayerCount() const
	{
		return _container->m_numLayers;
	}

	bimg::TextureFormat::Enum Image::getFormat() const
	{
		return _container->m_format;
	}

	bgfx::TextureInfo Image::getTextureInfo() const
	{
		bgfx::TextureInfo info;
		bgfx::calcTextureSize(
			info
			, uint16_t(_container->m_width)
			, uint16_t(_container->m_height)
			, uint16_t(_container->m_depth)
			, _container->m_cubeMap
			, 1 < _container->m_numMips
			, _container->m_numLayers
			, bgfx::TextureFormat::Enum(_container->m_format)
		);
		return info;
	}


	DataImageLoader::DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI* alloc)
		: _dataLoader(dataLoader)
		, _allocator(alloc)
	{
	}

	std::shared_ptr<Image> DataImageLoader::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		if (data == nullptr || data->empty())
		{
			throw std::runtime_error("got empty data");
		}

		bx::Error err;
		bimg::ImageContainer* container = bimg::imageParse(_allocator, data->ptr(), (uint32_t)data->size(), bimg::TextureFormat::Count, &err);
		checkError(err);
		if (container == nullptr)
		{
			throw std::runtime_error("got empty image container");
		}
		return std::make_shared<Image>(container);
	}

}