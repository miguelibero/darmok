#include <darmok/mesh.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>

#include <bgfx/embedded_shader.h>
#include "generated/shaders/basic_vertex.h"
#include "generated/shaders/basic_fragment.h"



namespace darmok
{
	Mesh::Mesh(const std::shared_ptr<Material>& material, std::vector<float>&& vertices, bgfx::VertexLayout layout, std::vector<VertexIndex>&& indices) noexcept
		: _material(material)
		, _vertices(std::move(vertices))
		, _indices(std::move(indices))
		, _vertexBuffer(_vertices, layout)
		, _indexBuffer(_indices)
	{
	}

	const std::shared_ptr<Material>& Mesh::getMaterial() const
	{
		return _material;
	}

	const bgfx::VertexBufferHandle& Mesh::getVertexBuffer() const
	{
		return _vertexBuffer.getHandle();
	}

	const bgfx::IndexBufferHandle& Mesh::getIndexBuffer() const
	{
		return _indexBuffer.getHandle();
	}

	MeshComponent::MeshComponent(const std::vector<std::shared_ptr<Mesh>>& meshes)
		: _meshes(meshes)
	{
	}

	const std::vector<std::shared_ptr<Mesh>>& MeshComponent::getMeshes() const
	{
		return _meshes;
	}

	void MeshComponent::setMeshes(const std::vector<std::shared_ptr<Mesh>>& data)
	{
		_meshes = data;
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
		_uniforms.TextureColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		_uniforms.DiffuseColor = bgfx::createUniform("a_color0", bgfx::UniformType::Vec4);
	}

	void MeshRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
	{
		auto view = registry.view<const MeshComponent>();
		for (auto [entity, mesh] : view.each())
		{
			auto& meshes = mesh.getMeshes();
			if (meshes.size() == 0)
			{
				continue;
			}
			for (auto& mesh : meshes)
			{
				Transform::bgfxConfig(entity, encoder, registry);
				renderMesh(*mesh, encoder, viewId, registry);
			}
		}
	}

	void MeshRenderer::renderMesh(const Mesh& mesh, bgfx::Encoder& encoder, bgfx::ViewId viewId, Registry& registry)
	{
		mesh.getMaterial()->configure(encoder, _uniforms);

		const auto vertexStream = 0;
		encoder.setVertexBuffer(vertexStream, mesh.getVertexBuffer());
		encoder.setIndexBuffer(mesh.getIndexBuffer());

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