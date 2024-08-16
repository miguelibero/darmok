#include <darmok/mesh.hpp>
#include <darmok/vertex.hpp>
#include <darmok/shape.hpp>
#include <darmok/data.hpp>
#include <array>
#include <mikktspace.h>

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
		if (!vertices.empty())
		{
			_vertexBuffer = bgfx::createVertexBuffer(vertices.copyMem(), layout, flags);
		}
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

	std::string Mesh::toString() const noexcept
	{
		return getMeshDescription("Mesh", _vertNum, _idxNum, getVertexLayout());
	}

	void MeshRenderConfig::fix(uint32_t maxVertices, uint32_t maxIndices) noexcept
	{
		auto max = maxVertices - startVertex;
		if (numVertices == 0 || numVertices > max)
		{
			numVertices = max;
		}
		max = maxIndices - startVertex;
		if (numIndices == 0 || numIndices > max)
		{
			numIndices = max;
		}
	}

	bool Mesh::render(bgfx::Encoder& encoder, RenderConfig config) const noexcept
	{
		if (!isValid(_vertexBuffer))
		{
			return false;
		}

		config.fix(_vertNum, _idxNum);
		if (config.numVertices == 0)
		{
			return false;
		}
		encoder.setVertexBuffer(config.vertexStream, _vertexBuffer, config.startVertex, config.numVertices);
		if (isValid(_indexBuffer))
		{
			encoder.setIndexBuffer(_indexBuffer, config.startIndex, config.numIndices);
		}
		return true;
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

	std::string DynamicMesh::toString() const noexcept
	{
		return getMeshDescription("DynamicMesh", _vertNum, _idxNum, getVertexLayout());
	}

	bool DynamicMesh::empty() const noexcept
	{
		return _vertNum == 0;
	}

	void DynamicMesh::updateVertices(DataView data, uint32_t offset) noexcept
	{
		if (data.empty())
		{
			return;
		}
		bgfx::update(_vertexBuffer, offset, data.copyMem());
	}

	void DynamicMesh::updateIndices(DataView data, uint32_t offset) noexcept
	{
		if (data.empty())
		{
			return;
		}
		bgfx::update(_indexBuffer, offset, data.copyMem());
	}

	bool DynamicMesh::render(bgfx::Encoder& encoder, RenderConfig config) const noexcept
	{
		if (!isValid(_vertexBuffer))
		{
			return false;
		}
		config.fix(_vertNum, _idxNum);
		if (config.numVertices == 0)
		{
			return false;
		}
		encoder.setVertexBuffer(config.vertexStream, _vertexBuffer, config.startVertex, config.numVertices);
		if (isValid(_indexBuffer))
		{
			encoder.setIndexBuffer(_indexBuffer, config.startIndex, config.numIndices);
		}
		return true;
	}

	TransientMesh::TransientMesh(const bgfx::VertexLayout& layout, DataView vertices, DataView indices, bool index32)
		: _layout(layout)
		, _vertNum(uint32_t(vertices.size() / layout.getStride()))
		, _idxNum(uint32_t(indices.size() / getMeshIndexSize(index32)))
	{
		if (!vertices.empty())
		{
			if (_vertNum > 0 && !bgfx::getAvailTransientVertexBuffer(_vertNum, layout))
			{
				throw std::runtime_error("not enought transient vertex buffer space");
			}
			bgfx::allocTransientVertexBuffer(&_vertexBuffer, _vertNum, layout);
			bx::memCopy(_vertexBuffer.data, vertices.ptr(), vertices.size());
		}
		else
		{
			_vertexBuffer.data = nullptr;
			_vertexBuffer.size = 0;
			_vertexBuffer.handle.idx = bgfx::kInvalidHandle;
		}
		if (!indices.empty())
		{
			if (!bgfx::getAvailTransientIndexBuffer(_idxNum, index32))
			{
				throw std::runtime_error("not enought transient index buffer space");
			}
			bgfx::allocTransientIndexBuffer(&_indexBuffer, _idxNum, index32);
			bx::memCopy(_indexBuffer.data, indices.ptr(), indices.size());
		}
		else
		{
			_indexBuffer.data = nullptr;
			_indexBuffer.size = 0;
			_indexBuffer.handle.idx = bgfx::kInvalidHandle;
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

	std::string TransientMesh::toString() const noexcept
	{
		return getMeshDescription("TransientMesh", _vertNum, _idxNum, getVertexLayout());
	}

	bool TransientMesh::render(bgfx::Encoder& encoder, RenderConfig config) const noexcept
	{
		config.fix(_vertNum, _idxNum);
		if(config.numVertices == 0)
		{
			return false;
		}
		encoder.setVertexBuffer(config.vertexStream, &_vertexBuffer, config.startVertex, config.numVertices);
		if (config.numIndices > 0)
		{
			encoder.setIndexBuffer(&_indexBuffer, config.startIndex, config.numIndices);
		}
		return true;
	}

	const bgfx::VertexLayout& TransientMesh::getVertexLayout() const noexcept
	{
		return _layout;
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
				writer.write(bgfx::Attrib::Position, i, vertex.position);
			}
			if (vertexLayout.has(bgfx::Attrib::TexCoord0))
			{
				writer.write(bgfx::Attrib::TexCoord0, i, vertex.texCoord);
			}
			if (vertexLayout.has(bgfx::Attrib::Normal))
			{
				writer.write(bgfx::Attrib::Normal, i, vertex.normal);
			}
			if (vertexLayout.has(bgfx::Attrib::Tangent))
			{
				writer.write(bgfx::Attrib::Tangent, i, vertex.tangent);
			}
			if (vertexLayout.has(bgfx::Attrib::Color0))
			{
				writer.write(bgfx::Attrib::Color0, i, vertex.color);
			}
			++i;
		}

		vertexData = writer.finish();
		indexData = indices;
	}

	std::unique_ptr<IMesh> MeshData::createMesh(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& meshConfig) const
	{
		Data vertexData, indexData;
		exportData(vertexLayout, vertexData, indexData);
		return IMesh::create(type, vertexLayout, vertexData, indexData, meshConfig);
	}

	MeshData::MeshData(const Cube& Cube) noexcept
	{
		const static std::vector<Vertex> basicVertices = {
			{ { 1,  1,  1 }, { 0, 0 }, {  0,  0,  1 } },
			{ { 0,  1,  1 }, { 1, 0 }, {  0,  0,  1 } },
			{ { 0,  0,  1 }, { 1, 1 }, {  0,  0,  1 } },
			{ { 1,  0,  1 }, { 0, 1 }, {  0,  0,  1 } },
			{ { 1,  1,  0 }, { 0, 0 }, {  0,  0, -1 } },
			{ { 1,  0,  0 }, { 1, 0 }, {  0,  0, -1 } },
			{ { 0,  0,  0 }, { 1, 1 }, {  0,  0, -1 } },
			{ { 0,  1,  0 }, { 0, 1 }, {  0,  0, -1 } },
			{ { 1,  1,  1 }, { 0, 0 }, {  0,  1,  0 } },
			{ { 1,  1,  0 }, { 1, 0 }, {  0,  1,  0 } },
			{ { 0,  1,  0 }, { 1, 1 }, {  0,  1,  0 } },
			{ { 0,  1,  1 }, { 0, 1 }, {  0,  1,  0 } },
			{ { 1,  0,  1 }, { 0, 0 }, {  0, -1,  0 } },
			{ { 0,  0,  1 }, { 1, 0 }, {  0, -1,  0 } },
			{ { 0,  0,  0 }, { 1, 1 }, {  0, -1,  0 } },
			{ { 1,  0,  0 }, { 0, 1 }, {  0, -1,  0 } },
			{ { 1,  1,  1 }, { 0, 0 }, {  1,  0,  0 } },
			{ { 1,  0,  1 }, { 1, 0 }, {  1,  0,  0 } },
			{ { 1,  0,  0 }, { 1, 1 }, {  1,  0,  0 } },
			{ { 1,  1,  0 }, { 0, 1 }, {  1,  0,  0 } },
			{ { 0,  1,  1 }, { 0, 0 }, { -1,  0,  0 } },
			{ { 0,  1,  0 }, { 1, 0 }, { -1,  0,  0 } },
			{ { 0,  0,  0 }, { 1, 1 }, { -1,  0,  0 } },
			{ { 0,  0,  1 }, { 0, 1 }, { -1,  0,  0 } }
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

		auto trans = glm::scale(glm::mat4(1), Cube.size);
		trans = glm::translate(trans, (Cube.origin / Cube.size) - glm::vec3(0.5f));
		*this *= trans;

		calcTangents();
	}

	MeshData::MeshData(const Rectangle& rect, RectangleMeshType type) noexcept
	{
		static const std::vector<MeshData::Vertex> basicVertices = {
			{ { 1, 1, 0 }, { 1, 0 }, { 0, 0, -1 } },
			{ { 1, 0, 0 }, { 1, 1 }, { 0, 0, -1 } },
			{ { 0, 0, 0 }, { 0, 1 }, { 0, 0, -1 } },
			{ { 0, 1, 0 }, { 0, 0 }, { 0, 0, -1 } }
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

		auto trans = glm::scale(glm::mat4(1), glm::vec3(rect.size, 0));
		trans = glm::translate(trans, glm::vec3(rect.origin / rect.size - glm::vec2(0.5F), 0));
		*this *= trans;

		calcTangents();
	}

	MeshData::MeshData(const Sphere& sphere, unsigned int lod) noexcept
		: MeshData(Capsule(0, sphere.radius, sphere.origin), lod)
	{
	}

	MeshData::MeshData(const Capsule& capsule, unsigned int lod) noexcept
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
					vertices.emplace_back(pos, glm::vec2(
						u * texScale,
						v * texScale
					), pos);
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
		auto trans = glm::translate(glm::mat4(capsule.radius), capsule.origin / capsule.radius);
		*this *= trans;

		calcTangents();
    }

	 MeshData::MeshData(const Ray& ray) noexcept
		 : MeshData(ray.toLine(), LineMeshType::Diamond)
	 {
	 }

	MeshData::MeshData(const Line& line, LineMeshType type) noexcept
	{
		if (type == LineMeshType::Line)
		{
			vertices.emplace_back(line.points[0], glm::vec2(0, 0));
			vertices.emplace_back(line.points[1], glm::vec2(1, 1));
			calcTangents();
			return;
		}
		static const float kInter = .2f;
		static const std::vector<glm::vec3> pos = {
			{ 1.f, 0.f, 0.f		}, { kInter, .1f, .1f },
			{ kInter, .1f, -.1f }, { kInter, -.1f, -.1f },
			{ kInter, -.1f, .1f }, { 0.f, 0.f, 0.f }
		};
		static const std::vector<glm::vec2> tex = {
			{ 0.f, 0.f	}, { .1f, .1f },
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
				{ pos[0], tex[0], norm[0] }, { pos[2], tex[1], norm[0] }, { pos[1], tex[1], norm[0] },
				{ pos[5], tex[0], norm[1] }, { pos[1], tex[2], norm[1] }, { pos[2], tex[2], norm[1] },
				{ pos[0], tex[0], norm[2] }, { pos[3], tex[3], norm[2] }, { pos[2], tex[3], norm[2] },
				{ pos[5], tex[0], norm[3] }, { pos[2], tex[4], norm[3] }, { pos[3], tex[4], norm[3] },
				{ pos[0], tex[2], norm[4] }, { pos[4], tex[5], norm[4] }, { pos[3], tex[2], norm[4] },
				{ pos[5], tex[3], norm[5] }, { pos[3], tex[5], norm[5] }, { pos[4], tex[3], norm[5] },
				{ pos[0], tex[4], norm[6] }, { pos[1], tex[5], norm[6] }, { pos[4], tex[4], norm[6] },
				{ pos[5], tex[1], norm[7] }, { pos[4], tex[5], norm[7] }, { pos[1], tex[1], norm[7] }
			}
		};
		vertices = basicVertices;

		calcTangents();
	}

	MeshData::MeshData(const Triangle& tri) noexcept
	{
		auto n = tri.getNormal();
		vertices = {
			{ 
				{ tri.vertices[0], { 0, 0 }, n },
				{ tri.vertices[1], { 1, 0 }, n },
				{ tri.vertices[2], { 1, 1 }, n }
			}
		};
		calcTangents();
	}

	MeshData::MeshData(const Polygon& poly) noexcept
	{
		for (auto& tri : poly.triangles)
		{
			*this += MeshData(tri);
		}
		*this *= glm::translate(glm::mat4(1), poly.origin);
	}

	MeshData& MeshData::operator+=(const MeshData& other) noexcept
	{
		auto offset = vertices.size();
		MeshData fother = other;
		indices.reserve(indices.size() + other.indices.size());
		for (auto& idx : fother.indices)
		{
			indices.push_back(offset + idx);
		}
		vertices.reserve(vertices.size() + other.vertices.size());
		vertices.insert(vertices.end(), fother.vertices.begin(), fother.vertices.end());
		return *this;
	}

	MeshData& MeshData::operator*=(const glm::mat4& transform) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position = transform * glm::vec4(vertex.position, 1.F);
			vertex.normal = transform * glm::vec4(vertex.normal, 0.F);
			vertex.tangent = transform * vertex.tangent;
		}
		return *this;
	}

	MeshData& MeshData::operator*=(const glm::mat2& textureTransform) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.texCoord = textureTransform * vertex.texCoord;
		}
		return *this;
	}

	MeshData& MeshData::operator*=(const Color& color) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.color *= Colors::normalize(color);
		}
		return *this;
	}

	MeshData& MeshData::operator*=(const glm::uvec2& textureScale) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.texCoord *= textureScale;
		}
		return *this;
	}

	MeshData& MeshData::operator+=(const glm::uvec2& textureOffset) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.texCoord += textureOffset;
		}
		return *this;
	}

	MeshData& MeshData::shiftIndices(Index offset) noexcept
	{
		for (auto& idx : indices)
		{
			idx += offset;
		}
		return *this;
	}


	struct CalcTangentsOperation final
	{
	public:
		CalcTangentsOperation() noexcept
		{
			iface.m_getNumFaces = getNumFaces;
			iface.m_getNumVerticesOfFace = getNumFaceVertices;
			iface.m_getNormal = getNormal;
			iface.m_getPosition = getPosition;
			iface.m_getTexCoord = getTexCoords;
			iface.m_setTSpaceBasic = setTangent;

			context.m_pInterface = &iface;
		}

		void operator()(MeshData& mesh) noexcept
		{
			context.m_pUserData = &mesh;
			genTangSpaceDefault(&this->context);
		}

	private:
		SMikkTSpaceInterface iface{};
		SMikkTSpaceContext context{};

		static MeshData& getMeshDataFromContext(const SMikkTSpaceContext* context) noexcept
		{
			return *static_cast<MeshData*>(context->m_pUserData);
		}

		static int getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert) noexcept
		{
			MeshData& mesh = getMeshDataFromContext(context);
			auto faceSize = getNumFaceVertices(context, iFace);
			auto index = (iFace * faceSize) + iVert;
			if (mesh.indices.empty())
			{
				return index;
			}
			return mesh.indices[index];
		}

		static int getNumFaces(const SMikkTSpaceContext* context) noexcept
		{
			MeshData& mesh = getMeshDataFromContext(context);
			if (mesh.indices.empty())
			{
				return mesh.vertices.size() / 3;
			}
			return mesh.indices.size() / 3;
		}

		static int getNumFaceVertices(const SMikkTSpaceContext* context, int iFace) noexcept
		{
			return 3;
		}

		static void getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert) noexcept
		{
			MeshData& mesh = getMeshDataFromContext(context);
			auto index = getVertexIndex(context, iFace, iVert);
			auto& vert = mesh.vertices[index];
			outpos[0] = vert.position.x;
			outpos[1] = vert.position.y;
			outpos[2] = vert.position.z;
		}

		static void getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert) noexcept
		{
			MeshData& mesh = getMeshDataFromContext(context);
			auto index = getVertexIndex(context, iFace, iVert);
			auto& vert = mesh.vertices[index];
			outnormal[0] = vert.normal.x;
			outnormal[1] = vert.normal.y;
			outnormal[2] = vert.normal.z;
		}

		static void getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert) noexcept
		{
			MeshData& mesh = getMeshDataFromContext(context);
			auto index = getVertexIndex(context, iFace, iVert);
			auto& vert = mesh.vertices[index];
			outuv[0] = vert.texCoord.x;
			outuv[1] = vert.texCoord.y;
		}

		static void setTangent(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert) noexcept
		{
			MeshData& mesh = getMeshDataFromContext(context);
			auto index = getVertexIndex(context, iFace, iVert);
			auto& vert = mesh.vertices[index];
			vert.tangent.x = tangentu[0];
			vert.tangent.y = tangentu[1];
			vert.tangent.z = tangentu[2];
			vert.tangent.w = fSign;
		}
	};

	MeshData& MeshData::calcTangents() noexcept
	{
		CalcTangentsOperation op;
		op(*this);
		return *this;
	}
}