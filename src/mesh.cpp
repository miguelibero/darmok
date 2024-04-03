#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>

namespace darmok
{

	Mesh::Mesh(const std::shared_ptr<Material>& material, Data&& vertices, Data&& indices) noexcept
		: _material(material)
		, _vertices(std::move(vertices))
		, _indices(std::move(indices))
		, _vertexBuffer(bgfx::createVertexBuffer(_vertices.makeRef(), material->getVertexLayout()))
		, _indexBuffer(bgfx::createIndexBuffer(_indices.makeRef()))
	{
	}

	Mesh::Mesh(const std::shared_ptr<Material>& material, Data&& vertices) noexcept
		: _material(material)
		, _vertices(std::move(vertices))
		, _vertexBuffer(bgfx::createVertexBuffer(_vertices.makeRef(), material->getVertexLayout()))
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
			_vertexBuffer.idx = bgfx::kInvalidHandle;
		}
		if (isValid(_indexBuffer))
		{
			bgfx::destroy(_indexBuffer);
			_indexBuffer.idx = bgfx::kInvalidHandle;
		}
	}

	Mesh::Mesh(const Mesh& other) noexcept
		: _material(other._material)
		, _vertices(other._vertices)
		, _indices(other._indices)
		, _vertexBuffer(bgfx::createVertexBuffer(other._vertices.makeRef(), other.getMaterial()->getVertexLayout()))
		, _indexBuffer(bgfx::createIndexBuffer(other._indices.makeRef()))
	{
	}

	Mesh& Mesh::operator=(const Mesh& other) noexcept
	{
		destroyHandles();

		_material = other._material;
		_vertices = other._vertices;
		_indices = other._indices;

		_vertexBuffer = bgfx::createVertexBuffer(_vertices.makeRef(), _material->getVertexLayout());
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

	struct MeshData
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;
		std::vector<VertexIndex> indices;
	};

	static const std::shared_ptr<Mesh> createMesh(const std::shared_ptr<Material>& material, const MeshData& data) noexcept
	{
		VertexDataWriter writer(material->getVertexLayout(), data.positions.size());
		uint32_t i = 0;
		for (auto& pos : data.positions)
		{
			auto v = pos - glm::vec3(0.5f);
			writer.set(bgfx::Attrib::Position, i++, v);
		}
		writer.set(bgfx::Attrib::Normal, data.normals);
		writer.set(bgfx::Attrib::TexCoord0, data.texCoords);
		return std::make_shared<Mesh>(material, writer.finish(), Data::copy(data.indices));
	}

	const std::shared_ptr<Mesh> Mesh::createCube(const std::shared_ptr<Material>& material) noexcept
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
		return createMesh(material, data);
	}

	const std::shared_ptr<Mesh> Mesh::createQuad(const std::shared_ptr<Material>& material, const glm::uvec2& size) noexcept
	{
		static const std::vector<glm::vec3> normals{
				{ 0, 0, 1 }, { 0, 0, 1 }, { 0, 0, 1 }, { 0, 0, 1 },
		};
		static const std::vector<glm::vec2> texCoords{
				{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
		};
		static const std::vector<VertexIndex> lineIndices{ 0, 1, 1, 2, 2, 3, 3, 0 };
		static const std::vector<VertexIndex> triIndices{ 0, 1, 2, 2, 3, 0 };

		auto lines = material->getPrimitiveType() == MaterialPrimitiveType::Line;

		return createMesh(material, MeshData{
			{
				{ size.x,  size.y, 0 }, { size.x, 0, 0 }, { 0, 0, 0 }, { 0, size.y, 0 },
			},
			normals,
			texCoords,
			lines ? lineIndices : triIndices
		});
	}

	const std::shared_ptr<Mesh> Mesh::createSphere(const std::shared_ptr<Material>& material, float radius, float lod) noexcept
	{
		return createCube(material);
	}

	const std::shared_ptr<Mesh> Mesh::createSprite(const std::shared_ptr<Texture>& texture, const ProgramDefinition& progDef, float scale, const Color& color) noexcept
	{
		auto material = std::make_shared<Material>(progDef);
		material->setTexture(MaterialTextureType::Diffuse, texture);
		material->setColor(MaterialColorType::Diffuse, color);
		auto size = glm::vec2(texture->getImage()->getSize()) * scale;
		return Mesh::createQuad(material, size);
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