#include "image.hpp"
#include <bimg/decode.h>
#include <darmok/utils.hpp>
#include <darmok/color.hpp>

namespace darmok
{
    Image::Image(bimg::ImageContainer* container) noexcept
		: _container(container)
	{
	}

	Image::~Image() noexcept
	{
		bimg::imageFree(_container);
	}

	const bgfx::Memory* Image::makeRef() const noexcept
	{
		return bgfx::makeRef(
			_container->m_data
			, _container->m_size
		);
	}

	std::shared_ptr<Image> Image::create(bx::AllocatorI* alloc, const Color& color, const glm::uvec2& size) noexcept
	{
		auto container = bimg::imageAlloc(
			alloc, bimg::TextureFormat::RGBA8, size.x, size.y, 0, 1, false, false
		);
		bimg::imageSolid(container->m_data, size.x, size.y, Colors::toNumber(color));
		return std::make_shared<Image>(container);
	}

	bool Image::empty() const noexcept
	{
		return _container == nullptr || _container->m_size == 0;
	}

	glm::uvec2 Image::getSize() const noexcept
	{
		return glm::uvec2(_container->m_width, _container->m_height);
	}

	uint32_t Image::getDepth() const noexcept
	{
		return _container->m_depth;
	}

	bool Image::isCubeMap() const noexcept
	{
		return _container->m_cubeMap;
	}

	uint8_t Image::getMipCount() const noexcept
	{
		return _container->m_numMips;
	}

	uint16_t Image::getLayerCount() const noexcept
	{
		return _container->m_numLayers;
	}

	bimg::TextureFormat::Enum Image::getFormat() const noexcept
	{
		return _container->m_format;
	}

	bgfx::TextureInfo Image::getTextureInfo() const noexcept
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

	DataImageLoader::DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI* alloc) noexcept
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