#pragma once

#include <string>
#include <optional>

#include <darmok/asset.hpp>

#include <bx/file.h>
#include <bx/pixelformat.h>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>

#include <assimp/Importer.hpp>
#include <pugixml.hpp>


namespace darmok
{
	class FileReader final : public bx::FileReader
	{
		typedef bx::FileReader super;
	public:
		void setBasePath(const std::string& basePath);
		bool open(const bx::FilePath& filePath, bx::Error* err) override;
	private:
		std::string _basePath;
	};

	class FileWriter final : public bx::FileWriter
	{
		typedef bx::FileWriter super;
	public:
		void setBasePath(const std::string& basePath);
		virtual bool open(const bx::FilePath& filePath, bool append, bx::Error* err) override;
	private:
		std::string _basePath;
	};

	class Data;
	class Image;
	struct Texture;
	struct TextureAtlas;
	class Model;

	class FileDataLoader final : public IDataLoader
	{
	public:
		FileDataLoader(bx::FileReaderI* fileReader, bx::AllocatorI* alloc = nullptr);
		std::shared_ptr<Data> operator()(const std::string& filePath) override;
	private:
		bx::FileReaderI* _fileReader;
		bx::AllocatorI* _allocator;
	};

	class DataProgramLoader final : public IProgramLoader
	{
	public:
		DataProgramLoader(IDataLoader& dataLoader);
		std::shared_ptr<Program> operator()(const std::string& vertexName, const std::string& fragmentName = "") override;
	private:
		IDataLoader& _dataLoader;

		bgfx::ShaderHandle loadShader(const std::string& filePath);
		static std::string getShaderExt();
	};

	class DataImageLoader final : public IImageLoader
	{
	public:
		DataImageLoader(IDataLoader& dataLoader, bx::AllocatorI* alloc);
		std::shared_ptr<Image> operator()(const std::string& name, bimg::TextureFormat::Enum format = bimg::TextureFormat::Count) override;
	private:
		IDataLoader& _dataLoader;
		bx::AllocatorI* _allocator;
	};

	class ImageTextureLoader final : public ITextureLoader
	{
	public:
		ImageTextureLoader(IImageLoader& imgLoader);
		std::shared_ptr<Texture> operator()(const std::string& name, uint64_t flags = defaultTextureCreationFlags) override;
	private:
		IImageLoader& _imgLoader;
		bx::AllocatorI* _allocator;
	};

	class TexturePackerTextureAtlasLoader final : public ITextureAtlasLoader
	{
	public:
		TexturePackerTextureAtlasLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader);
		std::shared_ptr<TextureAtlas> operator()(const std::string& name, uint64_t flags = defaultTextureCreationFlags) override;
	private:
		IDataLoader& _dataLoader;
		ITextureLoader& _textureLoader;

		static std::pair<int, size_t> readXmlValueInt(const std::string& str, size_t i);
		static std::pair<std::optional<TextureVec2>, size_t> readXmlValueVec(const std::string& str, size_t i);
		static TextureAtlasElement loadElement(pugi::xml_node& xml);
	};

	class AssimpModelLoader final : public IModelLoader
	{
	public:
		AssimpModelLoader(IDataLoader& dataLoader, ITextureLoader& textureLoader);
		std::shared_ptr<Model> operator()(const std::string& name) override;
	private:
		Assimp::Importer _importer;
		IDataLoader& _dataLoader;
		ITextureLoader& _textureLoader;
	};

	class AssetContextImpl final
	{
	public:
		AssetContextImpl();
		IImageLoader& getImageLoader() noexcept;
		IProgramLoader& getProgramLoader() noexcept;
		ITextureLoader& getTextureLoader() noexcept;
		ITextureAtlasLoader& getTextureAtlasLoader() noexcept;
		IModelLoader& getModelLoader() noexcept;
		bx::AllocatorI* getAllocator() noexcept;
		void setBasePath(const std::string& path) noexcept;
	private:
		FileReader _fileReader;
		FileWriter _fileWriter;
		bx::DefaultAllocator _allocator;
		FileDataLoader _dataLoader;
		DataImageLoader _imageLoader;
		DataProgramLoader _programLoader;
		ImageTextureLoader _textureLoader;
		TexturePackerTextureAtlasLoader _textureAtlasLoader;
		AssimpModelLoader _modelLoader;
	};

}
