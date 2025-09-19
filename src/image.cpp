#include <darmok/image.hpp>
#include <darmok/utils.hpp>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include <darmok/glm_serialize.hpp>

#include <stdexcept>
#include <bx/readerwriter.h>
#include <bimg/decode.h>
#include <fmt/format.h>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_format.hpp>

namespace darmok
{
	Image::Image(DataView data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format)
		: _container{ nullptr }
	{
		if(!data.empty())
		{
			bx::Error err;
			_container = bimg::imageParse(&alloc, data.ptr(), (uint32_t)data.size(), format, &err);
			throwIfError(err);
		}
	}

	Image::Image(const std::array<DataView, 6>& faceData, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format)
	{
		auto right	= Image{ faceData[0], alloc, format };
		auto left	= Image{ faceData[1], alloc, format };
		auto top	= Image{ faceData[2], alloc, format };
		auto bottom = Image{ faceData[3], alloc, format };
		auto front	= Image{ faceData[4], alloc, format };
		auto back	= Image{ faceData[5], alloc, format };

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
		auto ptr = static_cast<uint8_t*>(_container->m_data);
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
		: _container{ bimg::imageAlloc(
			&alloc, format, size.x, size.y, 0, 1, false, false
		) }
	{
		std::memset(_container->m_data, 0, _container->m_size);
	}

	Image::Image(const Image& other) noexcept
		: _container{ nullptr }
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
		: _container{ container }
	{
	}

	Image::~Image() noexcept
	{
		if (_container)
		{
			bimg::imageFree(_container);
		}
	}

	Image::Image(Image&& other) noexcept
		: _container{ other._container }
	{
		other._container = nullptr;
	}

	Image& Image::operator=(Image&& other) noexcept
	{
		if (_container)
		{
			bimg::imageFree(_container);
		}
		_container = other._container;
		other._container = nullptr;
		return *this;
	}

	DataView Image::getData() const noexcept
	{
		if (!_container)
		{
			return {};
		}
		return DataView{ _container->m_data
			, _container->m_size };
	}

	expected<void, std::string> Image::setData(DataView data) noexcept
	{
		return update({ 0, 0 }, getSize(), data);
	}

	bool Image::isTextureValid(uint64_t flags) const noexcept
	{
		auto depth = getDepth();
		auto layers = getLayerCount();
		auto format = static_cast<bgfx::TextureFormat::Enum>(getFormat());
		return bgfx::isTextureValid(depth, false, layers, format, flags);
	}

	Image::TextureType Image::getTextureType() const noexcept
	{
		if (isCubeMap())
		{
			return Texture::Definition::CubeMap;
		}
		auto depth = getDepth();
		if (1 < depth)
		{
			return Texture::Definition::Texture3D;
		}
		return Texture::Definition::Texture2D;
	}

	bx::AllocatorI& Image::getAllocator() const
	{
		if (!_container)
		{
			throw std::runtime_error("image is empty");
		}
		return *_container->m_allocator;
	}

	expected<void, std::string> Image::encode(ImageEncoding encoding, bx::WriterI& writer) const noexcept
	{
		if (!_container)
		{
			return unexpected{ "image is empty" };
		}
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
		default:
			return unexpected{ fmt::format("cannot encode encoding {}", encoding) };
		}
		if (auto errMsg = checkError(err))
		{
			return unexpected{ *errMsg };
		}
		return {};
	}

	expected<Data, std::string> Image::encode(ImageEncoding encoding) const noexcept
	{
		Data data;
		DataMemoryBlock block{ data };
		bx::MemoryWriter writer{ &block };
		auto result = encode(encoding, writer);
		if(!result)
		{
			return unexpected{ result.error() };
		}
		return data;
	}

	bimg::TextureFormat::Enum Image::readFormat(std::string_view name) noexcept
	{
		if (name.empty())
		{
			return bimg::TextureFormat::RGBA8;
		}
		auto result = magic_enum::enum_cast<bimg::TextureFormat::Enum>(name);
		if (!result)
		{
			return bimg::TextureFormat::Unknown;
		}
		return *result;
	}

	ImageEncoding Image::readEncoding(std::string_view name) noexcept
	{
		auto lname = StringUtils::toLower(name);
		if (lname == "png")
		{
			return ImageEncoding::Png;
		}
		if (lname == "jpg" || lname == "jpeg")
		{
			return ImageEncoding::Jpg;
		}
		if (lname == "exr")
		{
			return ImageEncoding::Exr;
		}
		if (lname == "hdr")
		{
			return ImageEncoding::Hdr;
		}
		if (lname == "dds")
		{
			return ImageEncoding::Dds;
		}
		if (lname == "ktx")
		{
			return ImageEncoding::Ktx;
		}
		if (lname == "tga")
		{
			return ImageEncoding::Tga;
		}
		return ImageEncoding::Count;
	}

	ImageEncoding Image::getEncodingForPath(const std::filesystem::path& path) noexcept
	{
		auto ext = path.extension().string();
		if (ext[0] == '.')
		{
			ext = ext.substr(1);
		}
		return readEncoding(ext);
	}

	expected<void, std::string> Image::write(ImageEncoding encoding, std::ostream& stream) const noexcept
	{
		StreamWriter writer{ stream };
		return encode(encoding, writer);
	}

	bool Image::empty() const noexcept
	{
		return _container == nullptr || _container->m_size == 0;
	}

	expected<void, std::string> Image::update(const glm::uvec2& pos, const glm::uvec2& size, DataView data, size_t elmOffset, size_t elmSize)
	{
		auto imgSize = getSize();
		if (pos.x + size.x > imgSize.x || pos.y + size.y > imgSize.y)
		{
			return unexpected{ "size does not fit" };
		}
		auto bpp = getTextureInfo().bitsPerPixel / 8;
		elmOffset = elmOffset < 0 ? 0 : elmOffset;
		elmSize = elmSize <= 0 || elmSize > bpp ? bpp : elmSize;
		size_t updateSize = elmSize * size.x * size.y;
		if (data.size() < updateSize)
		{
			return unexpected{ "data is too small" };
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
		return {};
	}

	glm::uvec2 Image::getSize() const noexcept
	{
		if (!_container)
		{
			return {};
		}
		return glm::uvec2{ _container->m_width, _container->m_height };
	}

	uint32_t Image::getDepth() const noexcept
	{
		if(!_container)
		{
			return 0;
		}
		return _container->m_depth;
	}

	bool Image::isCubeMap() const noexcept
	{
		if (!_container)
		{
			return false;
		}
		return _container->m_cubeMap;
	}

	uint8_t Image::getMipCount() const noexcept
	{
		if (!_container)
		{
			return 0;
		}
		return _container->m_numMips;
	}

	uint16_t Image::getLayerCount() const noexcept
	{
		if (!_container)
		{
			return 0;
		}
		return _container->m_numLayers;
	}

	bimg::TextureFormat::Enum Image::getFormat() const noexcept
	{
		if (!_container)
		{
			return bimg::TextureFormat::Unknown;
		}
		return _container->m_format;
	}

	bgfx::TextureInfo Image::getTextureInfo() const noexcept
	{
		bgfx::TextureInfo info;
		if (_container)
		{
			bgfx::calcTextureSize(
				info
				, static_cast<uint16_t>(_container->m_width)
				, static_cast<uint16_t>(_container->m_height)
				, static_cast<uint16_t>(_container->m_depth)
				, _container->m_cubeMap
				, 1 < _container->m_numMips
				, _container->m_numLayers
				, static_cast<bgfx::TextureFormat::Enum>(_container->m_format)
			);
		}
		return info;
	}

	Image::TextureConfig Image::getTextureConfig() const noexcept
	{
		TextureConfig config;
		*config.mutable_size() = protobuf::convert(getSize());
		config.set_format(Texture::Format(getFormat()));
		config.set_type(Texture::Type(getTextureType()));
		config.set_depth(getDepth());
		config.set_mips(getMipCount() > 1);
		config.set_layers(getLayerCount());
		return config;
	}

	ImageLoader::ImageLoader(IDataLoader& dataLoader, bx::AllocatorI& alloc) noexcept
		: _dataLoader{ dataLoader }
		, _alloc{ alloc }
	{
	}

	ImageLoader::Result ImageLoader::operator()(std::filesystem::path path)
	{
		auto dataResult = _dataLoader(path);
		if (!dataResult)
		{
			return unexpected{ dataResult.error() };
		}
		return std::make_shared<Image>(dataResult.value(), _alloc);
	}

	ImageFileImporter::ImageFileImporter() noexcept
		: _outputEncoding{ ImageEncoding::Count }
	{
	}

	bool ImageFileImporter::startImport(const Input& input, bool dry)
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

	ImageFileImporter::Outputs ImageFileImporter::getOutputs(const Input& input) noexcept
	{
		return { _outputPath };
	}

	ImageFileImporter::Dependencies ImageFileImporter::getDependencies(const Input& input) noexcept
	{
		if (_cubemapFaces)
		{
			return { _cubemapFaces->begin(), _cubemapFaces->end() };
		}
		return {};
	}

	void ImageFileImporter::writeOutput(const Input& input, size_t outputIndex, std::ostream& out)
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
		expected<void, std::string> writeResult;
		if (_cubemapFaces)
		{
			std::array<Data, 6> faceData;
			std::array<DataView, 6> faceDataView;
			size_t i = 0;
			for (auto& facePath : _cubemapFaces.value())
			{
				if (auto result = Data::fromFile(facePath))
				{
					faceData[i] = result.value();
					faceDataView[i] = faceData[i];
				}
				++i;
			}
			Image img{ faceDataView, _alloc, format };
			writeResult = img.write(_outputEncoding, out);
		}
		else
		{
			auto readResult = Data::fromFile(input.path);
			if (readResult)
			{
				throw std::runtime_error{ fmt::format("failed to read data: {}", readResult.error()) };
			}

			Image img{ readResult.value(), _alloc, format };
			writeResult = img.write(_outputEncoding, out);
		}

		if (!writeResult)
		{
			throw std::runtime_error{ fmt::format("failed to write image: {}", writeResult.error()) };
		}
	}

	void ImageFileImporter::endImport(const Input& input) noexcept
	{
		_outputPath.clear();
		_cubemapFaces.reset();
	}

	const std::string& ImageFileImporter::getName() const noexcept
	{
		static const std::string name = "image";
		return name;
	}
}