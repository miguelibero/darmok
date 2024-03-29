#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>

namespace darmok
{

	Mesh::Mesh(const std::shared_ptr<Material>& material, const bgfx::VertexLayout& layout, Data&& vertices, Data&& indices) noexcept
		: _material(material)
		, _layout(layout)
		, _vertices(std::move(vertices))
		, _indices(std::move(indices))
		, _vertexBuffer(bgfx::createVertexBuffer(_vertices.makeRef(), layout))
		, _indexBuffer(bgfx::createIndexBuffer(_indices.makeRef()))
	{
	}

	Mesh::Mesh(const std::shared_ptr<Material>& material, const bgfx::VertexLayout& layout, Data&& vertices) noexcept
		: _material(material)
		, _layout(layout)
		, _vertices(std::move(vertices))
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
		, _layout(other._layout)
		, _vertices(other._vertices)
		, _indices(other._indices)
		, _vertexBuffer(bgfx::createVertexBuffer(other._vertices.makeRef(), other._layout))
		, _indexBuffer(bgfx::createIndexBuffer(other._indices.makeRef()))
	{
	}

	Mesh& Mesh::operator=(const Mesh& other) noexcept
	{
		destroyHandles();

		_material = other._material;
		_layout = other._layout;
		_vertices = other._vertices;
		_indices = other._indices;

		_vertexBuffer = bgfx::createVertexBuffer(_vertices.makeRef(), _layout);
		_indexBuffer = bgfx::createIndexBuffer(_indices.makeRef());

		return *this;
	}

	Mesh::Mesh(Mesh&& other) noexcept
		: _material(other._material)
		, _layout(other._layout)
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
		_layout = other._layout;
		_indices = std::move(other._indices);
		_vertices = std::move(other._vertices);
		_vertexBuffer = other._vertexBuffer;
		_indexBuffer = other._indexBuffer;

		other._material = nullptr;
		other._vertexBuffer.idx = bgfx::kInvalidHandle;
		other._indexBuffer.idx = bgfx::kInvalidHandle;
		return *this;
	}

	const Data& Mesh::getVertexData() const
	{
		return _vertices;
	}

	const Data& Mesh::getIndexData() const
	{
		return _indices;
	}

	const std::shared_ptr<Material>& Mesh::getMaterial() const
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

	const std::shared_ptr<Mesh> createMesh(const std::shared_ptr<Material>& material, const MeshData& data)
	{
		auto& layout = material->getVertexLayout();
		VertexDataWriter writer(layout, data.positions.size());
		uint32_t i = 0;
		for (auto& pos : data.positions)
		{
			auto v = pos - glm::vec3(0.5f);
			writer.set(bgfx::Attrib::Position, i++, v);
		}
		writer.set(bgfx::Attrib::Normal, data.normals);
		writer.set(bgfx::Attrib::TexCoord0, data.texCoords);
		return std::make_shared<Mesh>(material, layout, writer.finish(), Data::copy(data.indices));
	}

	const std::shared_ptr<Mesh> Mesh::createCube(const std::shared_ptr<Material>& material)
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

	void Mesh::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, uint8_t vertexStream) const
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
		_material->submit(encoder, viewId);
	}

	MeshComponent::MeshComponent(const std::shared_ptr<Mesh>& mesh)
		: _meshes{ mesh }
	{
	}

	MeshComponent::MeshComponent(const std::vector<std::shared_ptr<Mesh>>& meshes)
		: _meshes(meshes)
	{
	}

	const std::vector<std::shared_ptr<Mesh>>& MeshComponent::getMeshes() const
	{
		return _meshes;
	}

	MeshComponent& MeshComponent::setMeshes(const std::vector<std::shared_ptr<Mesh>>& meshes)
	{
		_meshes = meshes;
		return *this;
	}

	MeshComponent& MeshComponent::setMesh(const std::shared_ptr<Mesh>& mesh)
	{
		_meshes = { mesh };
		return *this;
	}
}