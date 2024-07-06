#include <bimg/decode.h>
#include <darmok/image.hpp>
#include <darmok/utils.hpp>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/texture.hpp>
#include <stdexcept>

namespace darmok
{
	Image::Image(const DataView& data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format)
		: _container(nullptr)
	{
		if(data.empty())
		{
			throw std::runtime_error("got empty data");
		}

		bx::Error err;
		_container = bimg::imageParse(&alloc, data.ptr(), (uint32_t)data.size(), format, &err);
		checkError(err);
		if (_container == nullptr)
		{
			throw std::runtime_error("got empty image container");
		}
	}

	Image::Image(const Color& color, bx::AllocatorI& alloc, const glm::uvec2& size) noexcept
		: _container(bimg::imageAlloc(
			&alloc, bimg::TextureFormat::RGBA8, size.x, size.y, 0, 1, false, false
		))
	{
		auto c = Colors::toReverseNumber(color);
		bimg::imageSolid(_container->m_data, size.x, size.y, c);
	}

    Image::Image(bimg::ImageContainer* container)
		: _container(container)
	{
		if (_container == nullptr)
		{
			throw std::runtime_error("got empty image container");
		}
	}

	Image::~Image() noexcept
	{
		if (_container != nullptr)
		{
			bimg::imageFree(_container);
		}
	}

	Image::Image(Image&& other) noexcept
		: _container(other._container)
	{
		other._container = nullptr;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		if (_container != nullptr)
		{
			bimg::imageFree(_container);
		}
		_container = other._container;
		other._container = nullptr;
		return *this;
	}

	DataView Image::getData() const noexcept
	{
		return DataView(_container->m_data
			, _container->m_size);
	}

	TextureType Image::getTextureType(uint64_t flags) const noexcept
	{
		if (isCubeMap())
		{
			return TextureType::CubeMap;
		}
		auto depth = getDepth();
		if (1 < depth)
		{
			return TextureType::Texture3D;
		}
		auto format = bgfx::TextureFormat::Enum(getFormat());
		auto layers = getLayerCount();
		auto renderer = bgfx::getCaps()->rendererType;
		if (renderer == bgfx::RendererType::Noop)
		{
			// command line tools
			return TextureType::Texture2D;
		}
		if (bgfx::isTextureValid(depth, false, layers, format, flags))
		{
			return TextureType::Texture2D;
		}
		return TextureType::Unknown;
	}

	bx::AllocatorI& Image::getAllocator() const noexcept
	{
		return *_container->m_allocator;
	}

	bool Image::empty() const noexcept
	{
		return _container == nullptr || _container->m_size == 0;
	}

	void Image::update(const glm::uvec2& pos, DataView data)
	{
		// TODO
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

	TextureConfig Image::getTextureConfig(uint64_t flags) const noexcept
	{
		return {
			getSize(),
			bgfx::TextureFormat::Enum(getFormat()),
			getTextureType(flags),
			uint16_t(getDepth()),
			getMipCount() > 1,
			getLayerCount()
		};
	}

	DataImageLoader::DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc) noexcept
		: _dataLoader(dataLoader)
		, _allocator(alloc)
	{
	}

	std::shared_ptr<Image> DataImageLoader::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		return std::make_shared<Image>(data, _allocator);
	}
}