#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>

namespace darmok
{
	static Entity addModelNodeToScene(Scene& scene, const ModelNode& node, Entity entity = 0, Transform* parent = nullptr)
	{
		if (entity == 0)
		{
			entity = scene.createEntity();
		}
		auto& t = scene.addComponent<Transform>(entity, node.getTransform(), parent);

		std::vector<std::shared_ptr<MeshData>> meshData;
		for (auto& mesh : node.getMeshes())
		{
			meshData.push_back(MeshData::fromModel(mesh));
		}
		scene.addComponent<Mesh>(entity, meshData);
		for (auto& child : node.getChildren())
		{
			addModelNodeToScene(scene, child, 0, &t);
		}
		return entity;
	}

    static Entity addModelToScene(Scene& scene, const Model& model, Entity entity)
	{
		if (entity == 0)
		{
			entity = scene.createEntity();
		}
		return addModelNodeToScene(scene, model.getRootNode(), entity);
	}

	Material::Material(const std::vector<bgfx::TextureHandle>& textures)
		: _textures(textures)
	{
	}

	std::shared_ptr<Material> Material::fromModel(const ModelMaterial& modelMaterial)
	{
		std::vector<bgfx::TextureHandle> textures;
		for (auto& modelTex : modelMaterial.getTextures())
		{
			auto path = modelTex.getPath();
			auto tex = AssetContext::get().loadTexture(path);
			textures.push_back(tex);
		}
		return std::make_shared<Material>(textures);
	}

	MeshData::MeshData(const std::shared_ptr<Material>& material, std::vector<float>&& vertices, bgfx::VertexLayout layout, std::vector<VertexIndex>&& indices) noexcept
		: _material(material)
		, _vertices(std::move(vertices))
		, _indices(std::move(indices))
		, _vertexBuffer(_vertices, layout)
		, _indexBuffer(_indices)
	{
	}

	std::shared_ptr<MeshData> MeshData::fromModel(const ModelMesh& modelMesh)
	{
		bgfx::VertexLayout layout;
		layout.begin();
		if (!modelMesh.getVertices().empty())
		{
			layout.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		}
		if (!modelMesh.getNormals().empty())
		{
			layout.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
		}
		if (!modelMesh.getTangents().empty())
		{
			layout.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float);
		}
		if (!modelMesh.getBitangents().empty())
		{
			layout.add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Float);
		}
		auto i = 0;
		for(auto& texCoords : modelMesh.getTexCoords())
		{
			auto attrib = bgfx::Attrib::TexCoord0 + i++;
			layout.add((bgfx::Attrib::Enum)attrib, texCoords.getCompCount(), bgfx::AttribType::Float);
		}
		layout.end();

		auto material = Material::fromModel(modelMesh.getMaterial());

		std::vector<float> vertices;
		for (size_t i = 0; i < modelMesh.getVertexCount(); i++)
		{
		}
		std::vector<VertexIndex> indices;

		return std::make_shared<MeshData>(material, std::move(vertices), layout, std::move(indices));
	}

	const std::shared_ptr<Material>& MeshData::getMaterial() const
	{
		return _material;
	}

	const bgfx::VertexBufferHandle& MeshData::getVertexBuffer() const
	{
		return _vertexBuffer.getHandle();
	}

	const bgfx::IndexBufferHandle& MeshData::getIndexBuffer() const
	{
		return _indexBuffer.getHandle();
	}

	Mesh::Mesh(const std::vector<std::shared_ptr<MeshData>>& data)
		: _datas(data)
	{
	}

	const std::vector<std::shared_ptr<MeshData>>& Mesh::getData() const
	{
		return _datas;
	}

	void Mesh::setData(const std::vector<std::shared_ptr<MeshData>>& data)
	{
		_datas = data;
	}

	MeshRenderer::MeshRenderer(bgfx::ProgramHandle program)
		: _program(program)
	{
	}

	MeshRenderer::~MeshRenderer()
	{

	}

	void MeshRenderer::init(Registry& registry)
	{

	}

	void MeshRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
	{
	}
}