#include <darmok/mesh.hpp>
#include <darmok/vertex.hpp>
#include <darmok/shape.hpp>
#include <darmok/data.hpp>
#include <glm/gtx/component_wise.hpp>

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

	std::unique_ptr<IMesh> IMesh::create(const MeshDefinition& def)
	{
		return create(def.type, def.layout, def.vertices, def.indices, def.config);
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

	MeshLoader::MeshLoader(IMeshDefinitionLoader& defLoader) noexcept
		: BasicFromDefinitionLoader(defLoader)
	{
	}

	std::shared_ptr<IMesh> MeshLoader::create(const std::shared_ptr<Definition>& def)
	{
		return IMesh::create(*def);
	}

	MeshData::MeshData(MeshType type) noexcept
		: type(type)
	{
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
				.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float)
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
			writer.write(bgfx::Attrib::Position, i, vertex.position);
			writer.write(bgfx::Attrib::TexCoord0, i, vertex.texCoord);
			writer.write(bgfx::Attrib::Normal, i, vertex.normal);
			writer.write(bgfx::Attrib::Tangent, i, vertex.tangent);
			writer.write(bgfx::Attrib::Color0, i, vertex.color);
			++i;
		}

		vertexData = writer.finish();
		
		// TODO: support int32 indices
		indexData = DataView(indices);
	}

	MeshDefinition MeshData::createMeshDefinition(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& meshConfig) const
	{
		MeshDefinition def
		{
			.type = type,
			.config = meshConfig,
			.layout = vertexLayout,
		};
		exportData(vertexLayout, def.vertices, def.indices);
		return def;
	}

	std::unique_ptr<IMesh> MeshData::createMesh(const bgfx::VertexLayout& vertexLayout, const IMesh::Config& meshConfig) const
	{
		auto def = createMeshDefinition(vertexLayout, meshConfig);
		return IMesh::create(def);
	}

	const std::vector<MeshData::Index> MeshData::_cuboidTriangleIndices
	{
		0,  1,  2,  2,  3,  0,
		4,  5,  6,  6,  7,  4,
		8,  9, 10, 10, 11,  8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	MeshData::MeshData(const Cube& cube, RectangleMeshType type) noexcept
	{
		const static std::vector<Vertex> basicVertices = {
			{ { 1,  1,  1 }, { 0, 0 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 0,  1,  1 }, { 1, 0 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 0,  0,  1 }, { 1, 1 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 1,  0,  1 }, { 0, 1 }, {  0,  0,  1 }, {  1,  0,  0 } },
			{ { 1,  1,  0 }, { 0, 0 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 1,  0,  0 }, { 1, 0 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 0,  0,  0 }, { 1, 1 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 0,  1,  0 }, { 0, 1 }, {  0,  0, -1 }, { -1,  0,  0 } },
			{ { 1,  1,  1 }, { 0, 0 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 1,  1,  0 }, { 1, 0 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 0,  1,  0 }, { 1, 1 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 0,  1,  1 }, { 0, 1 }, {  0,  1,  0 }, {  0,  0,  1 } },
			{ { 1,  0,  1 }, { 0, 0 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 0,  0,  1 }, { 1, 0 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 0,  0,  0 }, { 1, 1 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 1,  0,  0 }, { 0, 1 }, {  0, -1,  0 }, {  0,  0, -1 } },
			{ { 1,  1,  1 }, { 0, 0 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 1,  0,  1 }, { 1, 0 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 1,  0,  0 }, { 1, 1 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 1,  1,  0 }, { 0, 1 }, {  1,  0,  0 }, {  0,  1,  0 } },
			{ { 0,  1,  1 }, { 0, 0 }, { -1,  0,  0 }, {  0, -1,  0 } },
			{ { 0,  1,  0 }, { 1, 0 }, { -1,  0,  0 }, {  0, -1,  0 } },
			{ { 0,  0,  0 }, { 1, 1 }, { -1,  0,  0 }, {  0, -1,  0 } },
			{ { 0,  0,  1 }, { 0, 1 }, { -1,  0,  0 }, {  0, -1,  0 } }
		};

		vertices = basicVertices;
		indices = _cuboidTriangleIndices;

		if (type == RectangleMeshType::Outline)
		{
			convertQuadIndicesToLine();
		}

		if (cube.size.x == 0.F || cube.size.y == 0.F || cube.size.z == 0.F)
		{
			for (auto& vertex : vertices)
			{
				vertex.position = cube.origin;
			}
			return;
		}

		auto trans = glm::scale(glm::mat4(1), cube.size);
		trans = glm::translate(trans, (cube.origin / cube.size) - glm::vec3(0.5f));
		*this *= trans;
	}

	void MeshData::setupBasicRectangle() noexcept
	{
		static const std::vector<MeshData::Vertex> basicVertices = {
			{ {  0.5F,  0.5F, 0 }, { 1, 0 }, { 0, 0, 1 }, { 1, 0, 0} },
			{ {  0.5F, -0.5F, 0 }, { 1, 1 }, { 0, 0, 1 }, { 1, 0, 0} },
			{ { -0.5F, -0.5F, 0 }, { 0, 1 }, { 0, 0, 1 }, { 1, 0, 0} },
			{ { -0.5F,  0.5F, 0 }, { 0, 0 }, { 0, 0, 1 }, { 1, 0, 0} }
		};
		static const std::vector<Index> basicIndices = { 0, 1, 2, 2, 3, 0 };
		vertices = basicVertices;
		indices = basicIndices;
	}

	MeshData::MeshData(const Rectangle& rect, RectangleMeshType type) noexcept
	{
		setupBasicRectangle();

		if (type == RectangleMeshType::Outline)
		{
			convertQuadIndicesToLine();
		}

		if (rect.size.x == 0.F || rect.size.y == 0.F)
		{
			for (auto& vertex : vertices)
			{
				vertex.position = glm::vec3(rect.origin, 0);
			}
			return;
		}

		auto trans = glm::scale(glm::mat4(1), glm::vec3(rect.size, 1.F));
		trans = glm::translate(trans, glm::vec3(rect.origin / rect.size, 0));
		*this *= trans;
	}

	MeshData::MeshData(const Plane& plane, RectangleMeshType type, float scale) noexcept
	{
		setupBasicRectangle();

		if (type == RectangleMeshType::Outline)
		{
			convertQuadIndicesToLine();
		}

		scalePositions(glm::vec3(scale));

		*this *= plane.getTransform();
	}

	MeshData::MeshData(const Frustum& frust, RectangleMeshType type) noexcept
	{
		vertices = {
			{ frust.getCorner(Frustum::CornerType::FarTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarTopLeft),			{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomRight),		{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomRight),	{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearTopLeft),		{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearTopRight),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearTopLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopLeft),			{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomLeft),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomRight),	{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopRight),		{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomRight),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomRight),	{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::NearTopRight),		{ 0, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarTopLeft),			{ 0, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearTopLeft),		{ 1, 0 } },
			{ frust.getCorner(Frustum::CornerType::NearBottomLeft),		{ 1, 1 } },
			{ frust.getCorner(Frustum::CornerType::FarBottomLeft),		{ 0, 1 } }
		};

		indices = _cuboidTriangleIndices;

		if (type == RectangleMeshType::Outline)
		{
			convertQuadIndicesToLine();
		}
		else
		{
			calcNormals();
			calcTangents();
		}
	}

	MeshData& MeshData::convertQuadIndicesToLine() noexcept
	{
		std::vector<Index> v(indices);
		indices.clear();
		indices.reserve(v.size() * 8 / 6);
		for (auto i = 0; i < v.size() - 5; i += 6)
		{
			indices.push_back(v[i]);
			indices.push_back(v[i + 1]);
			indices.push_back(v[i + 1]);
			indices.push_back(v[i + 2]);
			indices.push_back(v[i + 2]);
			indices.push_back(v[i + 4]);
			indices.push_back(v[i + 4]);
			indices.push_back(v[i]);
		}

		return *this;
	}

	MeshData& MeshData::subdivide(size_t amount) noexcept
	{
		vertices.reserve(vertices.size() * (3 + amount) / 3);
		indices.reserve(indices.size() * (1 + amount));
		for (size_t i = 0; i < amount; ++i)
		{
			auto size = indices.size() / 3;
			for (auto j = 0; j < size; ++j)
			{
				doSubdivide(j);
			}
		}
		return *this;
	}

	size_t MeshData::subdivideDensity(float maxDistance) noexcept
	{
		float maxv = -bx::kFloatInfinity;
		bool found = true;
		size_t count = 0;
		while(found)
		{
			found = false;
			for (auto i = 0; i < indices.size() / 3; ++i)
			{
				if (doSubdivide(i, maxDistance))
				{
					found = true;
					++count;
					break;
				}
			}
		}
		return count;
	}

	bool MeshData::doSubdivide(size_t i, float maxDistance) noexcept
	{
		i *= 3;
		const auto i1 = indices[i];
		const auto i2 = indices[i + 1];
		const auto i3 = indices[i + 2];
		const auto i4 = vertices.size();
		const auto v1 = vertices[i1];
		const auto v2 = vertices[i2];
		const auto v3 = vertices[i3];

		auto d = glm::vec3(
			glm::distance(v2.position, v1.position),
			glm::distance(v3.position, v2.position),
			glm::distance(v1.position, v3.position)
		);
		auto maxd = glm::compMax(d);
		if (maxd < maxDistance)
		{
			return false;
		}

		auto& v4 = vertices.emplace_back();
		if (maxd == d.x)
		{
			v4 = mix(v1, v2, 0.5F);
			indices[i + 1] = i4;
			indices.push_back(i4);
			indices.push_back(i2);
			indices.push_back(i3);
		}
		else if (maxd == d.y)
		{
			v4 = mix(v2, v3, 0.5F);
			indices[i + 2] = i4;
			indices.push_back(i1);
			indices.push_back(i4);
			indices.push_back(i3);
		}
		else if (maxd == d.z)
		{
			v4 = mix(v3, v1, 0.5F);
			indices[i + 2] = i4;
			indices.push_back(i4);
			indices.push_back(i2);
			indices.push_back(i3);
		}

		return true;
	}

	MeshData::Vertex MeshData::mix(const Vertex& v1, const Vertex& v2, float f) noexcept
	{
		return {
			.position = glm::mix(v1.position, v2.position, f),
			.texCoord = glm::mix(v1.texCoord, v2.texCoord, f),
			.normal = glm::normalize(glm::mix(v1.normal, v2.normal, f)),
			.tangent = glm::normalize(glm::mix(v1.tangent, v2.tangent, f)),
			.color = glm::mix(v1.color, v2.color, f),
		};
	}

	MeshData::MeshData(const Sphere& sphere, unsigned int lod) noexcept
		: MeshData(Capsule(0, sphere.radius, sphere.origin), lod)
	{
	}

	MeshData::MeshData(const Capsule& capsule, unsigned int lod) noexcept
	{
		unsigned int radialSegments = lod;
		unsigned int capSegments = lod;
		auto halfLength = capsule.cylinderHeight / capsule.radius * 0.5F;

		auto calcCapVertices = [this, capSegments, radialSegments, halfLength](bool topCap)
		{
			int baseIndex = static_cast<int>(vertices.size());
			float f = topCap ? 1.F : -1.F;
			for (int i = 0; i <= capSegments; ++i)
			{
				float v = float(i) / capSegments;
				float theta = v * glm::half_pi<float>();
				float sinTheta = glm::sin(theta);
				float cosTheta = glm::cos(theta);

				for (int j = 0; j <= radialSegments; ++j)
				{
					float u = float(j) / radialSegments;
					if (!topCap)
					{
						u = 1.F - u;
					}
					float phi = u * glm::two_pi<float>();
					float sinPhi = glm::sin(phi);
					float cosPhi = glm::cos(phi);

					auto normPos = glm::vec3(
						cosPhi * cosTheta,
						f * sinTheta,
						sinPhi * cosTheta);

					auto tangent = f * glm::vec3(-sinPhi, 0, cosPhi);
					vertices.emplace_back(
						normPos + glm::vec3(0, f * halfLength, 0),
						glm::vec2(u, topCap ? v : 1.0f - v),
						normPos,
						tangent
					);

					if (j < radialSegments && i < capSegments)
					{
						int current = baseIndex + i * (radialSegments + 1) + j;
						int next = current + radialSegments + 1;

						indices.push_back(current);
						indices.push_back(next);
						indices.push_back(current + 1);

						indices.push_back(current + 1);
						indices.push_back(next);
						indices.push_back(next + 1);
					}
				}
			}
		};
		
		calcCapVertices(true);
		calcCapVertices(false);

		// body
		int baseIndex = static_cast<int>(vertices.size());
		for (int i = 0; i < 2; ++i)
		{
			float v = float(i);
			float y = ((v * 2.F) - 1.F) * halfLength;

			for (int j = 0; j <= radialSegments; ++j)
			{
				float u = float(j) / radialSegments;
				float phi = u * glm::two_pi<float>();
				float sinPhi = glm::sin(phi);
				float cosPhi = glm::cos(phi);

				vertices.emplace_back(
					glm::vec3(cosPhi, y, sinPhi),
					glm::vec2(u, (1.F + v) * 0.5F),
					glm::vec3(cosPhi, 0, sinPhi),
					glm::vec3(-sinPhi, 0, cosPhi)
				);

				if (j < radialSegments && i == 0)
				{
					int current = baseIndex + j;
					int next = current + radialSegments + 1;

					indices.push_back(current);
					indices.push_back(next);
					indices.push_back(current + 1);

					indices.push_back(current + 1);
					indices.push_back(next);
					indices.push_back(next + 1);
				}
			}
		}

		auto trans = glm::translate(glm::mat4(capsule.radius), capsule.origin / capsule.radius);
		*this *= trans;
    }

	 MeshData::MeshData(const Ray& ray) noexcept
		 : MeshData(ray.toLine(), LineMeshType::Arrow)
	 {
	 }

	MeshData::MeshData(const Line& line, LineMeshType type) noexcept
	{
		if (type == LineMeshType::Line)
		{
			vertices.emplace_back(line.points[0], glm::vec2{ 0, 0 });
			vertices.emplace_back(line.points[1], glm::vec2{ 1, 1 });
			return;
		}
		static const float kInter = .2f;
		
		static const std::vector<glm::vec3> pos = {
			{  0.f,  0.f, 1.f    }, {  .1f,  .1f, kInter },
			{ -.1f,  .1f, kInter }, { -.1f, -.1f, kInter },
			{  .1f, -.1f, kInter }, {  0.f,  0.f, 0.f }
		};

		static const std::vector<glm::vec2> tex = {
			{ 0.f, 0.f	}, { .1f, .1f },
			{ .1f, -.1f }, { -.1f, -.1f },
			{ -.1f, .1f }, { 0.f, 0.f }
		};
		vertices = {
			{
				{ pos[0], tex[0] }, { pos[2], tex[1] }, { pos[1], tex[1] },
				{ pos[5], tex[0] }, { pos[1], tex[2] }, { pos[2], tex[2] },
				{ pos[0], tex[0] }, { pos[3], tex[3] }, { pos[2], tex[3] },
				{ pos[5], tex[0] }, { pos[2], tex[4] }, { pos[3], tex[4] },
				{ pos[0], tex[2] }, { pos[4], tex[5] }, { pos[3], tex[2] },
				{ pos[5], tex[3] }, { pos[3], tex[5] }, { pos[4], tex[3] },
				{ pos[0], tex[4] }, { pos[1], tex[5] }, { pos[4], tex[4] },
				{ pos[5], tex[1] }, { pos[4], tex[5] }, { pos[1], tex[1] }
			}
		};

		auto trans = line.getTransform();

		for (auto& vertex : vertices)
		{
			vertex.position = trans * glm::vec4(vertex.position, 1.0);
		}

		calcNormals();
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

	MeshData::MeshData(const Grid& grid) noexcept
	{
		auto dx = grid.getAlong();		
		auto dy = glm::cross(dx, grid.normal);
		dx *= grid.separation.x;
		dy *= grid.separation.y;
		auto amount = glm::vec2(grid.amount) * 0.5F;
		vertices.reserve(2 * (grid.amount.x + grid.amount.y));

		auto addVertex = [&](float x, float y)
		{
			auto p = grid.origin + (dx * x) + (dy * y);
			vertices.emplace_back(p, glm::vec2(x, y), grid.normal);
		};

		for (float x = -amount.x; x <= amount.x; ++x)
		{
			auto i = vertices.size();
			addVertex(x, -amount.y);
			addVertex(x, +amount.y);
			indices.push_back(i);
			indices.push_back(i + 1);
		}
		for (float y = -amount.y; y <= amount.y; ++y)
		{
			auto i = vertices.size();
			addVertex(-amount.x, y);
			addVertex(amount.x, y);
			indices.push_back(i);
			indices.push_back(i + 1);
		}
	}

	MeshData MeshData::operator+(const MeshData& other) const noexcept
	{
		MeshData r(*this);
		r += other;
		return r;
	}

	void MeshData::doCreateIndices(std::vector<Index>& indices, size_t size) noexcept
	{
		indices.reserve(size);
		for (size_t i = 0; i < size; ++i)
		{
			indices.push_back(i);
		}
	}

	MeshData& MeshData::createIndices() noexcept
	{
		doCreateIndices(indices, vertices.size());
		return *this;
	}

	MeshData& MeshData::operator+=(const MeshData& other) noexcept
	{
		auto offset = vertices.size();
		auto otherIndices = other.indices;
		if (indices.empty() && !otherIndices.empty())
		{
			createIndices();
		}
		if (!indices.empty() && otherIndices.empty())
		{
			doCreateIndices(otherIndices, other.vertices.size());
		}
		indices.reserve(indices.size() + otherIndices.size());
		for (auto& idx : otherIndices)
		{
			indices.push_back(offset + idx);
		}
		vertices.reserve(vertices.size() + other.vertices.size());
		vertices.insert(vertices.end(), other.vertices.begin(), other.vertices.end());
		return *this;
	}

	MeshData& MeshData::operator*=(const glm::mat4& trans) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position = trans * glm::vec4(vertex.position, 1.F);
			vertex.normal = trans * glm::vec4(vertex.normal, 0.F);
			vertex.normal = glm::normalize(vertex.normal);
			vertex.tangent = trans * glm::vec4(vertex.tangent, 0.F);
			vertex.tangent = glm::normalize(vertex.tangent);
		}
		return *this;
	}

	MeshData& MeshData::operator*=(const Color& color) noexcept
	{
		auto ncolor = Colors::normalize(color);
		for (auto& vertex : vertices)
		{
			vertex.color = glm::vec4(vertex.color) * ncolor;
		}
		return *this;
	}

	MeshData& MeshData::scalePositions(const glm::vec3& scale) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position *= scale;
		}
		return *this;
	}

	MeshData& MeshData::translatePositions(const glm::vec3& pos) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.position += pos;
		}
		return *this;
	}

	MeshData& MeshData::scaleTexCoords(const glm::vec2& scale) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.texCoord *= scale;
		}
		return *this;
	}
	MeshData& MeshData::translateTexCoords(const glm::vec2& pos) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.texCoord += pos;
		}
		return *this;
	}

	MeshData& MeshData::setColor(const Color& color) noexcept
	{
		for (auto& vertex : vertices)
		{
			vertex.color = color;
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


	struct MeshDataCalcTangentsOperation final
	{
	public:
		MeshDataCalcTangentsOperation() noexcept
		{
			_iface.m_getNumFaces = getNumFaces;
			_iface.m_getNumVerticesOfFace = getNumFaceVertices;
			_iface.m_getNormal = getNormal;
			_iface.m_getPosition = getPosition;
			_iface.m_getTexCoord = getTexCoords;
			_iface.m_setTSpaceBasic = setTangent;

			_context.m_pInterface = &_iface;
		}

		void operator()(MeshData& mesh) noexcept
		{
			_context.m_pUserData = &mesh;
			genTangSpaceDefault(&_context);
		}

	private:
		SMikkTSpaceInterface _iface{};
		SMikkTSpaceContext _context{};

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
		}
	};

	MeshData& MeshData::calcTangents() noexcept
	{
		MeshDataCalcTangentsOperation op;
		op(*this);
		return *this;
	}

	MeshData& MeshData::calcNormals() noexcept
	{
		for (auto& face : getFaces())
		{
			auto& vert1 = vertices[face[0]];
			auto& vert2 = vertices[face[1]];
			auto& vert3 = vertices[face[2]];
			auto edge1 = vert2.position - vert1.position;
			auto edge2 = vert3.position - vert1.position;

			auto normal = glm::cross(edge1, edge2);
			normal = glm::normalize(normal);

			vert1.normal = normal;
			vert2.normal = normal;
			vert3.normal = normal;
		}
		return *this;
	}

	std::vector<MeshData::Face> MeshData::getFaces() const noexcept
	{
		std::vector<Face> faces;
		if (indices.empty())
		{
			faces.reserve(vertices.size() / 3);
			for (Index i = 0; i < vertices.size(); i += 3)
			{
				Face face = { (Index)i, (Index)(i + 1), (Index)(i + 2) };
				faces.push_back(std::move(face));
			}
		}
		else
		{
			faces.reserve(indices.size() / 3);
			for (size_t i = 0; i < indices.size() - 2; i += 3)
			{
				Face face = { indices[i], indices[i + 1], indices[i + 2] };
				faces.push_back(std::move(face));
			}
		}

		return faces;
	}

	BoundingBox MeshData::getBounds() const noexcept
	{
		BoundingBox bb;

		if (indices.empty())
		{
			if (!vertices.empty())
			{
				bb.min = vertices[0].position;
				bb.max = bb.min;
			}
			for (auto& vert : vertices)
			{
				bb.expandToPosition(vert.position);
			}
		}
		else
		{
			bb.min = vertices[indices[0]].position;
			bb.max = bb.min;

			for (auto& idx : indices)
			{
				auto& vert = vertices[idx];
				bb.expandToPosition(vert.position);
			}
		}

		return bb;
	}
}