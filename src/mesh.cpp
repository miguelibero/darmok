#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/vertex.hpp>
#include <glm/gtx/vector_angle.hpp>

namespace darmok
{

	Mesh::Mesh(const bgfx::VertexLayout& layout, Data&& vertices, Data&& indices, const std::shared_ptr<Material>& material) noexcept
		: _layout(layout)
		, _vertices(std::move(vertices))
		, _indices(std::move(indices))
		, _material(material)
		, _vertexBuffer(bgfx::createVertexBuffer(_vertices.makeRef(), layout))
		, _indexBuffer(bgfx::createIndexBuffer(_indices.makeRef()))
	{
	}

	Mesh::Mesh(const bgfx::VertexLayout& layout, Data&& vertices, const std::shared_ptr<Material>& material) noexcept
		: _layout(layout)
		, _vertices(std::move(vertices))
		, _material(material)
		, _vertexBuffer(bgfx::createVertexBuffer(_vertices.makeRef(), layout))
		, _indexBuffer{ bgfx::kInvalidHandle }
	{
	}

	Mesh::~Mesh()
	{
		destroyHandles();
	}

	void Mesh::destroyHandles()
	{
		if (isValid(_indexBuffer))
		{
			bgfx::destroy(_vertexBuffer);
		}
		if (isValid(_indexBuffer))
		{
			bgfx::destroy(_indexBuffer);
		}
	}

	Mesh::Mesh(const Mesh& other) noexcept
		: _material(other._material)
		, _vertices(other._vertices)
		, _indices(other._indices)
		, _vertexBuffer(bgfx::createVertexBuffer(other._vertices.makeRef(), _layout))
		, _indexBuffer(bgfx::createIndexBuffer(other._indices.makeRef()))
	{
	}

	Mesh& Mesh::operator=(const Mesh& other) noexcept
	{
		destroyHandles();

		_material = other._material;
		_vertices = other._vertices;
		_indices = other._indices;

		_vertexBuffer = bgfx::createVertexBuffer(_vertices.makeRef(), _layout);
		_indexBuffer = bgfx::createIndexBuffer(_indices.makeRef());

		return *this;
	}

	Mesh::Mesh(Mesh&& other) noexcept
		: _material(other._material)
		, _vertices(std::move(other._vertices))
		, _indices(std::move(other._indices))
		, _vertexBuffer(other._vertexBuffer)
		, _indexBuffer(other._indexBuffer)
	{
		other._material = nullptr;
		other._vertexBuffer.idx = bgfx::kInvalidHandle;
		other._indexBuffer.idx = bgfx::kInvalidHandle;
	}

	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		destroyHandles();

		_material = other._material;
		_indices = std::move(other._indices);
		_vertices = std::move(other._vertices);
		_vertexBuffer = other._vertexBuffer;
		_indexBuffer = other._indexBuffer;

		other._material = nullptr;
		other._vertexBuffer.idx = bgfx::kInvalidHandle;
		other._indexBuffer.idx = bgfx::kInvalidHandle;
		return *this;
	}

	const Data& Mesh::getVertexData() const noexcept
	{
		return _vertices;
	}

	const Data& Mesh::getIndexData() const noexcept
	{
		return _indices;
	}

	const std::shared_ptr<Material>& Mesh::getMaterial() const noexcept
	{
		return _material;
	}

	void Mesh::setMaterial(const std::shared_ptr<Material>& material) noexcept
	{
		_material = material;
	}

	void Mesh::bgfxConfig(bgfx::Encoder& encoder, uint8_t vertexStream) const
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
		return std::make_shared<Mesh>(layout, std::move(vertexData), Data::copy(meshData.indices));
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

	std::shared_ptr<Mesh> MeshCreator::createSprite(const std::shared_ptr<Texture>& texture) noexcept
	{
		auto material = std::make_shared<Material>();
		material->setTexture(MaterialTextureType::Diffuse, texture);
		material->setColor(MaterialColorType::Diffuse, config.color);
		auto mesh = createQuad(Quad(texture->getImage()->getSize()));
		mesh->setMaterial(material);
		return mesh;
	}

	MeshComponent::MeshComponent(const std::shared_ptr<Mesh>& mesh) noexcept
		: _meshes{ mesh }
	{
	}

	MeshComponent::MeshComponent(const std::vector<std::shared_ptr<Mesh>>& meshes) noexcept
		: _meshes(meshes)
	{
	}

	const std::vector<std::shared_ptr<Mesh>>& MeshComponent::getMeshes() const noexcept
	{
		return _meshes;
	}

	MeshComponent& MeshComponent::setMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes) noexcept
	{
		_meshes = meshes;
		return *this;
	}

	MeshComponent& MeshComponent::setMesh(const std::shared_ptr<Mesh>& mesh) noexcept
	{
		_meshes = { mesh };
		return *this;
	}

	MeshComponent& MeshComponent::addMesh(const std::shared_ptr<Mesh>& mesh) noexcept
	{
		_meshes.push_back(mesh);
		return *this;
	}
}