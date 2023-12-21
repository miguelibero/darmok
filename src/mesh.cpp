#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/material.hpp>

#include <bgfx/embedded_shader.h>
#include "generated/shaders/basic_vertex.h"
#include "generated/shaders/basic_fragment.h"

#include <filesystem>

namespace darmok
{
	static Entity addModelNodeToScene(Scene& scene, const ModelNode& node, const std::string& basePath, Entity entity = 0, Transform* parent = nullptr)
	{
		if (entity == 0)
		{
			entity = scene.createEntity();
		}
		auto& t = scene.addComponent<Transform>(entity, node.getTransform(), parent);

		std::vector<std::shared_ptr<MeshData>> meshData;
		for (auto& mesh : node.getMeshes())
		{
			meshData.push_back(MeshData::fromModel(mesh, basePath));
		}
		scene.addComponent<Mesh>(entity, meshData);
		for (auto& child : node.getChildren())
		{
			addModelNodeToScene(scene, child, basePath, 0, &t);
		}
		return entity;
	}

    Entity addModelToScene(Scene& scene, const std::shared_ptr<Model>& model, Entity entity)
	{
		if (entity == 0)
		{
			entity = scene.createEntity();
		}
		auto basePath = std::filesystem::path(model->getPath()).parent_path().string();
		return addModelNodeToScene(scene, model->getRootNode(), basePath, entity);
	}

	MeshData::MeshData(const std::shared_ptr<Material>& material, std::vector<float>&& vertices, bgfx::VertexLayout layout, std::vector<VertexIndex>&& indices) noexcept
		: _material(material)
		, _vertices(std::move(vertices))
		, _indices(std::move(indices))
		, _vertexBuffer(_vertices, layout)
		, _indexBuffer(_indices)
	{
	}

	static bgfx::VertexLayout createModelMeshVertexLayout(const ModelMesh& modelMesh)
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
		for (auto& texCoords : modelMesh.getTexCoords())
		{
			auto attrib = bgfx::Attrib::TexCoord0 + i++;
			layout.add((bgfx::Attrib::Enum)attrib, texCoords.getCompCount(), bgfx::AttribType::Float);
		}
		layout.end();
		return layout;
	}

	std::vector<float> createModelMeshVertexData(const ModelMesh& modelMesh, const bgfx::VertexLayout& layout)
	{
		std::vector<float> vertices;
		vertices.reserve(layout.getSize(modelMesh.getVertexCount()) / sizeof(float));
		auto& verts = modelMesh.getVertices();
		auto& norms = modelMesh.getNormals();
		auto& tangs = modelMesh.getTangents();
		auto& btngs = modelMesh.getBitangents();
		for (size_t i = 0; i < modelMesh.getVertexCount(); i++)
		{
			if(!verts.empty())
			{
				auto& v = verts[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}
			if (!norms.empty())
			{
				auto& v = norms[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}
			if (!tangs.empty())
			{
				auto& v = tangs[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}
			if (!btngs.empty())
			{
				auto& v = btngs[i];
				vertices.insert(vertices.end(), { v.x, v.y, v.z });
			}

			for (auto& texCoords : modelMesh.getTexCoords())
			{
				auto& v = texCoords.getCoords()[i];
				switch (texCoords.getCompCount())
				{
				case 1:
					vertices.push_back(v.x);
					break;
				case 2:
					vertices.push_back(v.x);
					vertices.push_back(v.y);
					break;
				case 3:
					vertices.push_back(v.x);
					vertices.push_back(v.y);
					vertices.push_back(v.z);
					break;
				default:
					break;
				}
			}
		}
		return vertices;
	}

	std::vector<VertexIndex> createModelMeshIndexData(const ModelMesh& modelMesh)
	{
		std::vector<VertexIndex> indices;
		for (auto& face : modelMesh.getFaces())
		{
			indices.insert(indices.end(), face.getIndices().begin(), face.getIndices().end());
		}
		return indices;
	}

	std::shared_ptr<MeshData> MeshData::fromModel(const ModelMesh& modelMesh, const std::string& basePath)
	{
		auto layout = createModelMeshVertexLayout(modelMesh);
		auto material = Material::fromModel(modelMesh.getMaterial(), basePath);
		auto vertices = createModelMeshVertexData(modelMesh, layout);
		auto indices = createModelMeshIndexData(modelMesh);
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

	static const bgfx::EmbeddedShader _meshEmbeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(basic_vertex),
		BGFX_EMBEDDED_SHADER(basic_fragment),
		BGFX_EMBEDDED_SHADER_END()
	};

	MeshRenderer::MeshRenderer(bgfx::ProgramHandle program)
		: _program(program)
	{
	}

	MeshRenderer::~MeshRenderer()
	{

	}

	void MeshRenderer::init(Registry& registry)
	{
		if (!isValid(_program))
		{
			auto type = bgfx::getRendererType();
			_program = bgfx::createProgram(
				bgfx::createEmbeddedShader(_meshEmbeddedShaders, type, "basic_vertex"),
				bgfx::createEmbeddedShader(_meshEmbeddedShaders, type, "basic_fragment"),
				true
			);
		}
		_texColorUniforn = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
	}

	void MeshRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
	{
		auto view = registry.view<const Mesh>();
		for (auto [entity, mesh] : view.each())
		{
			auto& data = mesh.getData();
			if (data.size() == 0)
			{
				continue;
			}
			for (auto& meshData : data)
			{
				Transform::bgfxConfig(entity, encoder, registry);
				renderData(*meshData, encoder, viewId, registry);
			}
		}
	}

	void MeshRenderer::renderData(const MeshData& data, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
	{

		auto textureUnit = 0;
		for (auto& texture : data.getMaterial()->getTextures(MaterialTextureType::Diffuse))
		{
			encoder.setTexture(textureUnit, _texColorUniforn, texture.getTexture()->getHandle());
			textureUnit++;
		}
		const auto vertexStream = 0;
		encoder.setVertexBuffer(vertexStream, data.getVertexBuffer());
		encoder.setIndexBuffer(data.getIndexBuffer());

		// TODO: configure state
		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
			;
		encoder.setState(state);
		encoder.submit(viewId, _program);
	}
}