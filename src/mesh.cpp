#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include <darmok/vertex.hpp>
#include <darmok/shape.hpp>
#include <darmok/data.hpp>
#include <darmok/material.hpp>

namespace darmok
{
	Mesh::Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, const DataView& indices, bool dynamic) noexcept
		: _layout(layout)
		, _vertexBuffer{ bgfx::kInvalidHandle }
		, _indexBuffer{ bgfx::kInvalidHandle }
		, _vertexSize(vertices.size())
		, _indexSize(indices.size())
		, _dynamic(dynamic)
	{
		if (dynamic)
		{
			uint64_t flags = BGFX_BUFFER_ALLOW_RESIZE;
			_vertexBuffer = bgfx::createDynamicVertexBuffer(vertices.copyMem(), layout, flags).idx;
			if (!indices.empty())
			{
				_indexBuffer = bgfx::createDynamicIndexBuffer(indices.copyMem(), flags).idx;
			}
		}
		else
		{
			_vertexBuffer = bgfx::createVertexBuffer(vertices.copyMem(), layout).idx;
			if (!indices.empty())
			{
				_indexBuffer = bgfx::createIndexBuffer(indices.copyMem()).idx;
			}
		}
	}

	Mesh::Mesh(const bgfx::VertexLayout& layout, const DataView& vertices, bool dynamic) noexcept
		: Mesh(layout, vertices, DataView(), dynamic)
	{
	}

	const bgfx::VertexLayout& Mesh::getVertexLayout() const noexcept
	{
		return _layout;
	}

	Mesh::~Mesh() noexcept
	{
		if (_vertexBuffer != bgfx::kInvalidHandle)
		{
			if (_dynamic)
			{
				bgfx::destroy(bgfx::DynamicVertexBufferHandle{ _vertexBuffer });
			}
			else
			{
				bgfx::destroy(bgfx::VertexBufferHandle{ _vertexBuffer });
			}
		}
		if (_indexBuffer != bgfx::kInvalidHandle)
		{
			if (_dynamic)
			{
				bgfx::destroy(bgfx::DynamicIndexBufferHandle{ _indexBuffer });
			}
			else
			{
				bgfx::destroy(bgfx::IndexBufferHandle{ _indexBuffer });
			}
		}
	}

	Mesh::Mesh(Mesh&& other) noexcept
		: _layout(other._layout)
		, _vertexBuffer(other._vertexBuffer)
		, _indexBuffer(other._indexBuffer)
		, _vertexSize(other._vertexSize)
		, _indexSize(other._indexSize)
		, _dynamic(other._dynamic)
	{
		other._vertexBuffer = bgfx::kInvalidHandle;
		other._indexBuffer = bgfx::kInvalidHandle;
		other._vertexSize = 0;
		other._indexSize = 0;
	}

	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		_layout = other._layout;
		_vertexBuffer = other._vertexBuffer;
		_indexBuffer = other._indexBuffer;
		_vertexSize = other._vertexSize;
		_indexSize = other._indexSize;
		_dynamic = other._dynamic;
		other._vertexBuffer = bgfx::kInvalidHandle;
		other._indexBuffer = bgfx::kInvalidHandle;
		other._vertexSize = 0;
		other._indexSize = 0;
		return *this;
	}


	void Mesh::updateVertices(const DataView& data, size_t offset)
	{
		if (!_dynamic)
		{
			throw std::runtime_error("cannot update static mesh");
		}
		bgfx::DynamicVertexBufferHandle handle{ _vertexBuffer };
		bgfx::update(handle, offset, data.copyMem());
	}
	
	void Mesh::updateIndices(const DataView& data, size_t offset)
	{
		if (!_dynamic)
		{
			throw std::runtime_error("cannot update static mesh");
		}
		bgfx::DynamicIndexBufferHandle handle{ _indexBuffer };
		bgfx::update(handle, offset, data.copyMem());
	}

	std::string Mesh::to_string() const noexcept
	{
		auto stride = getVertexLayout().getStride();
		auto numVerts = _vertexSize / stride;
		auto numIdx = _indexSize / sizeof(VertexIndex);
		return "Mesh(" + std::to_string(numVerts) + " vertices, "
			+ std::to_string(stride) + " stride, "
			+ std::to_string(numIdx) + " indices)";
	}

	void Mesh::render(bgfx::Encoder& encoder, uint8_t vertexStream) const
	{
		if (_vertexBuffer == bgfx::kInvalidHandle)
		{
			throw std::runtime_error("invalid mesh vertex buffer");
		}
		if (_dynamic)
		{
			encoder.setVertexBuffer(vertexStream, bgfx::DynamicVertexBufferHandle{ _vertexBuffer });
		}
		else
		{
			encoder.setVertexBuffer(vertexStream, bgfx::VertexBufferHandle{ _vertexBuffer });
		}
		if (_indexBuffer != bgfx::kInvalidHandle)
		{
			if (_dynamic)
			{
				encoder.setIndexBuffer(bgfx::DynamicIndexBufferHandle{ _indexBuffer });
			}
			else
			{
				encoder.setIndexBuffer(bgfx::IndexBufferHandle{ _indexBuffer });
			}
		}
	}

	MeshCreator::MeshCreator(const bgfx::VertexLayout& layout) noexcept
		: layout(layout)
		, config{}
	{
	}

	std::shared_ptr<Mesh> MeshCreator::createMesh(const MeshData& meshData) noexcept
	{
		return createMesh(meshData, config);
	}

	std::shared_ptr<Mesh> MeshCreator::createMesh(const MeshData& meshData, const MeshCreationConfig& cfg) noexcept
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
		return std::make_shared<Mesh>(layout, DataView(vertexData), DataView(meshData.indices), cfg.dynamic);
	}

	std::shared_ptr<Mesh> MeshCreator::createCube() noexcept
	{
		return createCube(Cube::standard());
	}

	std::shared_ptr<Mesh> MeshCreator::createCube(const Cube& cube) noexcept
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

	std::shared_ptr<Mesh> MeshCreator::createQuadMesh(const bgfx::VertexLayout& layout, const MeshData& data, const Quad& quad) noexcept
	{
		auto cfg = config;
		cfg.scale *= glm::vec3(quad.size, 0);
		cfg.offset += glm::vec3(quad.origin - glm::vec2(0.5F), 0);
		return createMesh(data, cfg);
	}

	std::shared_ptr<Mesh> MeshCreator::createQuad(const Quad& quad) noexcept
	{
		MeshData data = _quadMeshData;
		data.indices = { 0, 1, 2, 2, 3, 0 };
		return createQuadMesh(layout, data, quad);
	}

	std::shared_ptr<Mesh> MeshCreator::createLineQuad(const Quad& quad) noexcept
	{
		MeshData data = _quadMeshData;
		data.indices = { 0, 1, 1, 2, 2, 3, 3, 0 };
		return createQuadMesh(layout, data, quad);
	}

	std::shared_ptr<Mesh> MeshCreator::createSphere(int lod) noexcept
	{
		return createSphere(Sphere::standard());
	}

	std::shared_ptr<Mesh> MeshCreator::createQuad() noexcept
	{
		return createQuad(Quad::standard());
	}

	std::shared_ptr<Mesh> MeshCreator::createLineQuad() noexcept
	{
		return createLineQuad(Quad::standard());
	}

	std::shared_ptr<Mesh> MeshCreator::createSphere(const Sphere& sphere, int lod) noexcept
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

	std::shared_ptr<Mesh> MeshCreator::createRay(const Ray& ray) noexcept
	{
		return createLine(ray.toLine());
	}

	std::shared_ptr<Mesh> MeshCreator::createLine(const Line& line) noexcept
	{
		return createLines({ line });
	}

	std::shared_ptr<Mesh> MeshCreator::createLines(const std::vector<Line>& lines) noexcept
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

	Renderable::Renderable(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Material>& material) noexcept
		: _mesh(mesh)
		, _material(material)
	{
	}

	Renderable::Renderable(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Texture>& texture) noexcept
		: _mesh(mesh)
		, _material(std::make_shared<Material>(texture))
	{
	}

	Renderable::Renderable(const std::shared_ptr<Material>& material) noexcept
		: _material(material)
	{
	}

	std::shared_ptr<Mesh> Renderable::getMesh() const noexcept
	{
		return _mesh;
	}

	Renderable& Renderable::setMesh(const std::shared_ptr<Mesh>& mesh) noexcept
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