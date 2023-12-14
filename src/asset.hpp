#pragma once

#include <string>
#include <bx/file.h>
#include <bx/pixelformat.h>
#include <bimg/bimg.h>
#include <bgfx/bgfx.h>


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

	class Data final
	{
	public:
		Data(void* ptr, uint64_t size, bx::AllocatorI* alloc = nullptr) noexcept;
		~Data() noexcept;
		Data(Data&& other) noexcept;
		Data& operator=(Data&& other) noexcept;
		void* ptr() const noexcept;
		uint64_t size() const noexcept;
		bool empty() const noexcept;
	private:
		Data(const Data& other) = delete;
		Data& operator=(const Data& other) = delete;
		void* _ptr;
		uint64_t _size;
		bx::AllocatorI* _alloc;
	};

	class Image;
	struct TextureWithInfo;
	struct TextureAtlas;

	class AssetContextImpl final
	{
	public:
		bgfx::ShaderHandle loadShader(const std::string& name);
		bgfx::ProgramHandle loadProgram(const std::string& vertexName, const std::string& fragmentName = "");
		bgfx::TextureHandle loadTexture(const std::string& name, uint64_t flags);
		TextureWithInfo loadTextureWithInfo(const std::string& name, uint64_t flags);
		Image loadImage(const std::string& filePath, bgfx::TextureFormat::Enum dstFormat = bgfx::TextureFormat::Count);
		TextureAtlas loadAtlas(const std::string& filePath, uint64_t flags);

		bx::AllocatorI* getAllocator() noexcept;
	private:

		TextureWithInfo loadTexture(const std::string& filePath, uint64_t flags, bool loadInfo);

		FileReader _fileReader;
		FileWriter _fileWriter;
		bx::DefaultAllocator _allocator;

		Data loadData(const std::string& filePath);
		const bgfx::Memory* loadMem(const std::string& filePath);

		bx::FileReaderI* getFileReader() noexcept;
		bx::FileWriterI* getFileWriter() noexcept;

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
