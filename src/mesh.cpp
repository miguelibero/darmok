#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/vertex.hpp>
#include <darmok/transform.hpp>

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
		, _indices(std::move(other._indices))
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

	const static std::vector<glm::vec3> _cubePositions = {
		{ 1,  1,  1 }, { 0,  1,  1 }, { 0,  0,  1 }, { 1,  0,  1 },
		{ 1,  1,  0 }, { 1,  0,  0 }, { 0,  0,  0 }, { 0,  1,  0 },
		{ 1,  1,  1 }, { 1,  1,  0 }, { 0,  1,  0 }, { 0,  1,  1 },
		{ 1,  0,  1 }, { 0,  0,  1 }, { 0,  0,  0 }, { 1,  0,  0 },
		{ 1,  1,  1 }, { 1,  0,  1 }, { 1,  0,  0 }, { 1,  1,  0 },
		{ 0,  1,  1 }, { 0,  1,  0 }, { 0,  0,  0 }, { 0,  0,  1 },
	};
	const static std::vector<glm::vec3> _cubeNormals = {
		{  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 }, {  0,  0,  1 },
		{  0,  0, -1 }, {  0,  0, -1 }, {  0,  0, -1 }, {  0,  0, -1 },
		{  0,  1,  0 }, {  0,  1,  0 }, {  0,  1,  0 }, {  0,  1,  0 },
		{  0, -1,  0 }, {  0, -1,  0 }, {  0, -1,  0 }, {  0, -1,  0 },
		{  1,  0,  0 }, {  1,  0,  0 }, {  1,  0,  0 }, {  1,  0,  0 },
		{ -1,  0,  0 }, { -1,  0,  0 }, { -1,  0,  0 }, { -1,  0,  0 },
	};
	const static std::vector<glm::vec2> _cubeTexCoords = {
		{ 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 },
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 },
		{ 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 },
	};

	const static std::vector<VertexIndex> _cubeIndices = {
		 0,  1,  2,  2,  3,  0,
		 4,  5,  6,  6,  7,  4,
		 8,  9, 10, 10, 11,  8,
		12, 13, 14, 14, 15, 12,
		16, 17, 18, 18, 19, 16,
		20, 21, 22, 22, 23, 20,
	};

	const std::shared_ptr<Mesh> Mesh::createCube(const std::shared_ptr<Material>& material)
	{
		auto& layout = material->getVertexLayout();
		VertexDataWriter writer(layout, _cubePositions.size());
		uint32_t i = 0;
		for (auto& pos : _cubePositions)
		{
			auto v = pos - glm::vec3(0.5f);
			writer.set(bgfx::Attrib::Position, i++, glm::value_ptr(v));
		}
		writer.set(bgfx::Attrib::Normal, _cubeNormals);
		writer.set(bgfx::Attrib::TexCoord0, _cubeTexCoords);
		return std::make_shared<Mesh>(material, layout, writer.release(), Data::copy(_cubeIndices));
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

	void MeshRenderer::render(EntityRuntimeView& entities, bgfx::Encoder& encoder, bgfx::ViewId viewId)
	{
		auto& registry = getRegistry();
		entities.iterate(registry.storage<MeshComponent>());
		for (auto entity : entities)
		{
			auto& comp = registry.get<const MeshComponent>(entity);
			auto& meshes = comp.getMeshes();
			if (meshes.empty())
			{
				continue;
			}
			Transform::bgfxConfig(entity, encoder, registry);
			for (auto& mesh : meshes)
			{
				mesh->render(encoder, viewId);
			}
		}
	}
}