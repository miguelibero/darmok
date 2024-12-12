#include <darmok/image.hpp>
#include <darmok/utils.hpp>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>

#include <stdexcept>
#include <bx/readerwriter.h>
#include <bimg/decode.h>

namespace darmok
{
	Image::Image(DataView data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format)
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

	Image::Image(const std::array<DataView, 6>& faceData, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format)
	{
		auto right	= Image(faceData[0], alloc, format);
		auto left	= Image(faceData[1], alloc, format);
		auto top	= Image(faceData[2], alloc, format);
		auto bottom = Image(faceData[3], alloc, format);
		auto front	= Image(faceData[4], alloc, format);
		auto back	= Image(faceData[5], alloc, format);

		auto fformat = right.getFormat();

		if (left.getFormat() != fformat || top.getFormat() != fformat || bottom.getFormat() != fformat
			|| front.getFormat() != fformat || back.getFormat() != fformat)
		{
			throw new std::runtime_error("all faces should have the same format");
		}

		auto size = right.getSize();
		if (left.getSize() != size || top.getSize() != size || bottom.getSize() != size
			|| front.getSize() != size || back.getSize() != size)
		{
			throw new std::runtime_error("all faces should have the same size");
		}

		_container = bimg::imageAlloc(&alloc, fformat, size.x, size.y, 1, 1, true, false);

		// Copy each face into the cubemap
		auto ptr = (uint8_t*)_container->m_data;
		auto memSize = right.getData().size();
		std::memcpy(ptr + memSize * 0, right.getData().ptr(), memSize);
		std::memcpy(ptr + memSize * 1, left.getData().ptr(), memSize);
		std::memcpy(ptr + memSize * 2, top.getData().ptr(), memSize);
		std::memcpy(ptr + memSize * 3, bottom.getData().ptr(), memSize);
		std::memcpy(ptr + memSize * 4, front.getData().ptr(), memSize);
		std::memcpy(ptr + memSize * 5, back.getData().ptr(), memSize);
	}

	Image::Image(const Color& color, bx::AllocatorI& alloc, const glm::uvec2& size) noexcept
		: Image(size, alloc, bimg::TextureFormat::RGBA8)
	{
		auto c = Colors::toReverseNumber(color);
		bimg::imageSolid(_container->m_data, size.x, size.y, c);
	}

	Image::Image(const glm::uvec2& size, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format)
		: _container(bimg::imageAlloc(
			&alloc, format, size.x, size.y, 0, 1, false, false
		))
	{
		std::memset(_container->m_data, 0, _container->m_size);
	}

	Image::Image(const Image& other) noexcept
		: _container(nullptr)
	{
		copyContainer(other);
	}

	Image& Image::operator=(const Image& other) noexcept
	{
		if (_container != nullptr)
		{
			bimg::imageFree(_container);
		}

		copyContainer(other);
		return *this;
	}

	void Image::copyContainer(const Image& other) noexcept
	{
		auto size = other.getSize();
		auto format = other.getFormat();

		_container = bimg::imageAlloc(
			other._container->m_allocator, format, size.x, size.y, 0, 1, false, false
		);

		auto info = other.getTextureInfo();
		auto bpp = info.bitsPerPixel / 8;
		uint32_t pitch = bpp * size.x;
		bimg::imageCopy(_container->m_data, size.x, size.y, info.depth, info.bitsPerPixel, pitch, other._container->m_data);
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

	void Image::encode(ImageEncoding encoding, bx::WriterI& writer) const noexcept
	{
		auto size = getSize();
		auto info = getTextureInfo();
		auto bpp = info.bitsPerPixel / 8;
		uint32_t pitch = bpp * size.x;
		auto ptr = _container->m_data;
		auto format = _container->m_format;
		auto ori = _container->m_orientation;
		auto yflip = ori == bimg::Orientation::HFlip || ori == bimg::Orientation::HFlipR90 || ori == bimg::Orientation::HFlipR270;
		bool srgb = format == bimg::TextureFormat::RGB8S;
		bx::Error err;
		
		switch (encoding)
		{
		case ImageEncoding::Tga:
			bimg::imageWriteTga(&writer, size.x, size.y, pitch, ptr, bpp == 1, yflip, &err);
			break;
		case ImageEncoding::Png:
			bimg::imageWritePng(&writer, size.x, size.y, pitch, ptr, format, yflip, &err);
			break;
		case ImageEncoding::Exr:
			bimg::imageWriteExr(&writer, size.x, size.y, pitch, ptr, format, yflip, &err);
			break;
		case ImageEncoding::Hdr:
			bimg::imageWriteHdr(&writer, size.x, size.y, pitch, ptr, format, yflip, &err);
			break;
		case ImageEncoding::Dds:
			bimg::imageWriteDds(&writer, *_container, ptr, info.storageSize, &err);
			break;
		case ImageEncoding::Ktx:
			bimg::imageWriteKtx(&writer, format, isCubeMap(), size.x, size.y, getDepth(), getMipCount(), getLayerCount(), srgb, ptr, &err);
			break;
		}
		checkError(err);
	}

	Data Image::encode(ImageEncoding encoding) const noexcept
	{
		Data data;
		DataMemoryBlock block(data);
		bx::MemoryWriter writer(&block);
		encode(encoding, writer);
		return data;
	}

	const std::unordered_map<bimg::TextureFormat::Enum, std::string> Image::_formatNames = {
		{ bimg::TextureFormat::BC1, "bc1" },
		{ bimg::TextureFormat::BC2, "bc2" },
		{ bimg::TextureFormat::BC3, "bc3" },
		{ bimg::TextureFormat::BC4, "bc4" },
		{ bimg::TextureFormat::BC5, "bc5" },
		{ bimg::TextureFormat::BC6H, "bc6h" },
		{ bimg::TextureFormat::BC7, "bc7" },
		{ bimg::TextureFormat::ETC1, "etc1" },
		{ bimg::TextureFormat::ETC2, "etc2" },
		{ bimg::TextureFormat::ETC2A, "etc2a" },
		{ bimg::TextureFormat::ETC2A1, "etc2a1" },
		{ bimg::TextureFormat::PTC12, "ptc12" },
		{ bimg::TextureFormat::PTC14, "ptc14" },
		{ bimg::TextureFormat::PTC12A, "ptc12a" },
		{ bimg::TextureFormat::PTC14A, "ptc14a" },
		{ bimg::TextureFormat::PTC22, "ptc22" },
		{ bimg::TextureFormat::PTC24, "ptc24" },
		{ bimg::TextureFormat::ATC, "atc" },
		{ bimg::TextureFormat::ATCE, "atce" },
		{ bimg::TextureFormat::ATCI, "atci" },
		{ bimg::TextureFormat::ASTC4x4, "astc4x4" },
		{ bimg::TextureFormat::ASTC5x4, "astc5x4" },
		{ bimg::TextureFormat::ASTC5x5, "astc5x5" },
		{ bimg::TextureFormat::ASTC6x5, "astc6x5" },
		{ bimg::TextureFormat::ASTC6x6, "astc6x6" },
		{ bimg::TextureFormat::ASTC8x5, "astc8x5" },
		{ bimg::TextureFormat::ASTC8x6, "astc8x6" },
		{ bimg::TextureFormat::ASTC8x8, "astc8x8" },
		{ bimg::TextureFormat::ASTC10x5, "astc10x5" },
		{ bimg::TextureFormat::ASTC10x6, "astc10x6" },
		{ bimg::TextureFormat::ASTC10x8, "astc10x8" },
		{ bimg::TextureFormat::ASTC10x10, "astc10x10" },
		{ bimg::TextureFormat::ASTC12x10, "astc12x10" },
		{ bimg::TextureFormat::ASTC12x12, "astc12x12" },
		{ bimg::TextureFormat::Unknown, "unknown" },
		{ bimg::TextureFormat::R1, "r1" },
		{ bimg::TextureFormat::A8, "a8" },
		{ bimg::TextureFormat::R8, "r8" },
		{ bimg::TextureFormat::R8I, "r81" },
		{ bimg::TextureFormat::R8U, "r8u" },
		{ bimg::TextureFormat::R8S, "r8s" },
		{ bimg::TextureFormat::R16, "r16" },
		{ bimg::TextureFormat::R16I, "r16i" },
		{ bimg::TextureFormat::R16U, "r16u" },
		{ bimg::TextureFormat::R16F, "r16f" },
		{ bimg::TextureFormat::R16S, "r16s" },
		{ bimg::TextureFormat::R32I, "r32i" },
		{ bimg::TextureFormat::R32U, "r32u" },
		{ bimg::TextureFormat::R32F, "r32f" },
		{ bimg::TextureFormat::RG8, "rg8" },
		{ bimg::TextureFormat::RG8I, "rg8i" },
		{ bimg::TextureFormat::RG8U, "rg8u" },
		{ bimg::TextureFormat::RG8S, "rg8s" },
		{ bimg::TextureFormat::RG16, "rg16" },
		{ bimg::TextureFormat::RG16I, "rg16i" },
		{ bimg::TextureFormat::RG16U, "rg16u" },
		{ bimg::TextureFormat::RG16F, "rg16f" },
		{ bimg::TextureFormat::RG16S, "rg16s" },
		{ bimg::TextureFormat::RG32I, "rg32i" },
		{ bimg::TextureFormat::RG32U, "rg32u" },
		{ bimg::TextureFormat::RG32F, "rg32f" },
		{ bimg::TextureFormat::RGB8, "rgb8" },
		{ bimg::TextureFormat::RGB8I, "rgb8i" },
		{ bimg::TextureFormat::RGB8U, "rgb8u" },
		{ bimg::TextureFormat::RGB8S, "rgb8s" },
		{ bimg::TextureFormat::RGB9E5F, "rgb9e5f" },
		{ bimg::TextureFormat::BGRA8, "bgra8" },
		{ bimg::TextureFormat::RGB8U, "rgb8u" },
		{ bimg::TextureFormat::RGBA8, "rgba8" },
		{ bimg::TextureFormat::RGBA8I, "rgba8i" },
		{ bimg::TextureFormat::RGBA8U, "rgba8u" },
		{ bimg::TextureFormat::RGBA8S, "rgba8s" },
		{ bimg::TextureFormat::RGBA16, "rgba16" },
		{ bimg::TextureFormat::RGBA16I, "rgba16i" },
		{ bimg::TextureFormat::RGBA16U, "rgba16u" },
		{ bimg::TextureFormat::RGBA16F, "rgba16f" },
		{ bimg::TextureFormat::RGBA16S, "rgba16s" },
		{ bimg::TextureFormat::RGBA32I, "rgba32i" },
		{ bimg::TextureFormat::RGBA32U, "rgba32u" },
		{ bimg::TextureFormat::RGBA32F, "rgba32f" },
		{ bimg::TextureFormat::B5G6R5, "b5g6r5" },
		{ bimg::TextureFormat::R5G6B5, "r5g6b5" },
		{ bimg::TextureFormat::BGRA4, "bgra4" },
		{ bimg::TextureFormat::RGBA4, "rgba4" },
		{ bimg::TextureFormat::BGR5A1, "bgr5a1" },
		{ bimg::TextureFormat::RGB5A1, "rgb5a1" },
		{ bimg::TextureFormat::RGB10A2, "rgb10a2" },
		{ bimg::TextureFormat::RG11B10F, "rg11b10f" },
		{ bimg::TextureFormat::UnknownDepth, "unknown_depth" },
		{ bimg::TextureFormat::D16, "d16" },
		{ bimg::TextureFormat::D24, "d24" },
		{ bimg::TextureFormat::D24S8, "d24s8" },
		{ bimg::TextureFormat::D32, "d32" },
		{ bimg::TextureFormat::D16F, "d16f" },
		{ bimg::TextureFormat::D24F, "d24f" },
		{ bimg::TextureFormat::D32F, "d32f" },
		{ bimg::TextureFormat::D0S8, "d0s8" }
	};

	bimg::TextureFormat::Enum Image::readFormat(std::string_view name) noexcept
	{
		if (name.empty())
		{
			return bimg::TextureFormat::RGBA8;
		}
		auto itr = std::find_if(_formatNames.begin(), _formatNames.end(), [name](auto& elm) { return elm.second == name; });
		if (itr == _formatNames.end())
		{
			return bimg::TextureFormat::Unknown;
		}
		return itr->first;
	}

	ImageEncoding Image::getEncodingForPath(const std::filesystem::path& path) noexcept
	{
		auto ext = StringUtils::getFileExt(path.filename().string());
		if (ext == ".png")
		{
			return ImageEncoding::Png;
		}
		if (ext == ".exr")
		{
			return ImageEncoding::Exr;
		}
		if (ext == ".hdr")
		{
			return ImageEncoding::Hdr;
		}
		if (ext == ".dds")
		{
			return ImageEncoding::Dds;
		}
		if (ext == ".ktx")
		{
			return ImageEncoding::Ktx;
		}
		if (ext == ".tga")
		{
			return ImageEncoding::Tga;
		}
		return ImageEncoding::Count;
	}

	void Image::write(ImageEncoding encoding, std::ostream& stream) const noexcept
	{
		StreamWriter writer(stream);
		encode(encoding, writer);
	}

	bool Image::empty() const noexcept
	{
		return _container == nullptr || _container->m_size == 0;
	}

	void Image::update(const glm::uvec2& pos, const glm::uvec2& size, DataView data, size_t elmOffset, size_t elmSize)
	{
		auto imgSize = getSize();
		if (pos.x + size.x > imgSize.x || pos.y + size.y > imgSize.y)
		{
			throw std::runtime_error("size does not fit");
		}
		auto bpp = getTextureInfo().bitsPerPixel / 8;
		elmOffset = elmOffset < 0 ? 0 : elmOffset;
		elmSize = elmSize <= 0 || elmSize > bpp ? bpp : elmSize;
		size_t updateSize = elmSize * size.x * size.y;
		if (data.size() < updateSize)
		{
			throw std::runtime_error("data is too small");
		}
		auto imgStart = pos.x + (pos.y * imgSize.x);
		auto rowSize = elmSize * size.x;
		auto imgData = (uint8_t*)_container->m_data;
		for (size_t y = 0; y < size.y; y++)
		{
			auto rowPtr = &imgData[elmOffset + ((imgStart + (y * imgSize.x)) * bpp)];
			auto row = data.view(y * rowSize, rowSize);
			if (elmSize == bpp)
			{
				std::memcpy(rowPtr, row.ptr(), rowSize);
			}
			else
			{
				for (size_t x = 0; x < size.x; x++)
				{
					auto pix = row.view(x, elmSize);
					std::memcpy(&rowPtr[x * bpp], pix.ptr(), elmSize);
				}
			}
		}
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

	ImageLoader::ImageLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc) noexcept
		: _dataLoader(dataLoader)
		, _alloc(alloc)
	{
	}

	std::shared_ptr<Image> ImageLoader::operator()(const std::filesystem::path& path)
	{
		auto data = _dataLoader(path);
		return std::make_shared<Image>(data, _alloc);
	}

	bool ImageImporter::startImport(const Input& input, bool dry)
	{
		if (input.config.is_null())
		{
			return false;
		}
		_outputPath = input.getOutputPath(".ktx");
		_outputEncoding = Image::getEncodingForPath(_outputPath);
		if (_outputEncoding == ImageEncoding::Count)
		{
			throw std::runtime_error("unknown output encoding");
		}

		_cubemapFaces.reset();

		if (input.config.contains("cubemap"))
		{
			auto& faces = _cubemapFaces.emplace();
			size_t i = 0;
			for (auto& elm : input.config["cubemap"])
			{
				faces[i] = input.basePath / elm.get<std::filesystem::path>();
				++i;
			}
		}

		return true;
	}

	ImageImporter::Outputs ImageImporter::getOutputs(const Input& input) noexcept
	{
		return { _outputPath };
	}

	ImageImporter::Dependencies ImageImporter::getDependencies(const Input& input) noexcept
	{
		if (_cubemapFaces)
		{
			return Dependencies(_cubemapFaces->begin(), _cubemapFaces->end());
		}
		return Dependencies();
	}

	void ImageImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out) noexcept
	{
		std::string formatStr;
		if (input.config.contains("outputFormat"))
		{
			formatStr = input.config["outputFormat"];
		}
		else if (input.dirConfig.contains("outputFormat"))
		{
			formatStr = input.config["outputFormat"];
		}
		auto format = Image::readFormat(formatStr);

		if (_cubemapFaces)
		{
			std::array<Data, 6> faceData;
			std::array<DataView, 6> faceDataView;
			size_t i = 0;
			for (auto& facePath : _cubemapFaces.value())
			{
				faceData[i] = Data::fromFile(facePath);
				faceDataView[i] = faceData[i];
				++i;
			}
			Image img(faceDataView, _alloc, format);
			img.write(_outputEncoding, out);
		}
		else
		{
			Image img(Data::fromFile(input.path), _alloc, format);
			img.write(_outputEncoding, out);
		}
	}

	void ImageImporter::endImport(const Input& input) noexcept
	{
		_outputPath = "";
		_cubemapFaces.reset();
	}

	const std::string& ImageImporter::getName() const noexcept
	{
		static const std::string name = "image";
		return name;
	}
}