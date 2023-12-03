#include "asset.hpp"
#include "dbg.h"
#include <bimg/decode.h>
#include <darmok/asset.hpp>

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

	Data::Data(void* ptr, uint64_t size, bx::AllocatorI* alloc)
		: _ptr(ptr)
		, _size(size)
		, _alloc(alloc)
	{
	}

	void* Data::ptr() const
	{
		return _ptr;
	}

	uint64_t Data::size() const
	{
		return _size;
	}

	bool Data::empty() const
	{
		return _size == 0ll;
	}

	Data::~Data()
	{
		bx::free(_alloc, _ptr);
	}

	bx::FileReaderI* AssetContextImpl::getFileReader()
	{
		return &_fileReader;
	}

	bx::FileWriterI* AssetContextImpl::getFileWriter()
	{
		return &_fileWriter;
	}

	bx::AllocatorI* AssetContextImpl::getAllocator()
	{
		return &_allocator;
	}

	std::unique_ptr<Data> AssetContextImpl::loadData(const std::string& filePath)
	{
		auto reader = getFileReader();
		auto alloc = getAllocator();
		if (bx::open(reader, filePath.c_str()))
		{
			auto size = bx::getSize(reader);
			void* data = bx::alloc(alloc, size);
			bx::read(reader, data, (int32_t)size, bx::ErrorAssert{});
			bx::close(reader);
			return std::make_unique<Data>(data, size, alloc);
		}

		DBG("Failed to load %s.", filePath.c_str());
		return nullptr;
	}

	const bgfx::Memory* AssetContextImpl::loadMem(const std::string& filePath)
	{
		auto reader = getFileReader();
		if (bx::open(reader, filePath.c_str()))
		{
			uint32_t size = (uint32_t)bx::getSize(reader);
			const bgfx::Memory* mem =bgfx::alloc(size + 1);
			bx::read(reader, mem->data, size, bx::ErrorAssert{});
			bx::close(reader);
			return mem;
		}

		DBG("Failed to load %s.", filePath.c_str());
		return NULL;
	}

	bgfx::ShaderHandle AssetContextImpl::loadShader(const std::string& name)
	{
		std::string shaderPath = "???";

		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Noop:
		case bgfx::RendererType::Direct3D9:  shaderPath = "shaders/dx9/";   break;
		case bgfx::RendererType::Direct3D11:
		case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/";  break;
		case bgfx::RendererType::Agc:
		case bgfx::RendererType::Gnm:        shaderPath = "shaders/pssl/";  break;
		case bgfx::RendererType::Metal:      shaderPath = "shaders/metal/"; break;
		case bgfx::RendererType::Nvn:        shaderPath = "shaders/nvn/";   break;
		case bgfx::RendererType::OpenGL:     shaderPath = "shaders/glsl/";  break;
		case bgfx::RendererType::OpenGLES:   shaderPath = "shaders/essl/";  break;
		case bgfx::RendererType::Vulkan:     shaderPath = "shaders/spirv/"; break;
		case bgfx::RendererType::WebGPU:     shaderPath = "shaders/spirv/"; break;

		case bgfx::RendererType::Count:
			BX_ASSERT(false, "You should not be here!");
			break;
		}

		std::string filePath = shaderPath + name + ".bin";
		auto mem = loadMem(filePath);
		mem->data[mem->size - 1] = '\0';
		bgfx::ShaderHandle handle = bgfx::createShader(mem);
		bgfx::setName(handle, name.c_str());

		return handle;
	}

	bgfx::ProgramHandle AssetContextImpl::loadProgram(const std::string& vsName, const std::string& fsName)
	{
		bgfx::ShaderHandle vsh = loadShader(vsName);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!fsName.empty())
		{
			fsh = loadShader(fsName);
		}

		return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
	}

	static void imageReleaseCb(void* ptr, void* userData)
	{
		BX_UNUSED(ptr);
		bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)userData;
		bimg::imageFree(imageContainer);
	}

	bgfx::TextureHandle AssetContextImpl::loadTexture(const std::string& filePath, uint64_t flags, uint8_t skip, bgfx::TextureInfo* info, bimg::Orientation::Enum* orientation)
	{
		BX_UNUSED(skip);
		bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;

		auto data = loadData(filePath);
		if (data != nullptr && !data->empty())
		{
			auto alloc = getAllocator();
			bimg::ImageContainer* imageContainer = ::bimg::imageParse(alloc, data->ptr(), (uint32_t)data->size());

			if (imageContainer != nullptr)
			{
				if (NULL != orientation)
				{
					*orientation = imageContainer->m_orientation;
				}

				const bgfx::Memory* mem = bgfx::makeRef(
					imageContainer->m_data
					, imageContainer->m_size
					, imageReleaseCb
					, imageContainer
				);

				if (imageContainer->m_cubeMap)
				{
					handle = bgfx::createTextureCube(
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
					handle = bgfx::createTexture3D(
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
					handle = bgfx::createTexture2D(
						uint16_t(imageContainer->m_width)
						, uint16_t(imageContainer->m_height)
						, 1 < imageContainer->m_numMips
						, imageContainer->m_numLayers
						, bgfx::TextureFormat::Enum(imageContainer->m_format)
						, flags
						, mem
					);
				}

				if (bgfx::isValid(handle))
				{
					bgfx::setName(handle, filePath.c_str());
				}

				if (info != nullptr)
				{
					bgfx::calcTextureSize(
						*info
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

		return handle;
	}

	bimg::ImageContainer* AssetContextImpl::loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat)
	{
		auto data = loadData(filePath);
		auto alloc = getAllocator();
		return bimg::imageParse(alloc, data->ptr(), (uint32_t)data->size(), bimg::TextureFormat::Enum(dstFormat));
	}

	AssetContext& AssetContext::get()
	{
		static AssetContext instance;
		return instance;
	}

	AssetContext::AssetContext()
		: _impl(std::make_unique<AssetContextImpl>())
	{
	}

	bgfx::ShaderHandle AssetContext::loadShader(const std::string& name)
	{
		return _impl->loadShader(name);
	}

	bgfx::ProgramHandle AssetContext::loadProgram(const std::string& vsName, const std::string& fsName)
	{
		return _impl->loadProgram(vsName, fsName);
	}

	bgfx::TextureHandle AssetContext::loadTexture(const std::string& name, uint64_t flags, uint8_t skip, bgfx::TextureInfo* info, bimg::Orientation::Enum* orientation)
	{
		return _impl->loadTexture(name, flags, skip, info, orientation);
	}

	bimg::ImageContainer* AssetContext::loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat)
	{
		return _impl->loadImage(filePath, dstFormat);
	}

	AssetContextImpl& AssetContext::getImpl()
	{
		return *_impl;
	}

	const AssetContextImpl& AssetContext::getImpl() const
	{
		return *_impl;
	}
}
