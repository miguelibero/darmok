#pragma once

#include <darmok/asset.hpp>

#include <bx/file.h>
#include <bx/pixelformat.h>

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

	class Data
	{
	public:
		Data(void* ptr, uint64_t size, bx::AllocatorI* alloc = nullptr);
		~Data();
		void* ptr() const;
		uint64_t size() const;
		bool empty() const;
	private:
		void* _ptr;
		uint64_t _size;
		bx::AllocatorI* _alloc;
	};

	class AssetContextImpl final
	{
	public:
		bgfx::ShaderHandle loadShader(const std::string& name);
		bgfx::ProgramHandle loadProgram(const std::string& vsName, const std::string& fsName);
		bgfx::TextureHandle loadTexture(const std::string& name, uint64_t flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE, uint8_t skip = 0, bgfx::TextureInfo* info = nullptr, bimg::Orientation::Enum* orientation = nullptr);
		bimg::ImageContainer* loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat);
	
		bx::AllocatorI* getAllocator();
	private:
		FileReader _fileReader;
		FileWriter _fileWriter;
		bx::DefaultAllocator _allocator;

		std::unique_ptr<Data> loadData(const std::string& filePath);
		const bgfx::Memory* loadMem(const std::string& filePath);

		bx::FileReaderI* getFileReader();
		bx::FileWriterI* getFileWriter();

	};

	/// Returns true if both internal transient index and vertex buffer have
	/// enough space.
	///
	/// @param[in] _numVertices Number of vertices.
	/// @param[in] _layout Vertex layout.
	/// @param[in] _numIndices Number of indices.
	///
	inline bool checkAvailTransientBuffers(uint32_t numVertices, const bgfx::VertexLayout& layout, uint32_t numIndices)
	{
		return numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, layout)
			&& (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices))
			;
	}

	///
	inline uint32_t encodeNormalRgba8(float x, float y = 0.0f, float z = 0.0f, float w = 0.0f)
	{
		const float src[] =
		{
			x * 0.5f + 0.5f,
			y * 0.5f + 0.5f,
			z * 0.5f + 0.5f,
			w * 0.5f + 0.5f,
		};
		uint32_t dst;
		bx::packRgba8(&dst, src);
		return dst;
	}
}
