#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/vertex.hpp>

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

	struct MeshData
	{
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;
		std::vector<VertexIndex> indices;
	};

	template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
	static bool writeVertexAttrib(bgfx::Attrib::Enum attrib, const bgfx::VertexLayout& layout, const std::vector<glm::vec<L, T, Q>>& collection, Data& data) noexcept
	{
		if (!layout.has(attrib))
		{
			return false;
		}
		uint32_t i = 0;
		for (auto& v : collection)
		{
			std::array<float, 4> finput = { 0, 0, 0, 0 };
			for (glm::length_t j = 0; j < L && j < finput.size(); j++)
			{
				finput[j] = static_cast<float>(v[j]);
			}
			bgfx::vertexPack(&finput.front(), false, attrib, layout, data.ptr(), i++);
		}
		return true;
	}

	static const std::shared_ptr<Mesh> createMesh(const bgfx::VertexLayout& layout, const MeshData& meshData, const glm::vec3& origin = {}, const glm::vec3& scale = glm::vec3(1)) noexcept
	{
		VertexDataWriter writer(layout, meshData.positions.size());
		uint32_t i = 0;
		for (auto& pos : meshData.positions)
		{
			auto v = scale * (pos - origin);
			writer.write(bgfx::Attrib::Position, i++, v);
		}
		writer.write(bgfx::Attrib::Normal, meshData.normals);
		writer.write(bgfx::Attrib::TexCoord0, meshData.texCoords);
		auto vertexData = writer.finish();
		return std::make_shared<Mesh>(layout, std::move(vertexData), Data::copy(meshData.indices));
	}

	const std::shared_ptr<Mesh> Mesh::createCube(const bgfx::VertexLayout& layout) noexcept
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
		return createMesh(layout, data, glm::vec3(0.5f));
	}

	const static MeshData _quadMeshData = {
	{
		{ 1, 1, 0 }, { 1, 0, 0 }, { 0, 0, 0 }, { 0, 1, 0 },
	},
	{
		{ 0, 0, 1 }, { 0, 0, 1 }, { 0, 0, 1 }, { 0, 0, 1 },
	},
	{
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
	}
	};

	const std::shared_ptr<Mesh> Mesh::createLineQuad(const bgfx::VertexLayout& layout, const glm::uvec2& size) noexcept
	{
		MeshData data = _quadMeshData;
		data.indices = { 0, 1, 1, 2, 2, 3, 3, 0 };
		return createMesh(layout, data, glm::vec3(0.5F, 0.5F, 0.F), glm::vec3(size, 1));
	}

	const std::shared_ptr<Mesh> Mesh::createQuad(const bgfx::VertexLayout& layout, const glm::uvec2& size) noexcept
	{
		MeshData data = _quadMeshData;
		data.indices = { 0, 1, 2, 2, 3, 0 };
		return createMesh(layout, data, glm::vec3(0.5F, 0.5F, 0.F), glm::vec3(size, 1));
	}

	const std::shared_ptr<Mesh> Mesh::createSphere(const bgfx::VertexLayout& layout, float radius, int lod) noexcept
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

		return createMesh(layout, data, {}, glm::vec3(0.5F));
	}

	const std::shared_ptr<Mesh> Mesh::createSprite(const std::shared_ptr<Texture>& texture, const bgfx::VertexLayout& layout, float scale, const Color& color) noexcept
	{
		auto material = std::make_shared<Material>();
		material->setTexture(MaterialTextureType::Diffuse, texture);
		material->setColor(MaterialColorType::Diffuse, color);
		auto size = glm::vec2(texture->getImage()->getSize()) * scale;
		auto mesh = Mesh::createQuad(layout, size);
		mesh->setMaterial(material);
		return mesh;
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