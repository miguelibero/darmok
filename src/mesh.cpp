#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include <darmok/vertex.hpp>
#include <darmok/shape.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>

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
		switch (cfg.type)
		{
		case MeshType::Dynamic:
			return std::make_shared<DynamicMesh>(layout, vertDataView, idxDataView);
		case MeshType::Transient:
			return std::make_shared<TransientMesh>(layout, vertDataView, idxDataView);
		default:
			return std::make_shared<Mesh>(layout, vertDataView, idxDataView);
		}
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

	const static MeshData _quadMeshData = {
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

	std::shared_ptr<IMesh> MeshCreator::createQuadMesh(const bgfx::VertexLayout& layout, const MeshData& data, const Quad& quad) noexcept
	{
		auto cfg = config;
		cfg.scale *= glm::vec3(quad.size, 0);
		cfg.offset += glm::vec3(quad.origin - glm::vec2(0.5F), 0);
		return createMesh(data, cfg);
	}

	std::shared_ptr<IMesh> MeshCreator::createQuad(const Quad& quad) noexcept
	{
		MeshData data = _quadMeshData;
		data.indices = { 0, 1, 2, 2, 3, 0 };
		return createQuadMesh(layout, data, quad);
	}

	std::shared_ptr<IMesh> MeshCreator::createLineQuad(const Quad& quad) noexcept
	{
		MeshData data = _quadMeshData;
		data.indices = { 0, 1, 1, 2, 2, 3, 3, 0 };
		return createQuadMesh(layout, data, quad);
	}

	std::shared_ptr<IMesh> MeshCreator::createSphere(int lod) noexcept
	{
		return createSphere(Sphere::standard());
	}

	std::shared_ptr<IMesh> MeshCreator::createQuad() noexcept
	{
		return createQuad(Quad::standard());
	}

	std::shared_ptr<IMesh> MeshCreator::createLineQuad() noexcept
	{
		return createLineQuad(Quad::standard());
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
			data.texCoords.emplace_back(0, 1);
			data.indices.emplace_back(i++);
			data.indices.emplace_back(i++);
		}
		return createMesh(data, config);
	}

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept
		: _mesh(mesh)
		, _material(material)
	{
	}

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Texture>& texture) noexcept
		: _mesh(mesh)
		, _material(std::make_shared<Material>(texture))
	{
	}

	Renderable::Renderable(const std::shared_ptr<Material>& material) noexcept
		: _material(material)
	{
	}

	std::shared_ptr<IMesh> Renderable::getMesh() const noexcept
	{
		return _mesh;
	}

	Renderable& Renderable::setMesh(const std::shared_ptr<IMesh>& mesh) noexcept
	{
		_mesh = mesh;
		return *this;
	}

	std::shared_ptr<Material> Renderable::getMaterial() const noexcept
	{
		return _material;
	}

	Renderable& Renderable::setMaterial(const std::shared_ptr<Material>& material) noexcept
	{
		_material = material;
		return *this;
	}
}