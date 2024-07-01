#include <darmok/mesh.hpp>
#include <darmok/vertex.hpp>
#include <darmok/shape.hpp>
#include <darmok/data.hpp>

namespace darmok
{
	uint16_t MeshConfig::getFlags() const noexcept
	{
		uint16_t flags = 0;
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

	std::unique_ptr<IMesh> IMesh::create(MeshType type, const bgfx::VertexLayout& layout, DataView vertices, Config config)
	{
		switch (type)
		{
		case MeshType::Dynamic:
			return std::make_unique<DynamicMesh>(layout, vertices, config);
		case MeshType::Transient:
			return std::make_unique<TransientMesh>(layout, vertices, config.index32);
		default:
			return std::make_unique<Mesh>(layout, vertices, config);
		}
	}

	std::unique_ptr<IMesh> IMesh::create(MeshType type, const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config)
	{
		switch (type)
		{
		case MeshType::Dynamic:
			return std::make_unique<DynamicMesh>(layout, vertices, indices, config);
		case MeshType::Transient:
			return std::make_unique<TransientMesh>(layout, vertices, indices, config.index32);
		default:
			return std::make_unique<Mesh>(layout, vertices, indices, config);
		}
	}

	static std::string getMeshDescription(const std::string& name, size_t vertNum, size_t idxNum, const bgfx::VertexLayout& layout) noexcept
	{
		auto stride = layout.getStride();
		return name + "(" + std::to_string(vertNum) + " vertices, "
			+ std::to_string(stride) + " stride, "
			+ std::to_string(idxNum) + " indices)";
	}

	Mesh::Mesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config) noexcept
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

	Mesh::Mesh(const bgfx::VertexLayout& layout, DataView vertices, Config config) noexcept
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

	DynamicMesh::DynamicMesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, Config config) noexcept
		: _layout(layout)
		, _vertexBuffer{ bgfx::kInvalidHandle }
		, _indexBuffer{ bgfx::kInvalidHandle }
		, _vertNum(vertices.size() / layout.getStride())
		, _idxNum(indices.size() / config.getIndexSize())
		, _idxSize(config.getIndexSize())
	{
		auto flags = config.getFlags() | BGFX_BUFFER_ALLOW_RESIZE;
		if (!vertices.empty())
		{
			_vertexBuffer = bgfx::createDynamicVertexBuffer(vertices.copyMem(), layout, flags);
		}
		if (!indices.empty())
		{
			_indexBuffer = bgfx::createDynamicIndexBuffer(indices.copyMem(), flags);
		}
	}

	DynamicMesh::DynamicMesh(const bgfx::VertexLayout& layout, DataView vertices, Config config) noexcept
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

	bool DynamicMesh::empty() const noexcept
	{
		return _vertNum == 0;
	}

	void DynamicMesh::updateVertices(DataView data, uint32_t offset) noexcept
	{
		bgfx::update(_vertexBuffer, offset, data.copyMem());
	}

	void DynamicMesh::updateIndices(DataView data, uint32_t offset) noexcept
	{
		bgfx::update(_indexBuffer, offset, data.copyMem());
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

	TransientMesh::TransientMesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, bool index32)
		: _layout(layout)
		, _vertNum(uint32_t(vertices.size() / layout.getStride()))
		, _idxNum(uint32_t(indices.size() / getMeshIndexSize(index32)))
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

	TransientMesh::TransientMesh(const bgfx::VertexLayout& layout, DataView vertices, bool index32)
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

	template<typename T>
	void doUpdateMeshIndexData(Data& data, const std::vector<VertexIndex>& indices, T offset)
	{
		std::vector<T> indices32;
		indices32.reserve(indices.size());
		for (auto& idx : indices)
		{
			indices32.push_back(offset + idx);
		}
		data = indices32;
	}

	template<typename T>
	void updateMeshIndexData(Data& data, const std::vector<VertexIndex>& indices, T offset)
	{
		doUpdateMeshIndexData(data, indices, offset);
	}


	template<>
	void updateMeshIndexData<VertexIndex>(Data& data, const std::vector<VertexIndex>& indices, VertexIndex offset)
	{
		if (offset == 0)
		{
			data = indices;
		}
		else
		{
			doUpdateMeshIndexData(data, indices, offset);
		}
	}

	const bgfx::VertexLayout& MeshData::getDefaultVertexLayout() noexcept
	{
		static bgfx::VertexLayout layout;
		if (layout.getStride() == 0)
		{
			layout.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.end();
		}
		return layout;
	}

	void MeshData::normalize() noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position = config.scale * (vertex.position + config.offset);
			vertex.texCoord = config.textureScale * (vertex.texCoord + config.textureOffset);
			vertex.color = config.color * vertex.color;
		}
		for (auto& index : indices)
		{
			index += config.indexOffset;
		}
		config.color = Colors::white();
		config.scale = glm::vec3(1);
		config.offset = glm::vec3(0);
		config.textureScale = glm::vec3(1);
		config.textureOffset = glm::vec3(0);
		config.indexOffset = 0;
	}

	void MeshData::denormalize(const Config& newConfig) noexcept
	{
		normalize();

		for (auto& vertex : vertices)
		{
			vertex.position = (vertex.position / newConfig.scale) - newConfig.offset;
			vertex.texCoord = (vertex.texCoord / newConfig.textureScale) - newConfig.textureOffset;
			vertex.color = vertex.color / newConfig.color;
		}
		for (auto& index : indices)
		{
			index -= config.indexOffset;
		}

		config.scale = newConfig.scale;
		config.offset = newConfig.offset;
		config.textureScale = newConfig.textureScale;
		config.textureOffset = newConfig.textureOffset;
		config.indexOffset = newConfig.indexOffset;
	}

	bool MeshData::empty() const noexcept
	{
		return vertices.empty();
	}

	void MeshData::clear() noexcept
	{
		vertices.clear();
		indices.clear();
	}

	void MeshData::exportData(const bgfx::VertexLayout& vertexLayout, Data& vertexData, Data& indexData) const noexcept
	{
		VertexDataWriter writer(vertexLayout, uint32_t(vertices.size()));
		uint32_t i = 0;
		for (auto& vertex : vertices)
		{
			if (vertexLayout.has(bgfx::Attrib::Position))
			{
				auto v = config.scale * (vertex.position + config.offset);
				writer.write(bgfx::Attrib::Position, i, v);
			}
			if (vertexLayout.has(bgfx::Attrib::TexCoord0))
			{
				auto v = config.textureScale * (vertex.texCoord + config.textureOffset);
				writer.write(bgfx::Attrib::TexCoord0, i, v);
			}
			if (vertexLayout.has(bgfx::Attrib::Normal))
			{
				writer.write(bgfx::Attrib::Normal, i, vertex.normal);
			}
			if (vertexLayout.has(bgfx::Attrib::Color0))
			{
				writer.write(bgfx::Attrib::Color0, i, vertex.color);
			}
			i++;
		}

		vertexData = writer.finish();
		if (config.index32)
		{
			updateMeshIndexData<int32_t>(indexData, indices, config.indexOffset);
		}
		else
		{
			updateMeshIndexData<VertexIndex>(indexData, indices, config.indexOffset);
		}
	}

	std::unique_ptr<IMesh> MeshData::createMesh(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& meshConfig) const
	{
		Data vertexData, indexData;
		exportData(vertexLayout, vertexData, indexData);
		return IMesh::create(config.type, vertexLayout, vertexData, indexData, meshConfig);
	}

	MeshData::MeshData(const Cube& Cube) noexcept
	{
		const static std::vector<Vertex> basicVertices = {
			{ { 1,  1,  1 }, {  0,  0,  1 }, { 0, 0 } },
			{ { 0,  1,  1 }, {  0,  0,  1 }, { 1, 0 } },
			{ { 0,  0,  1 }, {  0,  0,  1 }, { 1, 1 } },
			{ { 1,  0,  1 }, {  0,  0,  1 }, { 0, 1 } },
			{ { 1,  1,  0 }, {  0,  0, -1 }, { 1, 0 } },
			{ { 1,  0,  0 }, {  0,  0, -1 }, { 1, 1 } },
			{ { 0,  0,  0 }, {  0,  0, -1 }, { 0, 1 } },
			{ { 0,  1,  0 }, {  0,  0, -1 }, { 0, 0 } },
			{ { 1,  1,  1 }, {  0,  1,  0 }, { 1, 0 } },
			{ { 1,  1,  0 }, {  0,  1,  0 }, { 1, 1 } },
			{ { 0,  1,  0 }, {  0,  1,  0 }, { 0, 1 } },
			{ { 0,  1,  1 }, {  0,  1,  0 }, { 0, 0 } },
			{ { 1,  0,  1 }, {  0, -1,  0 }, { 1, 0 } },
			{ { 0,  0,  1 }, {  0, -1,  0 }, { 1, 1 } },
			{ { 0,  0,  0 }, {  0, -1,  0 }, { 0, 1 } },
			{ { 1,  0,  0 }, {  0, -1,  0 }, { 0, 0 } },
			{ { 1,  1,  1 }, {  1,  0,  0 }, { 1, 0 } },
			{ { 1,  0,  1 }, {  1,  0,  0 }, { 1, 1 } },
			{ { 1,  0,  0 }, {  1,  0,  0 }, { 0, 1 } },
			{ { 1,  1,  0 }, {  1,  0,  0 }, { 0, 0 } },
			{ { 0,  1,  1 }, { -1,  0,  0 }, { 0, 0 } },
			{ { 0,  1,  0 }, { -1,  0,  0 }, { 1, 0 } },
			{ { 0,  0,  0 }, { -1,  0,  0 }, { 1, 1 } },
			{ { 0,  0,  1 }, { -1,  0,  0 }, { 0, 1 } }
		};
		const static std::vector<Index> basicIndices
		{
			0,  1,  2,  2,  3,  0,
			4,  5,  6,  6,  7,  4,
			8,  9, 10, 10, 11,  8,
			12, 13, 14, 14, 15, 12,
			16, 17, 18, 18, 19, 16,
			20, 21, 22, 22, 23, 20,
		};

		vertices = basicVertices;
		indices = basicIndices;
		config.scale *= Cube.size;
		config.offset += (Cube.origin / Cube.size) - glm::vec3(0.5f);
	}

	MeshData::MeshData(const Rectangle& rect, RectangleMeshType type) noexcept
	{
		static const std::vector<MeshData::Vertex> basicVertices = {
			{ { 1, 1, 0 }, { 0, 0, -1 }, { 1, 0 } },
			{ { 1, 0, 0 }, { 0, 0, -1 }, { 1, 1 } },
			{ { 0, 0, 0 }, { 0, 0, -1 }, { 0, 1 } },
			{ { 0, 1, 0 }, { 0, 0, -1 }, { 0, 0 } }
		};
		vertices = basicVertices;
		if (type == RectangleMeshType::Outline)
		{
			indices = { 0, 1, 1, 2, 2, 3, 3, 0 };

		}
		else
		{
			indices = { 0, 1, 2, 2, 3, 0 };
		}

		config.scale *= glm::vec3(rect.size, 0);
		config.offset += glm::vec3(rect.origin / rect.size - glm::vec2(0.5F), 0);
	}

	MeshData::MeshData(const Sphere& sphere, int lod) noexcept
		: MeshData(Capsule(0, sphere.radius, sphere.origin), lod)
	{
	}

	 MeshData::MeshData(const Capsule& capsule, int lod) noexcept
	{
		auto rings = lod;
		auto sectors = lod;
		float texScale = 1.f;

		float R = 1.f / (float)(rings - 1);
		float S = 1.f / (float)(sectors - 1);
		auto n = (size_t)rings * sectors;
		auto pi = glm::pi<float>();

		{
			vertices.reserve(n);
			auto halfHeight = capsule.cylinderHeight / capsule.radius * 0.5F;
			for (int r = 0; r < rings; r++)
			{
				auto h = halfHeight;
				if (r > rings * 0.5)
				{
					h *= -1;
				}
				for (int s = 0; s < sectors; s++)
				{
					auto u = s * S;
					auto v = r * R;
					auto theta = u * 2.0f * pi;
					auto rho = v * pi;
					auto pos = glm::vec3(
						cos(theta) * sin(rho),
						sin((0.5F * pi) + rho) + h,
						sin(theta) * sin(rho)
					);
					vertices.emplace_back(pos, pos, glm::vec2(
						u * texScale,
						v * texScale
					));
				}
			}
		}

		{
			indices.reserve(n * 6);
			for (VertexIndex r = 0; r < rings - 1; r++)
			{
				for (VertexIndex s = 0; s < sectors - 1; s++)
				{
					indices.push_back(r * sectors + s);
					indices.push_back(r * sectors + (s + 1));
					indices.push_back((r + 1) * sectors + (s + 1));
					indices.push_back((r + 1) * sectors + (s + 1));
					indices.push_back((r + 1) * sectors + s);
					indices.push_back(r * sectors + s);
				}
			}
		}
		config.scale *= glm::vec3(capsule.radius);
		config.offset += capsule.origin / capsule.radius;
	}

	 MeshData::MeshData(const Ray& ray) noexcept
		 : MeshData(ray.toLine(), LineMeshType::Diamond)
	 {
	 }

	MeshData::MeshData(const Line& line, LineMeshType type) noexcept
	{
		if (type == LineMeshType::Line)
		{
			vertices.emplace_back(line.points[0], glm::vec3(), glm::vec2(0, 0));
			vertices.emplace_back(line.points[1], glm::vec3(), glm::vec2(1, 1));
			return;
		}
		static const float kInter = .2f;
		static const std::vector<glm::vec3> pos = {
			{ 1.f, 0.f, 0.f		}, { kInter, .1f, .1f },
			{ kInter, .1f, -.1f }, { kInter, -.1f, -.1f },
			{ kInter, -.1f, .1f }, { 0.f, 0.f, 0.f }
		};
		static const std::vector<glm::vec2> tex = {
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
		const std::vector<Vertex> basicVertices = {
			{
				{ pos[0], norm[0], tex[0] }, { pos[2], norm[0], tex[1] }, { pos[1], norm[0], tex[1] },
				{ pos[5], norm[1], tex[0] }, { pos[1], norm[1], tex[2] }, { pos[2], norm[1], tex[2] },
				{ pos[0], norm[2], tex[0] }, { pos[3], norm[2], tex[3] }, { pos[2], norm[2], tex[3] },
				{ pos[5], norm[3], tex[0] }, { pos[2], norm[3], tex[4] }, { pos[3], norm[3], tex[4] },
				{ pos[0], norm[4], tex[2] }, { pos[4], norm[4], tex[5] }, { pos[3], norm[4], tex[2] },
				{ pos[5], norm[5], tex[3] }, { pos[3], norm[5], tex[5] }, { pos[4], norm[5], tex[3] },
				{ pos[0], norm[6], tex[4] }, { pos[1], norm[6], tex[5] }, { pos[4], norm[6], tex[4] },
				{ pos[5], norm[7], tex[1] }, { pos[4], norm[7], tex[5] }, { pos[1], norm[7], tex[1] }
			}
		};
		vertices = basicVertices;
	}

	MeshData::MeshData(const Triangle& tri) noexcept
	{
		auto n = tri.getNormal();
		vertices = {
			{ 
				{ tri.vertices[0], n, { 0, 0 } },
				{ tri.vertices[1], n, { 1, 0 } },
				{ tri.vertices[2], n, { 1, 1 } }
			}
		};
	}

	MeshData MeshData::operator+(const MeshData& other) noexcept
	{
		MeshData sum = *this;
		sum += other;
		return sum;
	}

	MeshData& MeshData::operator+=(const MeshData& other) noexcept
	{
		auto offset = vertices.size();
		MeshData fother = other;
		fother.denormalize(config);
		for (auto& idx : fother.indices)
		{
			indices.push_back(offset + idx);
		}
		vertices.insert(vertices.begin(), fother.vertices.begin(), fother.vertices.end());
		return *this;
	}
}