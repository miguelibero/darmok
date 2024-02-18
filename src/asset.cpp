#include "asset.hpp"

#include <darmok/asset.hpp>
#include <darmok/utils.hpp>
#include <darmok/model.hpp>

#include <stdexcept>
#include <filesystem>

#include <bx/filepath.h>
#include <bimg/decode.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace darmok
{
	static std::string addBasePath(const std::string& path, const std::string& basePath)
	{
		std::filesystem::path p(path);

		if (p.is_absolute())
		{
			return path;
		}
		return (std::filesystem::path(basePath) / p).string();
	}

	static std::string addCurrentPath(const std::string& path, const std::string& currentPath)
	{
		std::filesystem::path p(path);
		if (p.has_parent_path())
		{
			return path;
		}
		auto parentPath = std::filesystem::path(currentPath).parent_path();
		return (parentPath / p).string();
	}

	void FileReader::setBasePath(const std::string& basePath)
	{
		_basePath = basePath;
	}

	bool FileReader::open(const bx::FilePath& filePath, bx::Error* err)
	{
		auto absFilePath = addBasePath(filePath.getCPtr(), _basePath);
		return super::open(absFilePath.c_str(), err);
	}

	void FileWriter::setBasePath(const std::string& basePath)
	{
		_basePath = basePath;
	}

	bool FileWriter::open(const bx::FilePath& filePath, bool append, bx::Error* err)
	{
		auto absFilePath = addBasePath(filePath.getCPtr(), _basePath);
		return super::open(absFilePath.c_str(), append, err);
	}

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

	uint32_t Image::getWidth() const
	{
		return _container->m_width;
	}

	uint32_t Image::getHeight() const
	{
		return _container->m_height;
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

	Texture::Texture(std::shared_ptr<Image> img, uint64_t flags, const std::string& name)
		: _img(img)
		, _handle{ bgfx::kInvalidHandle }
		, _type(TextureType::Unknown)
	{
		init(flags);
		if (!name.empty())
		{
			bgfx::setName(_handle, name.c_str());
		}
	}

	Texture::Texture(std::shared_ptr<Image> img, const std::string& name)
		: _img(img)
		, _handle{ bgfx::kInvalidHandle }
		, _type(TextureType::Unknown)
	{
		init(defaultTextureCreationFlags);
		if (!name.empty())
		{
			bgfx::setName(_handle, name.c_str());
		}
	}


	void Texture::init(uint64_t flags)
	{
		const auto mem = _img->makeRef();
		auto format = bgfx::TextureFormat::Enum(_img->getFormat());
		auto hasMips = 1 < _img->getMipCount();
		auto w = uint16_t(_img->getWidth());
		auto h = uint16_t(_img->getHeight());
		auto layers = _img->getLayerCount();
		if (_img->isCubeMap())
		{
			_handle = bgfx::createTextureCube(w, hasMips, layers, format, flags, mem);
			_type = TextureType::CubeMap;
		}
		else if (1 < _img->getDepth())
		{
			_handle = bgfx::createTexture3D(w, h, uint16_t(_img->getDepth()), hasMips, format, flags, mem);
			_type = TextureType::Texture3D;
		}
		else if (bgfx::isTextureValid(0, false, layers, format, flags))
		{
			_handle = bgfx::createTexture2D(w, h, hasMips, layers, format, flags, mem);
			_type = TextureType::Texture2D;
		}
	}

	const bgfx::TextureHandle& Texture::getHandle() const
	{
		return _handle;
	}

	std::shared_ptr<Image> Texture::getImage() const
	{
		return _img;
	}

	void Texture::releaseImage()
	{
		_img = nullptr;
		// TODO: is there a way to check if the image was uploaded?
	}

	TextureType Texture::getType() const
	{
		return _type;
	}

	Program::Program(const bgfx::ProgramHandle& handle)
		: _handle(handle)
	{
	}

	const bgfx::ProgramHandle& Program::getHandle() const
	{
		return _handle;
	}

	TextureBounds TextureAtlasElement::getBounds() const
	{
		return {
			originalSize,
			originalPosition
		};
	}

	TextureBounds TextureAtlas::getBounds(const std::string& prefix) const
	{
		TextureBounds bounds{};
		for (auto& elm : elements)
		{
			if (elm.name.starts_with(prefix))
			{
				// TODO: algorithm to combine bounds
				bounds = elm.getBounds();
			}
		}
		return bounds;
	}

	TextureAtlasElement* TextureAtlas::getElement(const std::string& name)
	{
		for (auto& elm : elements)
		{
			if (elm.name == name)
			{
				return &elm;
			}
		}
		return nullptr;
	}

	const TextureAtlasElement* TextureAtlas::getElement(const std::string& name) const
	{
		for (auto& elm : elements)
		{
			if (elm.name == name)
			{
				return &elm;
			}
		}
		return nullptr;
	}

	FileDataLoader::FileDataLoader(bx::FileReaderI* fileReader, bx::AllocatorI* alloc)
		: _fileReader(fileReader)
		, _allocator(alloc)
	{
	}

	std::shared_ptr<Data> FileDataLoader::operator()(const std::string& filePath)
	{
		if (!bx::open(_fileReader, filePath.c_str()))
		{
			throw std::runtime_error("Failed to load data from file \"" + filePath + "\".");
		}

		auto size = bx::getSize(_fileReader);
		void* data = nullptr;
		if (_allocator)
		{
			data = bx::alloc(_allocator, size);
		}
		else
		{
			data = std::malloc(size);
		}
		bx::read(_fileReader, data, (int32_t)size, bx::ErrorAssert{});
		bx::close(_fileReader);
		return std::make_shared<Data>(data, size, _allocator);
	}

	DataProgramLoader::DataProgramLoader(IDataLoader& dataLoader)
		: _dataLoader(dataLoader)
	{
	}

	std::string DataProgramLoader::getShaderExt()
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

	bgfx::ShaderHandle DataProgramLoader::loadShader(const std::string& name)
	{
		std::string dataName = name + "." + getShaderExt() + ".bin";
		auto data = _dataLoader(dataName);
		if (data == nullptr || data->empty())
		{
			throw std::runtime_error("got empty data");
		}
		bgfx::ShaderHandle handle = bgfx::createShader(data->makeRef());
		bgfx::setName(handle, name.c_str());
		return handle;
	}

	std::shared_ptr<Program> DataProgramLoader::operator()(const std::string& vertexName, const std::string& fragmentName)
	{
		bgfx::ShaderHandle vsh = loadShader(vertexName);
		bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
		if (!fragmentName.empty())
		{
			fsh = loadShader(fragmentName);
		}
		auto handle = bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		return std::make_shared<Program>(handle);
	}

	DataImageLoader::DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI* alloc)
		: _dataLoader(dataLoader)
		, _allocator(alloc)
	{
	}

	std::shared_ptr<Image> DataImageLoader::operator()(const std::string& name)
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

	ImageTextureLoader::ImageTextureLoader(IImageLoader& imgLoader)
		: _imgLoader(imgLoader)
	{
	}

	std::shared_ptr<Texture> ImageTextureLoader::operator()(const std::string& name, uint64_t flags)
	{
		auto img = _imgLoader(name);
		if (img == nullptr || img->empty())
		{
			throw std::runtime_error("got empty image");
		}

		auto tex = std::make_shared<Texture>(img, name);

		if (!bgfx::isValid(tex->getHandle()))
		{
			throw std::runtime_error("could not load texture");
		}

		return tex;
	}

	TexturePackerTextureAtlasLoader::TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader)
		: _dataLoader(dataLoader)
		, _textureLoader(textureLoader)
	{
	}

	std::pair<int, size_t> TexturePackerTextureAtlasLoader::readXmlValueInt(const std::string& str, size_t i)
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

	std::pair<std::optional<TextureVec2>, size_t> TexturePackerTextureAtlasLoader::readXmlValueVec(const std::string& str, size_t i)
	{
		auto p = readXmlValueInt(str, i);
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
		p = readXmlValueInt(str, i);
		if (p.first < 0)
		{
			return { std::nullopt, std::string::npos };
		}
		v.y = p.first;
		i = p.second;
		return { v, i };
	}

	TextureAtlasElement TexturePackerTextureAtlasLoader::loadElement(pugi::xml_node& xml)
	{
		std::vector<TextureAtlasVertex> vertices;

		std::string vertPosVals = xml.child("vertices").text().get();
		std::string vertTexVals = xml.child("verticesUV").text().get();
		size_t pi = 0, ti = 0;

		while (pi != std::string::npos && ti != std::string::npos)
		{
			auto pp = readXmlValueVec(vertPosVals, pi);
			if (!pp.first.has_value())
			{
				break;
			}
			auto tp = readXmlValueVec(vertTexVals, ti);
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
			auto p = readXmlValueInt(vertIdxVals, ii);
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
			std::string(xml.attribute("r").value()) == "y"
		};
	}

	std::shared_ptr<TextureAtlas> TexturePackerTextureAtlasLoader::operator()(const std::string& name, uint64_t flags)
	{
		auto data = _dataLoader(name);
		if (data == nullptr || data->empty())
		{
			throw std::runtime_error("got empty data");
		}

		pugi::xml_document doc;
		auto result = doc.load_buffer(data->ptr(), data->size());
		if (result.status != pugi::status_ok)
		{
			throw std::runtime_error(result.description());
		}
		auto atlasXml = doc.child("TextureAtlas");
		auto imagePath = atlasXml.attribute("imagePath").value();
		auto atlasFullName = addCurrentPath(imagePath, name);
		auto texture = _textureLoader(atlasFullName, flags);

		TextureVec2 size{
			atlasXml.attribute("width").as_int(),
			atlasXml.attribute("height").as_int(),
		};

		static const char* spriteTag = "sprite";
		std::vector<TextureAtlasElement> elements;
		for (pugi::xml_node spriteXml = atlasXml.child(spriteTag); spriteXml; spriteXml = spriteXml.next_sibling(spriteTag))
		{
			elements.push_back(loadElement(spriteXml));
		}

		return std::make_shared<TextureAtlas>(texture, elements, size);
	}

	AssimpModelLoader::AssimpModelLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader)
		: _dataLoader(dataLoader)
		, _textureLoader(textureLoader)
	{
	}

	std::shared_ptr<Model> AssimpModelLoader::operator()(const std::string& name)
	{
		auto data = _dataLoader(name);
		if (data == nullptr || data->empty())
		{
			throw std::runtime_error("got empty data");
		}
		unsigned int flags = aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType
			;

		auto scene = _importer.ReadFileFromMemory(data->ptr(), data->size(), flags, name.c_str());

		if (scene == nullptr)
		{
			throw std::runtime_error(_importer.GetErrorString());
		}

		return std::make_shared<Model>(scene);
	}

	AssetContextImpl::AssetContextImpl()
		: _dataLoader(&_fileReader, &_allocator)
		, _imageLoader(_dataLoader, &_allocator)
		, _programLoader(_dataLoader)
		, _textureLoader(_imageLoader)
		, _textureAtlasLoader(_dataLoader, _textureLoader)
		, _modelLoader(_dataLoader, _textureLoader)
	{
	}

	bx::AllocatorI* AssetContextImpl::getAllocator() noexcept
	{
		return &_allocator;
	}
	
	IImageLoader& AssetContextImpl::getImageLoader() noexcept
	{
		return _imageLoader;
	}

	IProgramLoader& AssetContextImpl::getProgramLoader() noexcept
	{
		return _programLoader;
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

	void AssetContextImpl::setBasePath(const std::string& path) noexcept
	{
		_fileReader.setBasePath(path);
		_fileWriter.setBasePath(path);
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

	IImageLoader& AssetContext::getImageLoader() noexcept
	{
		return _impl->getImageLoader();
	}

	IProgramLoader& AssetContext::getProgramLoader() noexcept
	{
		return _impl->getProgramLoader();
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

	AssetContextImpl& AssetContext::getImpl() noexcept
	{
		return *_impl;
	}

	const AssetContextImpl& AssetContext::getImpl() const noexcept
	{
		return *_impl;
	}
}
