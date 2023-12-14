#include "asset.hpp"
#include <stdexcept>
#include <optional>
#include <bimg/decode.h>
#include <pugixml.hpp>
#include <darmok/asset.hpp>
#include <darmok/utils.hpp>
#include <bx/filepath.h>

namespace darmok
{
	void FileReader::setBasePath(const std::string& basePath)
	{
		_basePath = basePath;
	}

	bool FileReader::open(const bx::FilePath& filePath, bx::Error* err)
	{
		auto absFilePath = _basePath + filePath.getCPtr();
		return super::open(absFilePath.c_str(), err);
	}

	void FileWriter::setBasePath(const std::string& basePath)
	{
		_basePath = basePath;
	}

	bool FileWriter::open(const bx::FilePath& filePath, bool append, bx::Error* err)
	{
		auto absFilePath = _basePath + filePath.getCPtr();
		return super::open(absFilePath.c_str(), append, err);
	}

	Data::Data(void* ptr, uint64_t size, bx::AllocatorI* alloc) noexcept
		: _ptr(ptr)
		, _size(size)
		, _alloc(alloc)
	{
	}

	void* Data::ptr() const noexcept
	{
		return _ptr;
	}

	uint64_t Data::size() const noexcept
	{
		return _size;
	}

	bool Data::empty() const noexcept
	{
		return _size == 0ll;
	}

	Data::~Data() noexcept
	{
		bx::free(_alloc, _ptr);
	}

	Data::Data(Data&& other) noexcept
		: _ptr(other._ptr)
		, _size(other._size)
		, _alloc(other._alloc)
	{
		other._ptr = nullptr;
		other._size = 0;
	}
	Data& Data::operator=(Data&& other) noexcept
	{
		bx::free(_alloc, _ptr);
		_ptr = other._ptr;
		_size = other._size;
		_alloc = other._alloc;
		other._ptr = nullptr;
		other._size = 0;
		return *this;
	}

	bx::FileReaderI* AssetContextImpl::getFileReader() noexcept
	{
		return &_fileReader;
	}

	bx::FileWriterI* AssetContextImpl::getFileWriter() noexcept
	{
		return &_fileWriter;
	}

	bx::AllocatorI* AssetContextImpl::getAllocator() noexcept
	{
		return &_allocator;
	}

	Data AssetContextImpl::loadData(const std::string& filePath)
	{
		auto reader = getFileReader();
		auto alloc = getAllocator();
		if (bx::open(reader, filePath.c_str()))
		{
			auto size = bx::getSize(reader);
			void* data = bx::alloc(alloc, size);
			bx::read(reader, data, (int32_t)size, bx::ErrorAssert{});
			bx::close(reader);
			return Data(data, size, alloc);
		}

		throw std::runtime_error("Failed to load data from file \"" + filePath + "\".");
	}

	const bgfx::Memory* AssetContextImpl::loadMem(const std::string& filePath)
	{
		auto reader = getFileReader();
		if (bx::open(reader, filePath.c_str()))
		{
			uint32_t size = (uint32_t)bx::getSize(reader);
			const bgfx::Memory* mem = bgfx::alloc(size + 1);
			bx::read(reader, mem->data, size, bx::ErrorAssert{});
			bx::close(reader);
			return mem;
		}
		throw std::runtime_error("Failed to load memory from file \"" + filePath + "\".");
	}

	static std::string getShaderExt()
	{
		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12: return "dx11";  break;
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:        return "pssl";  break;
		case bgfx::RendererType::Metal:      return "metal"; break;
		case bgfx::RendererType::Nvn:		 return "nvn";   break;
		case bgfx::RendererType::OpenGL:     return "glsl";  break;
		case bgfx::RendererType::OpenGLES:   return "essl";  break;
		case bgfx::RendererType::Vulkan:     return "spv"; break;

		case bgfx::RendererType::Count:
			BX_ASSERT(false, "You should not be here!");
			break;
		}
		return "???";
	}

	bgfx::ShaderHandle AssetContextImpl::loadShader(const std::string& name)
	{
		std::string filePath = name + "." + getShaderExt() + ".bin";
		auto mem = loadMem(filePath);
		
		if (mem == nullptr)
		{
			return { bgfx::kInvalidHandle };
		}
		
		bgfx::ShaderHandle handle = bgfx::createShader(mem);
		bgfx::setName(handle, name.c_str());

		return handle;
	}

	bgfx::ProgramHandle AssetContextImpl::loadProgram(const std::string& vertexName, const std::string& fragmentName)
	{
		bgfx::ShaderHandle vsh = loadShader(vertexName);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!fragmentName.empty())
		{
			fsh = loadShader(fragmentName);
		}

		return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
	}

	static void imageReleaseCb(void* ptr, void* userData)
	{
		BX_UNUSED(ptr);
		bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)userData;
		bimg::imageFree(imageContainer);
	}

	TextureWithInfo AssetContextImpl::loadTextureWithInfo(const std::string& name, uint64_t flags)
	{
		return loadTexture(name, flags, true);
	}

	bgfx::TextureHandle AssetContextImpl::loadTexture(const std::string& name, uint64_t flags)
	{
		return loadTexture(name, flags, false).texture;
	}

	TextureWithInfo AssetContextImpl::loadTexture(const std::string& filePath, uint64_t flags, bool loadInfo)
	{
		TextureWithInfo r;
		r.texture = BGFX_INVALID_HANDLE;

		auto data = loadData(filePath);
		if (!data.empty())
		{
			auto alloc = getAllocator();
			bx::Error err;
			bimg::ImageContainer* imageContainer = bimg::imageParse(alloc, data.ptr(), (uint32_t)data.size(), bimg::TextureFormat::Count, &err);
			checkError(err);
			if (imageContainer != nullptr)
			{
				r.orientation = imageContainer->m_orientation;

				const bgfx::Memory* mem = bgfx::makeRef(
					imageContainer->m_data
					, imageContainer->m_size
					, imageReleaseCb
					, imageContainer
				);

				if (imageContainer->m_cubeMap)
				{
					r.texture = bgfx::createTextureCube(
						uint16_t(imageContainer->m_width)
						, 1 < imageContainer->m_numMips
						, imageContainer->m_numLayers
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
						, flags
						, mem
					);
				}
				else if (1 < imageContainer->m_depth)
				{
					r.texture = bgfx::createTexture3D(
						uint16_t(imageContainer->m_width)
						, uint16_t(imageContainer->m_height)
						, uint16_t(imageContainer->m_depth)
						, 1 < imageContainer->m_numMips
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
						, flags
						, mem
					);
				}
				else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), flags))
				{
					r.texture = bgfx::createTexture2D(
						uint16_t(imageContainer->m_width)
						, uint16_t(imageContainer->m_height)
						, 1 < imageContainer->m_numMips
						, imageContainer->m_numLayers
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
						, flags
						, mem
					);
				}

				if (bgfx::isValid(r.texture))
				{
					bgfx::setName(r.texture, filePath.c_str());
				}

				if (loadInfo)
				{
					bgfx::calcTextureSize(
						r.info
						, uint16_t(imageContainer->m_width)
						, uint16_t(imageContainer->m_height)
						, uint16_t(imageContainer->m_depth)
						, imageContainer->m_cubeMap
						, 1 < imageContainer->m_numMips
						, imageContainer->m_numLayers
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
					);
				}
			}
		}

		if (!isValid(r.texture))
		{
			throw std::runtime_error("Could not load texture in path \"" + filePath + "\"");
		}

		return r;
	}

	Image AssetContextImpl::loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat)
	{
		auto data = loadData(filePath);
		auto alloc = getAllocator();
		bimg::ImageContainer* ptr = bimg::imageParse(alloc, data.ptr(), (uint32_t)data.size(), bimg::TextureFormat::Enum(dstFormat));
		return Image(ptr);
	}

	static std::pair<int, size_t> readAtlasXmlValueInt(const std::string& str, size_t i)
	{
		if (str.size() == 0 || i == std::string::npos || i >= str.size())
		{
			return { -1, std::string::npos };
		}
		auto ni = str.find(' ', i);
		if (ni == std::string::npos)
		{
			ni = str.size();
		}
		auto v = std::stoi(str.substr(i, ni - i));
		return { v, ni + 1 };
	}

	static std::pair<std::optional<TextureVec2>, size_t> readAtlasXmlValueVec(const std::string& str, size_t i)
	{
		auto p = readAtlasXmlValueInt(str, i);
		if (p.first < 0)
		{
			return { std::nullopt, std::string::npos };
		}
		TextureVec2 v = { p.first, 0 };
		i = p.second;
		if (i == std::string::npos)
		{
			return { v, i };
		}
		p = readAtlasXmlValueInt(str, i);
		if (p.first < 0)
		{
			return { std::nullopt, std::string::npos };
		}
		v.y = p.first;
		i = p.second;
		return { v, i };
	}

	static TextureAtlasElement loadAtlasElement(pugi::xml_node& xml)
	{
		std::vector<TextureAtlasVertex> vertices;

		std::string vertPosVals = xml.child("vertices").text().get();
		std::string vertTexVals = xml.child("verticesUV").text().get();
		size_t pi = 0, ti = 0;

		while (pi != std::string::npos && ti != std::string::npos)
		{
			auto pp = readAtlasXmlValueVec(vertPosVals, pi);
			if (!pp.first.has_value())
			{
				break;
			}
			auto tp = readAtlasXmlValueVec(vertTexVals, ti);
			if (!tp.first.has_value())
			{
				break;
			}
			vertices.push_back({ pp.first.value(), tp.first.value() });
			pi = pp.second;
			ti = tp.second;
		}

		std::string vertIdxVals = xml.child("triangles").text().get();
		size_t ii = 0;
		std::vector<TextureAtlasVertexIndex> indices;
		while (ii != std::string::npos)
		{
			auto p = readAtlasXmlValueInt(vertIdxVals, ii);
			if (p.first < 0)
			{
				break;
			}
			indices.push_back(p.first);
			ii = p.second;
		}

		return {
			xml.attribute("n").value(), vertices, indices,
			{ xml.attribute("x").as_int(), xml.attribute("y").as_int() },
			{ xml.attribute("w").as_int(), xml.attribute("h").as_int() },
			{ xml.attribute("oX").as_int(), xml.attribute("oY").as_int() },
			{ xml.attribute("oW").as_int(), xml.attribute("oH").as_int() },
			{ xml.attribute("pX").as_int(), xml.attribute("pY").as_int() },
			xml.attribute("r").value() == "y"
		};
	}

	TextureAtlas AssetContextImpl::loadAtlas(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat)
	{
		TextureAtlas atlas;
		auto data = loadData(filePath);
		pugi::xml_document doc;
		auto result = doc.load_buffer_inplace(data.ptr(), data.size());
		if (result.status != pugi::status_ok)
		{
			throw std::runtime_error(result.description());
		}
		auto atlasXml = doc.child("TextureAtlas");
		atlas.name = atlasXml.attribute("imagePath").value();
		atlas.size.x = atlasXml.attribute("width").as_int();
		atlas.size.y = atlasXml.attribute("height").as_int();
		const char* spriteTag = "sprite";
		for (pugi::xml_node spriteXml = atlasXml.child(spriteTag); spriteXml; spriteXml = spriteXml.next_sibling(spriteTag))
		{
			atlas.elements.push_back(loadAtlasElement(spriteXml));
		}

		std::string atlasTexturePath = atlas.name;
		bx::FilePath fp(atlasTexturePath.c_str());
		if (fp.getPath().isEmpty())
		{
			fp.set(filePath.c_str());
			std::string basePath(fp.getPath().getPtr(), fp.getPath().getLength());
			atlasTexturePath = basePath + atlasTexturePath;
		}
		atlas.texture = loadTexture(atlasTexturePath, dstFormat);

		return atlas;
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

	bgfx::ShaderHandle AssetContext::loadShader(const std::string& name)
	{
		return _impl->loadShader(name);
	}

	bgfx::ProgramHandle AssetContext::loadProgram(const std::string& vertexName, const std::string& fragmentName)
	{
		return _impl->loadProgram(vertexName, fragmentName);
	}

	bgfx::TextureHandle AssetContext::loadTexture(const std::string& name, uint64_t flags)
	{
		return _impl->loadTexture(name, flags);
	}

	TextureWithInfo AssetContext::loadTextureWithInfo(const std::string& name, uint64_t flags)
	{
		return _impl->loadTextureWithInfo(name, flags);
	}

	Image AssetContext::loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat)
	{
		return _impl->loadImage(filePath, dstFormat);
	}

	TextureAtlas AssetContext::loadAtlas(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat)
	{
		return _impl->loadAtlas(filePath, dstFormat);
	}

	AssetContextImpl& AssetContext::getImpl() noexcept
	{
		return *_impl;
	}

	const AssetContextImpl& AssetContext::getImpl() const noexcept
	{
		return *_impl;
	}

	Image::Image(bimg::ImageContainer* container)
		: _container(container)
		{
		}

	Image::~Image()
	{
		bimg::imageFree(_container);
	}
}
