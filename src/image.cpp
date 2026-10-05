#include <darmok/image.hpp>
#include <darmok/utils.hpp>
#include <darmok/color.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/stream.hpp>
#include <darmok/texture.hpp>
#include <darmok/string.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/math.hpp>
#include <darmok/multiarray.hpp>

#include <stdexcept>
#include <bx/readerwriter.h>
#include <bimg/decode.h>
#include <format>
#include <magic_enum/magic_enum.hpp>
#include <magic_enum/magic_enum_format.hpp>

namespace darmok
{
	expected<Image, std::string> Image::load(DataView data, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format) noexcept
	{
		bx::Error err;
		auto container = bimg::imageParse(&alloc, data.ptr(), (uint32_t)data.size(), format, &err);
		if (err.isOk())
		{
			return Image{ container };
		}
		return unexpected<std::string>{ err.getMessage().getCPtr() };
	}

	expected<Image, std::string> Image::load(const std::array<DataView, 6>& facesData, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format) noexcept
	{
		std::vector<Image> faces;
		bimg::TextureFormat::Enum fformat;
		glm::uvec2 size;
        std::optional<bool> hasMips;
		for (auto& faceData : facesData)
		{
			auto result = load(faceData, alloc, format);
			if (!result)
			{
				return unexpected{ std::move(result).error() };
			}
			auto face = std::move(result).value();
			if (!faces.empty())
			{
				if (face.getFormat() != fformat)
				{
					return unexpected<std::string>{"all faces should have the same format"};
				}
				if (face.getSize() != size)
				{
					return unexpected<std::string>{"all faces should have the same size"};
				}
			}
			else
			{
				fformat = face.getFormat();
				size = face.getSize();
			}
			faces.push_back(std::move(face));
            bool faceHasMips = faces.back().getMipCount() > 1;
            if(!hasMips.has_value())
            {
                hasMips = faceHasMips;
            }
            else if(hasMips.value() != faceHasMips)
            {
                return unexpected<std::string>{"all faces should have the same mip count"};
            }
		}

		auto container = bimg::imageAlloc(&alloc, fformat, size.x, size.y, 1, 1, true, *hasMips);
		auto ptr = static_cast<uint8_t*>(container->m_data);
        const auto faceSize = faces[0].getData().size();
        for(size_t i = 0; i < faces.size(); ++i)
        {
            std::memcpy(
                ptr + faceSize * i,
                faces[i].getData().ptr(),
                faceSize);
        }

		return Image{ container };
	}

	Image::Image(const Color& color, bx::AllocatorI& alloc, const glm::uvec2& size) noexcept
		: Image(size, alloc, bimg::TextureFormat::RGBA8)
	{
		auto c = Colors::toReverseNumber(color);
		bimg::imageSolid(_container->m_data, size.x, size.y, c);
	}

	Image::Image(const glm::uvec2& size, bx::AllocatorI& alloc, bimg::TextureFormat::Enum format) noexcept
		: _container{ bimg::imageAlloc(
			&alloc, format, size.x, size.y, 0, 1, false, false
		) }
	{
		std::memset(_container->m_data, 0, _container->m_size);
	}

    Image::Image(const PixelArray2d& pixels, bx::AllocatorI& alloc) noexcept
        : _container{bimg::imageAlloc(
            &alloc,
            bimg::TextureFormat::RGBA32F,
            pixels.extent(0),
            pixels.extent(1),
            1,
            1,
            false,
            false,
            pixels.data())
        }
    {
    }

    Image::Image(const PixelArray3d& cubePixels, bx::AllocatorI& alloc) noexcept
        : _container{
              bimg::imageAlloc(
                  &alloc,
                  bimg::TextureFormat::RGBA32F,
                  cubePixels.extent(1),
                  cubePixels.extent(2),
                  1,
                  1,
                  true,
                  false,
                  cubePixels.data())
        }
    {
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

		auto info = other.getTextureInfo();

		_container = bimg::imageAlloc(
            other._container->m_allocator, format, size.x, size.y, info.depth, info.numLayers, info.cubeMap, info.numMips > 1
		);

		auto bpp = info.bitsPerPixel / 8;
		uint32_t pitch = bpp * size.x;
		bimg::imageCopy(_container->m_data, size.x, size.y, info.depth, info.bitsPerPixel, pitch, other._container->m_data);
	}

    Image::Image(bimg::ImageContainer* container) noexcept
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

	bx::AllocatorI& Image::getAllocator() const noexcept
	{
		assert(_container);
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
			return unexpected{ std::format("cannot encode encoding {}", encoding) };
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

	expected<void, std::string> Image::update(const glm::uvec2& pos, const glm::uvec2& size, DataView data, size_t elmOffset, size_t elmSize) noexcept
	{
        if(!_container)
        {
            return unexpected{"image is empty"};
        }
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

    expected<bimg::ImageMip, std::string> Image::getMip(uint16_t side, uint8_t lod) const noexcept
    {
        if (!_container)
        {
            return unexpected{"image is empty"};
        }
        bimg::ImageMip mip;
        if(!bimg::imageGetRawData(
               *_container,
               side,
               lod,
               _container->m_data,
               _container->m_size,
               mip))
        {
            return unexpected{"failed to get mip level"};
        }
        return mip;
    }

    expected<Image, std::string> Image::convertFormat(bimg::TextureFormat::Enum format) const noexcept
    {
        if(!_container)
        {
            return unexpected{"image is empty"};
        }
        bimg::ImageContainer* converted = bimg::imageConvert(
            _container->m_allocator,
            format,
            *_container);
        if(!converted)
        {
            return unexpected{"failed to convert image"};
        }
        return Image{converted};
    }

    expected<PixelArray2d, std::string> Image::loadMipData(const bimg::ImageMip& mip) noexcept
    {
        if(mip.m_format != bimg::TextureFormat::RGBA32F)
        {
            return unexpected{"format is not RGBA32F"};
        }
        return PixelArray2d::load(DataView{mip.m_data, mip.m_size}, {mip.m_width, mip.m_height});
    }

    expected<PixelArray2d, std::string> Image::getPixels(uint8_t face, uint8_t lod) const noexcept
    {
        auto convertResult = convertFormat(bimg::TextureFormat::RGBA32F);
        if(!convertResult)
        {
            return unexpected{std::format("Failed to convert image: {}", convertResult.error())};
        }
        auto mipResult = convertResult->getMip(face, lod);
        if(!mipResult)
        {
            return unexpected{std::format("Failed to get image mip: {}", mipResult.error())};
        }
        auto arrayResult = loadMipData(mipResult.value());
        if(!arrayResult)
        {
            return unexpected{std::format("Failed to convert image mip to array: {}", arrayResult.error())};
        }
        return arrayResult;
    }

    expected<PixelArray3d, std::string> Image::getCubemapPixels(uint8_t lod) const noexcept
    {
        if(!_container)
        {
            return unexpected{"image is empty"};
        }

        if(!_container->m_cubeMap)
        {
            return unexpected{"image is not a cubemap"};
        }

        auto face = getPixels(0, lod);
        if(!face)
        {
            return unexpected{
                std::format("failed to get cubemap face: {}", face.error())};
        }

        const auto& facePixels = face.value();

        PixelArray3d pixels{
            6,
            facePixels.extent(1),
            facePixels.extent(0)};

        pixels[0] = std::move(face).value();

        for(glm::uint faceIndex = 1; faceIndex < 6; ++faceIndex)
        {
            auto result = getPixels(faceIndex, lod);
            if(!result)
            {
                return unexpected{
                    std::format(
                        "failed to get cubemap face {}: {}",
                        faceIndex,
                        result.error())};
            }

            pixels[faceIndex] = std::move(result).value();
        }

        return pixels;
    }

    glm::vec4 Image::sampleBilinear(const PixelArray2d& pixels, glm::vec2 uv) noexcept
    {
        uv.x = glm::fract(uv.x);
        uv.y = glm::clamp(uv.y, 0.0f, 1.0f);

        // extent(0) = width, extent(1) = height; raw data is row-major from bimg
        const auto width = static_cast<int>(pixels.extent(0));
        const auto height = static_cast<int>(pixels.extent(1));

        const glm::vec2 pos = uv * glm::vec2(width, height) - 0.5f;
        const glm::ivec2 p = glm::ivec2(glm::floor(pos));
        const glm::vec2 f = glm::fract(pos);

        const auto* data = pixels.data();

        const auto wrapX = [width](int x) {
            return (x % width + width) % width;
        };
        const auto clampY = [height](int y) {
            return std::clamp(y, 0, height - 1);
        };
        const auto fetch = [&](int x, int y) -> Pixel {
            return data[clampY(y) * width + wrapX(x)];
        };

        const auto a = fetch(p.x, p.y);
        const auto b = fetch(p.x + 1, p.y);
        const auto c = fetch(p.x, p.y + 1);
        const auto d = fetch(p.x + 1, p.y + 1);

        return glm::mix(
            glm::mix(a, b, f.x),
            glm::mix(c, d, f.x),
            f.y);
    }

    expected<PixelArray3d, std::string> Image::convertEquirectangularCubemap(const PixelArray2d& pixels) noexcept
    {
        if(pixels.extent(0) != pixels.extent(1) * 2)
        {
            return std::unexpected{"invalid equirectangular image dimensions"};
        }

        const auto faceSize = pixels.extent(0) / 4;

        PixelArray3d result{6, faceSize, faceSize};

        for(glm::uint face = 0; face < 6; ++face)
        {
            for(glm::uint y = 0; y < faceSize; ++y)
            {
                for(glm::uint x = 0; x < faceSize; ++x)
                {
                    const glm::vec2 uv =
                        (glm::vec2(x, y) + 0.5f) /
                        static_cast<float>(faceSize);

                    const glm::vec2 p = uv * 2.0f - 1.0f;

                    const auto direction =
                        Math::cubeDirection(face, p);

                    const auto sourceUV =
                        Math::equirectangularUV(direction);

                    result(face, y, x) =
                        sampleBilinear(pixels, sourceUV);
                }
            }
        }

        return result;
    }

    void Image::downsampleMip(
        const bimg::ImageMip& srcMip,
        bimg::ImageMip& dstMip) noexcept
    {
        const auto* src = reinterpret_cast<const glm::vec4*>(srcMip.m_data);
        auto* dst = reinterpret_cast<glm::vec4*>(const_cast<uint8_t*>(dstMip.m_data));

        const uint32_t srcWidth = srcMip.m_width;
        const uint32_t srcHeight = srcMip.m_height;
        const uint32_t dstWidth = dstMip.m_width;
        const uint32_t dstHeight = dstMip.m_height;

        for(uint32_t y = 0; y < dstHeight; ++y)
        {
            const uint32_t sy0 = y * 2;
            const uint32_t sy1 = std::min(sy0 + 1, srcHeight - 1);

            for(uint32_t x = 0; x < dstWidth; ++x)
            {
                const uint32_t sx0 = x * 2;
                const uint32_t sx1 = std::min(sx0 + 1, srcWidth - 1);

                const auto a = src[sy0 * srcWidth + sx0];
                const auto b = src[sy0 * srcWidth + sx1];
                const auto c = src[sy1 * srcWidth + sx0];
                const auto d = src[sy1 * srcWidth + sx1];

                dst[y * dstWidth + x] = (a + b + c + d) * 0.25f;
            }
        }
    }

    void Image::copyMip(
        const bimg::ImageMip& srcMip,
        bimg::ImageMip& dstMip) noexcept
    {
        std::memcpy(const_cast<uint8_t*>(dstMip.m_data), srcMip.m_data, srcMip.m_size);
    }

    expected<Image, std::string> Image::generateMips() const noexcept
    {
        if(!_container)
        {
            return unexpected{"image is empty"};
        }

        if(_container->m_numMips != 1)
        {
            return unexpected{"image already has mipmaps"};
        }

        auto convertResult = convertFormat(bimg::TextureFormat::RGBA32F);
        if(!convertResult)
        {
            return unexpected{"failed to convert to RGBA32F: " + convertResult.error()};
        }

        auto srcImage = std::move(convertResult).value();
        auto srcContainer = srcImage._container;
        const auto numMips = getMaxMipCount();

        auto* container = bimg::imageAlloc(
            srcContainer->m_allocator,
            srcContainer->m_format,
            srcContainer->m_width,
            srcContainer->m_height,
            srcContainer->m_depth,
            srcContainer->m_numLayers,
            srcContainer->m_cubeMap,
            true);

        if(!container)
        {
            return unexpected{"failed to allocate mipmaps"};
        }

        Image image{container};

        const auto sideCount = srcContainer->m_cubeMap
                                   ? srcContainer->m_numLayers * 6
                                   : srcContainer->m_numLayers;

        // Copy the original image into mip 0.
        for(uint16_t side = 0; side < sideCount; ++side)
        {
            auto src = srcImage.getMip(side, 0);
            auto dst = image.getMip(side, 0);

            if(!src || !dst)
            {
                return unexpected{
                    std::format("failed to access mip 0, side {}", side)};
            }

            copyMip(*src, *dst);
        }

        // Generate the remaining mip levels from the previous level.
        for(uint8_t level = 1; level < numMips; ++level)
        {
            for(uint16_t side = 0; side < sideCount; ++side)
            {
                auto src = image.getMip(side, level - 1);
                auto dst = image.getMip(side, level);

                if(!src || !dst)
                {
                    return unexpected{
                        std::format(
                            "failed to access mip {}, side {}",
                            level,
                            side)};
                }

                downsampleMip(*src, *dst);
            }
        }

        return image;
    }

    uint8_t Image::getMipCountForSize(glm::uint size)
    {
        return 1u + static_cast<uint8_t>(std::floor(std::log2(size)));
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

    uint8_t Image::getMaxMipCount() const noexcept
    {
        if(!_container)
        {
            return 0;
        }
        return bimg::imageGetNumMips(
            _container->m_format,
            _container->m_width,
            _container->m_height,
            _container->m_depth);
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
		*config.mutable_size() = convert<protobuf::Uvec2>(getSize());
		config.set_format(Texture::Format(getFormat()));
		config.set_type(Texture::Type(getTextureType()));
		config.set_depth(getDepth());
		config.set_mips(getMipCount() > 1);
		config.set_layers(getLayerCount());
		return config;
	}

	ImageLoader::ImageLoader(IDataLoader& dataLoader, OptionalRef<bx::AllocatorI> alloc) noexcept
		: _dataLoader{ dataLoader }
		, _alloc{ alloc }
	{
	}

    ImageLoader& ImageLoader::addConverter(IImageConverter& converter) noexcept
    {
        _converters.emplace_back(&converter);
        return *this;
    }

    bool ImageLoader::removeConverter(IImageConverter& converter) noexcept
    {
        auto itr = std::ranges::find_if(_converters, [&converter](auto c)
                                        { return c.ptr() == &converter; });
        if(itr != _converters.end())
        {
            _converters.erase(itr);
            return true;
        }
        return false;
    }

	ImageLoader::Result ImageLoader::operator()(std::filesystem::path path) noexcept
	{
		auto dataResult = _dataLoader(path);
		if (!dataResult)
		{
			return unexpected{ std::move(dataResult).error() };
		}
        auto& alloc = _alloc ? *_alloc : _defaultAlloc;
		auto loadResult = Image::load(dataResult.value(), alloc);
		if (!loadResult)
		{
			return unexpected{ std::move(loadResult).error() };
		}
        auto img = std::move(loadResult).value();
        for(auto& converter : _converters)
        {
            auto convertResult = (*converter)(img);
            if(!convertResult)
            {
                return unexpected{std::move(convertResult).error()};
            }
            img = std::move(convertResult).value();
        }
        
		return std::make_shared<Image>(img);
	}

    expected<Image, std::string> GenerateMipsImageConverter::operator()(const Image& source) noexcept
    {
        return source.generateMips();
    }

    BaseImageFileImporter::BaseImageFileImporter(OptionalRef<bx::AllocatorI> alloc)
        : _alloc{alloc}
        , _convertCubemap{false}
        , _generateMips{true}
        , _format{bimg::TextureFormat::Count}
    {
    }

    expected<void, std::string> BaseImageFileImporter::prepare(const Input& input) noexcept
    {
        _cubemapFaces.reset();

        auto itr = input.config.find("cubemap");
        if(itr != input.config.end())
        {
            auto& faces = _cubemapFaces.emplace();
            size_t i = 0;
            for(auto& elm : *itr)
            {
                auto path = input.basePath / elm.get<std::filesystem::path>();
                faces[i] = path;
                ++i;
            }
        }
        itr = input.config.find("convertCubemap");
        if(itr != input.config.end())
        {
            _convertCubemap = itr->get<bool>();
        }
        itr = input.config.find("generateMips");
        if(itr != input.config.end())
        {
            _generateMips = itr->get<bool>();
        }

        static constexpr std::string_view formatKey = "outputFormat";
        std::string formatStr;
        itr = input.config.find(formatKey);
        if(itr != input.config.end())
        {
            formatStr = *itr;
        }
        else
        {
            itr = input.dirConfig.find(formatKey);
            if(itr != input.dirConfig.end())
            {
                formatStr = *itr;
            }
        }
        _format = Image::readFormat(formatStr);

        return {};
    }

    FileImportDependencies BaseImageFileImporter::getDependencies() const noexcept
    {
        return _cubemapFaces ? FileImportDependencies(_cubemapFaces->begin(), _cubemapFaces->end()) : FileImportDependencies{};
    }

    expected<Image, std::string> BaseImageFileImporter::operator()(const Input& input) noexcept
    {        
        expected<Image, std::string> loadResult = std::unexpected{""};
        auto& alloc = _alloc ? *_alloc : _defaultAlloc;
        if(_cubemapFaces)
        {
            std::array<Data, 6> faceData;
            std::array<DataView, 6> faceDataView;
            size_t i = 0;
            for(auto& facePath : _cubemapFaces.value())
            {
                auto readResult = Data::fromFile(facePath);
                if(!readResult)
                {
                    return unexpected{"failed to read face data: " + readResult.error()};
                }
                faceData[i] = readResult.value();
                faceDataView[i] = faceData[i];
                ++i;
            }
            loadResult = Image::load(faceDataView, alloc, _format);
        }
        else
        {
            auto readResult = Data::fromFile(input.path);
            if(!readResult)
            {
                return unexpected{"failed to read data: " + readResult.error()};
            }

            loadResult = Image::load(readResult.value(), alloc, _format);
        }

        if(!loadResult)
        {
            return unexpected{"failed to load image " + loadResult.error()};
        }

        auto img = std::move(loadResult).value();

        if(_convertCubemap)
        {
            auto pixelsResult = img.getPixels();
            if(!pixelsResult)
            {
                return unexpected{"failed to get pixels: " + pixelsResult.error()};
            }
            auto cubemapResult = Image::convertEquirectangularCubemap(pixelsResult.value());
            if(!cubemapResult)
            {
                return unexpected{"failed to convert equirectangular to cubemap: " + cubemapResult.error()};
            }
            img = Image{cubemapResult.value(), alloc};
            if(_generateMips)
            {
                auto mipsResult = img.generateMips();
                if(!mipsResult)
                {
                    return unexpected{std::move(mipsResult).error()};
                }
                img = std::move(mipsResult).value();
            }
        }
        else if(_generateMips && img.getMipCount() == 1)
        {
            auto mipsResult = img.generateMips();
            if(!mipsResult)
            {
                return unexpected{std::move(mipsResult).error()};
            }
            img = std::move(mipsResult).value();
        }

        return img;
    }

	ImageFileImporter::ImageFileImporter(OptionalRef<bx::AllocatorI> alloc) noexcept
        : _base{alloc}
	{
	}

	expected<ImageFileImporter::Effect, std::string> ImageFileImporter::prepare(const Input& input) noexcept
	{
        Effect effect;
        if(input.config.is_null())
        {
            return effect;
        }

        auto baseResult = _base.prepare(input);
        if(!baseResult)
        {
            return unexpected{std::move(baseResult).error()};
        }
        effect.dependencies = _base.getDependencies();

        auto outputPath = input.getOutputPath(".ktx");
        _outputEncoding = Image::getEncodingForPath(outputPath);
        if(_outputEncoding == ImageEncoding::Count)
        {
            return unexpected{"unknown output encoding"};
        }

        effect.outputs.emplace_back(outputPath, true);

        return effect;
	}

	expected<void, std::string> ImageFileImporter::operator()(const Input& input, Config& config) noexcept
	{
        auto loadResult = _base(input);
        if(!loadResult)
        {
            return unexpected{"failed to load image: " + loadResult.error()};
        }
        auto img = std::move(loadResult).value();
        for(auto& optOut : config.outputStreams)
        {
            if(!optOut)
            {
                continue;
            }
            auto writeResult = img.write(_outputEncoding, *optOut);
			if (!writeResult)
			{
				return unexpected{ "failed to write image: " + writeResult.error() };
			}
		}

		return {};
	}

	const std::string& ImageFileImporter::getName() const noexcept
	{
		static const std::string name = "image";
		return name;
	}
}
