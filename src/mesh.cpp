#include <darmok/mesh.hpp>
#include <darmok/vertex.hpp>
#include <darmok/shape.hpp>
#include <darmok/data.hpp>

namespace darmok
{
	uint64_t MeshConfig::getFlags() const noexcept
	{
		uint64_t flags = 0;
		if (index32)
		{
			flags |= BGFX_BUFFER_INDEX32;
		}
		return flags;
	}

	static size_t getMeshIndexSize(bool index32) noexcept
	{
		if (index32)
		{
			return 4;
		}
		return sizeof(VertexIndex);
	}

	size_t MeshConfig::getIndexSize() const noexcept
	{
		return getMeshIndexSize(index32);
	}

	std::shared_ptr<IMesh> IMesh::create(MeshType type, const bgfx::VertexLayout& layout, const DataView& vertices, Config config) noexcept
	{
		switch (type)
		{
		case MeshType::Dynamic:
			return std::make_shared<DynamicMesh>(layout, vertices, config);
		case MeshType::Transient:
			return std::make_shared<TransientMesh>(layout, vertices, config.index32);
		default:
			return std::make_shared<Mesh>(layout, vertices, config);
		}
	}

	std::shared_ptr<IMesh> IMesh::create(MeshType type, const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config) noexcept
	{
		switch (type)
		{
		case MeshType::Dynamic:
			return std::make_shared<DynamicMesh>(layout, vertices, indices, config);
		case MeshType::Transient:
			return std::make_shared<TransientMesh>(layout, vertices, indices, config.index32);
		default:
			return std::make_shared<Mesh>(layout, vertices, indices, config);
		}
	}

	static std::string getMeshDescription(const std::string& name, size_t vertNum, size_t idxNum, const bgfx::VertexLayout& layout) noexcept
	{
		auto stride = layout.getStride();
		return name + "(" + std::to_string(vertNum) + " vertices, "
			+ std::to_string(stride) + " stride, "
			+ std::to_string(idxNum) + " indices)";
	}

	Mesh::Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config) noexcept
		: _layout(layout)
		, _vertexBuffer{ bgfx::kInvalidHandle }
		, _indexBuffer{ bgfx::kInvalidHandle }
		, _vertNum(vertices.size() / layout.getStride())
		, _idxNum(indices.size() / config.getIndexSize())
	{
		auto flags = config.getFlags();
		// TODO: do we need to always copy the memory?
		_vertexBuffer = bgfx::createVertexBuffer(vertices.copyMem(), layout, flags);
		if (!indices.empty())
		{
			_indexBuffer = bgfx::createIndexBuffer(indices.copyMem(), flags);
		}
	}

	Mesh::Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, Config config) noexcept
		: Mesh(layout, vertices, DataView(), config)
	{
	}

	const bgfx::VertexLayout& Mesh::getVertexLayout() const noexcept
	{
		return _layout;
	}

	Mesh::~Mesh() noexcept
	{
		if (isValid(_vertexBuffer))
		{
			bgfx::destroy(_vertexBuffer);
		}
		if (isValid(_indexBuffer))
		{
			bgfx::destroy(_indexBuffer);
		}
	}

	Mesh::Mesh(Mesh&& other) noexcept
		: _layout(other._layout)
		, _vertexBuffer(other._vertexBuffer)
		, _indexBuffer(other._indexBuffer)
		, _vertNum(other._vertNum)
		, _idxNum(other._idxNum)
	{
		other._vertexBuffer.idx = bgfx::kInvalidHandle;
		other._indexBuffer.idx = bgfx::kInvalidHandle;
		other._vertNum = 0;
		other._idxNum = 0;
	}

	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		_layout = other._layout;
		_vertexBuffer = other._vertexBuffer;
		_indexBuffer = other._indexBuffer;
		_vertNum = other._vertNum;
		_idxNum = other._idxNum;
		other._vertexBuffer.idx = bgfx::kInvalidHandle;
		other._indexBuffer.idx = bgfx::kInvalidHandle;
		other._vertNum = 0;
		other._idxNum = 0;
		return *this;
	}

	bgfx::VertexBufferHandle Mesh::getVertexHandle() const noexcept
	{
		return _vertexBuffer;
	}

	bgfx::IndexBufferHandle Mesh::getIndexHandle() const noexcept
	{
		return _indexBuffer;
	}

	std::string Mesh::to_string() const noexcept
	{
		return getMeshDescription("Mesh", _vertNum, _idxNum, getVertexLayout());
	}

	void Mesh::render(bgfx::Encoder& encoder, uint8_t vertexStream) const
	{
		if (!isValid(_vertexBuffer))
		{
			throw std::runtime_error("invalid mesh vertex buffer");
		}
		encoder.setVertexBuffer(vertexStream, _vertexBuffer);
		if (isValid(_indexBuffer))
		{
			encoder.setIndexBuffer(_indexBuffer);
		}
	}

	DynamicMesh::DynamicMesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, Config config) noexcept
		: _layout(layout)
		, _vertexBuffer{ bgfx::kInvalidHandle }
		, _indexBuffer{ bgfx::kInvalidHandle }
		, _vertNum(vertices.size() / layout.getStride())
		, _idxNum(indices.size() / config.getIndexSize())
		, _idxSize(config.getIndexSize())
	{
		uint64_t flags = config.getFlags();
		flags |= BGFX_BUFFER_ALLOW_RESIZE;
		_vertexBuffer = bgfx::createDynamicVertexBuffer(vertices.copyMem(), layout, flags);
		if (!indices.empty())
		{
			_indexBuffer = bgfx::createDynamicIndexBuffer(indices.copyMem(), flags);
		}
	}

	DynamicMesh::DynamicMesh(const bgfx::VertexLayout& layout, const DataView& vertices, Config config) noexcept
		: DynamicMesh(layout, vertices, DataView(), config)
	{
	}

	const bgfx::VertexLayout& DynamicMesh::getVertexLayout() const noexcept
	{
		return _layout;
	}

	DynamicMesh::~DynamicMesh() noexcept
	{
		if (isValid(_vertexBuffer))
		{
			bgfx::destroy(_vertexBuffer);
		}
		if (isValid(_indexBuffer))
		{
			bgfx::destroy(_indexBuffer);
		}
	}

	DynamicMesh::DynamicMesh(DynamicMesh&& other) noexcept
		: _layout(other._layout)
		, _vertexBuffer(other._vertexBuffer)
		, _indexBuffer(other._indexBuffer)
		, _vertNum(other._vertNum)
		, _idxNum(other._idxNum)
		, _idxSize(other._idxSize)
	{
		other._vertexBuffer.idx = bgfx::kInvalidHandle;
		other._indexBuffer.idx = bgfx::kInvalidHandle;
		other._vertNum = 0;
		other._idxNum = 0;
	}

	DynamicMesh& DynamicMesh::operator=(DynamicMesh&& other) noexcept
	{
		_layout = other._layout;
		_vertexBuffer = other._vertexBuffer;
		_indexBuffer = other._indexBuffer;
		_vertNum = other._vertNum;
		_idxNum = other._idxNum;
		_idxSize = other._idxSize;
		other._vertexBuffer.idx = bgfx::kInvalidHandle;
		other._indexBuffer.idx = bgfx::kInvalidHandle;
		other._vertNum = 0;
		other._idxNum = 0;
		return *this;
	}

	bgfx::DynamicVertexBufferHandle DynamicMesh::getVertexHandle() const noexcept
	{
		return _vertexBuffer;
	}

	bgfx::DynamicIndexBufferHandle DynamicMesh::getIndexHandle() const noexcept
	{
		return _indexBuffer;
	}

	std::string DynamicMesh::to_string() const noexcept
	{
		return getMeshDescription("DynamicMesh", _vertNum, _idxNum, getVertexLayout());
	}

	void DynamicMesh::updateVertices(const DataView& data, size_t offset) noexcept
	{
		bgfx::DynamicVertexBufferHandle handle{ _vertexBuffer };
		bgfx::update(handle, offset, data.copyMem());
		_vertNum = data.size() / _layout.getStride();
	}

	void DynamicMesh::updateIndices(const DataView& data, size_t offset) noexcept
	{
		bgfx::DynamicIndexBufferHandle handle{ _indexBuffer };
		bgfx::update(handle, offset, data.copyMem());
		_idxNum = data.size() / _idxSize;
	}

	void DynamicMesh::render(bgfx::Encoder& encoder, uint8_t vertexStream) const
	{
		if (!isValid(_vertexBuffer))
		{
			throw std::runtime_error("invalid mesh vertex buffer");
		}
		encoder.setVertexBuffer(vertexStream, _vertexBuffer);
		if (isValid(_indexBuffer))
		{
			encoder.setIndexBuffer(_indexBuffer);
		}
	}

	TransientMesh::TransientMesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, bool index32)
		: _layout(layout)
		, _vertNum(vertices.size() / layout.getStride())
		, _idxNum(indices.size() / getMeshIndexSize(index32))
	{
		if (!bgfx::getAvailTransientVertexBuffer(_vertNum, layout))
		{
			throw std::runtime_error("not enought transient vertex buffer space");
		}
		bgfx::allocTransientVertexBuffer(&_vertexBuffer, _vertNum, layout);
		bx::memCopy(_vertexBuffer.data, vertices.ptr(), vertices.size());
		if (!indices.empty())
		{
			if (!bgfx::getAvailTransientIndexBuffer(_idxNum, index32))
			{
				throw std::runtime_error("not enought transient index buffer space");
			}
			bgfx::allocTransientIndexBuffer(&_indexBuffer, _idxNum, index32);
			bx::memCopy(_indexBuffer.data, indices.ptr(), indices.size());
		}
	}

	TransientMesh::TransientMesh(const bgfx::VertexLayout& layout, const DataView& vertices, bool index32)
		: TransientMesh(layout, vertices, DataView(), index32)
	{
	}

	TransientMesh::TransientMesh(TransientMesh&& other) noexcept
		: _layout(other._layout)
		, _vertexBuffer(std::move(other._vertexBuffer))
		, _indexBuffer(std::move(other._indexBuffer))
		, _vertNum(other._vertNum)
		, _idxNum(other._idxNum)
	{
		other._vertNum = 0;
		other._idxNum = 0;
	}

	TransientMesh& TransientMesh::operator=(TransientMesh&& other) noexcept
	{
		_layout = other._layout;
		_vertexBuffer = std::move(other._vertexBuffer);
		_indexBuffer = std::move(other._indexBuffer);
		_vertNum = other._vertNum;
		_idxNum = other._idxNum;
		other._vertNum = 0;
		other._idxNum = 0;
		return *this;
	}

	std::string TransientMesh::to_string() const noexcept
	{
		return getMeshDescription("TransientMesh", _vertNum, _idxNum, getVertexLayout());
	}

	void TransientMesh::render(bgfx::Encoder& encoder, uint8_t vertexStream) const
	{
		encoder.setVertexBuffer(vertexStream, &_vertexBuffer);
		if (_idxNum > 0)
		{
			encoder.setIndexBuffer(&_indexBuffer);
		}
	}

	const bgfx::VertexLayout& TransientMesh::getVertexLayout() const noexcept
	{
		return _layout;
	}

	MeshCreator::MeshCreator(const bgfx::VertexLayout& layout) noexcept
		: layout(layout)
		, config{}
	{
	}

	std::shared_ptr<IMesh> MeshCreator::createMesh(const MeshData& meshData) noexcept
	{
		return createMesh(meshData, config);
	}

	std::shared_ptr<IMesh> MeshCreator::createMesh(const MeshData& meshData, const MeshCreationConfig& cfg) noexcept
	{
		VertexDataWriter writer(layout, meshData.positions.size());
		uint32_t i = 0;
		for (auto& pos : meshData.positions)
		{
			auto v = cfg.scale * (pos + cfg.offset);
			writer.write(bgfx::Attrib::Position, i++, v);
		}
		i = 0;
		for (auto& texCoord : meshData.texCoords)
		{
			auto v = cfg.textureScale * (texCoord + cfg.textureOffset);
			writer.write(bgfx::Attrib::TexCoord0, i++, v);
		}

		writer.write(bgfx::Attrib::Normal, meshData.normals);
		auto vertexData = writer.finish();
		DataView vertDataView(vertexData);
		DataView idxDataView(meshData.indices);

		return IMesh::create(config.type, layout, vertDataView, idxDataView);
	}

	std::shared_ptr<IMesh> MeshCreator::createCube() noexcept
	{
		return createCube(Cube::standard());
	}

	std::shared_ptr<IMesh> MeshCreator::createCube(const Cube& cube) noexcept
	{
		const static MeshData data = {
			{
				{ 1,  1,  1 }, { 0,  1,  1 }, { 0,  0,  1 }, { 1,  0,  1 },
				{ 1,  1,  0 }, { 1,  0,  0 }, { 0,  0,  0 }, { 0,  1,  0 },
				{ 1,  1,  1 }, { 1,  1,  0 }, { 0,  1,  0 }, { 0,  1,  1 },
				{ 1,  0,  1 }, { 0,  0,  1 }, { 0,  0,  0 }, { 1,  0,  0 },
				{ 1,  1,  1 }, { 1,  0,  1 }, { 1,  0,  0 }, { 1,  1,  0 },
				{ 0,  1,  1 }, { 0,  1,  0 }, { 0,  0,  0 }, { 0,  0,  1 },
			},
			{
				{  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 },
				{  0,  0, -1 }, {  0,  0, -1 }, {  0,  0, -1 }, {  0,  0, -1 },
				{  0,  1,  0 }, {  0,  1,  0 }, {  0,  1,  0 }, {  0,  1,  0 },
				{  0, -1,  0 }, {  0, -1,  0 }, {  0, -1,  0 }, {  0, -1,  0 },
				{  1,  0,  0 }, {  1,  0,  0 }, {  1,  0,  0 }, {  1,  0,  0 },
				{ -1,  0,  0 }, { -1,  0,  0 }, { -1,  0,  0 }, { -1,  0,  0 },
			},
			{
				{ 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 },
				{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
				{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
				{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
				{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
				{ 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 },
			},
			{
				 0,  1,  2,  2,  3,  0,
				 4,  5,  6,  6,  7,  4,
				 8,  9, 10, 10, 11,  8,
				12, 13, 14, 14, 15, 12,
				16, 17, 18, 18, 19, 16,
				20, 21, 22, 22, 23, 20,
			}
		};
		auto cfg = config;
		cfg.scale *= cube.size;
		cfg.offset += cube.origin - glm::vec3(0.5f);
		return createMesh(data, cfg);
	}

	const MeshData MeshCreator::_rectangleMeshData = {
		{
			{ 1, 1, 0 }, { 1, 0, 0 }, { 0, 0, 0 }, { 0, 1, 0 },
		},
		{
			{ 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 }, { 0, 0, -1 },
		},
		{
			{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
		}
	};

	std::shared_ptr<IMesh> MeshCreator::createRectangleMesh(const bgfx::VertexLayout& layout, const MeshData& data, const Rectangle& rect) noexcept
	{
		auto cfg = config;
		cfg.scale *= glm::vec3(rect.size, 0);
		cfg.offset += glm::vec3(rect.origin - glm::vec2(0.5F), 0);
		return createMesh(data, cfg);
	}

	std::shared_ptr<IMesh> MeshCreator::createRectangle(const Rectangle& rect) noexcept
	{
		MeshData data = _rectangleMeshData;
		data.indices = { 0, 1, 2, 2, 3, 0 };
		return createRectangleMesh(layout, data, rect);
	}

	std::shared_ptr<IMesh> MeshCreator::createLineRectangle(const Rectangle& rect) noexcept
	{
		MeshData data = _rectangleMeshData;
		data.indices = { 0, 1, 1, 2, 2, 3, 3, 0 };
		return createRectangleMesh(layout, data, rect);
	}

	std::shared_ptr<IMesh> MeshCreator::createSphere(int lod) noexcept
	{
		return createSphere(Sphere::standard());
	}

	std::shared_ptr<IMesh> MeshCreator::createRectangle() noexcept
	{
		return createRectangle(Rectangle::standard());
	}

	std::shared_ptr<IMesh> MeshCreator::createLineRectangle() noexcept
	{
		return createLineRectangle(Rectangle::standard());
	}

	std::shared_ptr<IMesh> MeshCreator::createSphere(const Sphere& sphere, int lod) noexcept
	{
		auto rings = lod;
		auto sectors = lod;
		float texScale = 1.f;

		float R = 1.f / (float)(rings - 1);
		float S = 1.f / (float)(sectors - 1);
		size_t n = rings * sectors;
		auto pi = glm::pi<float>();

		MeshData data;
		{
			data.positions.reserve(n);
			data.normals.reserve(n);
			data.texCoords.reserve(n);
			for (int r = 0; r < rings; r++)
			{
				for (int s = 0; s < sectors; s++)
				{
					auto u = s * S;
					auto v = r * R;
					auto theta = u * 2.0f * pi;
					auto rho = v * pi;
					auto pos = glm::vec3(
						cos(theta) * sin(rho),
						sin((0.5F * pi) + rho),
						sin(theta) * sin(rho)
					);
					data.positions.push_back(pos);
					data.normals.push_back(pos);
					data.texCoords.push_back(glm::vec2(
						u * texScale,
						v * texScale
					));
				}
			}
		}

		{
			data.indices.reserve(n * 6);
			for (VertexIndex r = 0; r < rings - 1; r++)
			{
				for (VertexIndex s = 0; s < sectors - 1; s++)
				{
					data.indices.push_back(r * sectors + s);
					data.indices.push_back(r * sectors + (s + 1));
					data.indices.push_back((r + 1) * sectors + (s + 1));
					data.indices.push_back((r + 1) * sectors + (s + 1));
					data.indices.push_back((r + 1) * sectors + s);
					data.indices.push_back(r * sectors + s);
				}
			}
		}
		auto cfg = config;
		cfg.scale *= glm::vec3(sphere.radius);
		cfg.offset += sphere.origin;
		return createMesh(data, cfg);
	}

	std::shared_ptr<IMesh> MeshCreator::createRay(const Ray& ray) noexcept
	{
		return createLine(ray.toLine());
	}

	std::shared_ptr<IMesh> MeshCreator::createLine(const Line& line) noexcept
	{
		return createLines({ line });
	}

	std::shared_ptr<IMesh> MeshCreator::createLines(const std::vector<Line>& lines) noexcept
	{
		MeshData data;
		VertexIndex i = 0;
		for (auto& line : lines)
		{
			data.positions.push_back(line.points[0]);
			data.positions.push_back(line.points[1]);
			data.normals.emplace_back();
			data.normals.emplace_back();
			data.texCoords.emplace_back(0, 0);
			data.texCoords.emplace_back(1, 1);
			data.indices.emplace_back(i++);
			data.indices.emplace_back(i++);
		}
		return createMesh(data, config);
	}

	std::shared_ptr<IMesh> MeshCreator::createBone() noexcept
	{
		const float kInter = .2f;
		const std::vector<glm::vec3> pos = {
			{ 1.f, 0.f, 0.f		}, { kInter, .1f, .1f },
			{ kInter, .1f, -.1f }, { kInter, -.1f, -.1f },
			{ kInter, -.1f, .1f }, { 0.f, 0.f, 0.f }
		};
		const std::vector<glm::vec2> tex = {
			{ 0.f, 0.f		}, { .1f, .1f },
			{ .1f, -.1f }, { -.1f, -.1f },
			{ -.1f, .1f }, { 0.f, 0.f }
		};
		const std::vector<glm::vec3> norm = {
			glm::normalize(glm::cross(pos[2] - pos[1], pos[2] - pos[0])),
			glm::normalize(glm::cross(pos[1] - pos[2], pos[1] - pos[5])),
			glm::normalize(glm::cross(pos[3] - pos[2], pos[3] - pos[0])),
			glm::normalize(glm::cross(pos[2] - pos[3], pos[2] - pos[5])),
			glm::normalize(glm::cross(pos[4] - pos[3], pos[4] - pos[0])),
			glm::normalize(glm::cross(pos[3] - pos[4], pos[3] - pos[5])),
			glm::normalize(glm::cross(pos[1] - pos[4], pos[1] - pos[0])),
			glm::normalize(glm::cross(pos[4] - pos[1], pos[4] - pos[5]))
		};
		const MeshData data = { 
			{ 
				pos[0], pos[2], pos[1], pos[5], pos[1], pos[2], pos[0], pos[3],
				pos[2], pos[5], pos[2], pos[3], pos[0], pos[4], pos[3], pos[5],
				pos[3], pos[4], pos[0], pos[1], pos[4], pos[5], pos[4], pos[1],
			},
			{
				 norm[0], norm[0], norm[0], norm[1], norm[1], norm[1], norm[2], norm[2],
				 norm[2], norm[3], norm[3], norm[3], norm[4], norm[4], norm[4], norm[5],
				 norm[5], norm[5], norm[6], norm[6], norm[6], norm[7], norm[7], norm[7],
			},
			{
				tex[0], tex[1], tex[1], tex[0], tex[2], tex[2], tex[0], tex[3],
				tex[3], tex[0], tex[4], tex[4], tex[2], tex[5], tex[2], tex[3],
				tex[5], tex[3], tex[4], tex[5], tex[4], tex[1], tex[5], tex[1],
			}
		};
		return createMesh(data, config);
	}
}