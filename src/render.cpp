#include <darmok/render.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/shape.hpp>
#include <darmok/texture.hpp>

namespace darmok
{
	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept
		: _mesh(mesh)
		, _material(material)
		, _enabled(true)
	{
	}

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept
		: Renderable(mesh, std::make_shared<Material>(program, texture))
	{
	}

	Renderable::Renderable(const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& program, const Color& color) noexcept
		: Renderable(mesh, std::make_shared<Material>(program, color))
	{
	}

	Renderable::Renderable(const std::shared_ptr<Material>& material) noexcept
		: Renderable(nullptr, material)
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

	bool Renderable::isEnabled() const noexcept
	{
		return _enabled;
	}

	Renderable& Renderable::setEnabled(bool enabled) noexcept
	{
		_enabled = enabled;
		return *this;
	}

	bool Renderable::valid() const noexcept
	{
		return _mesh != nullptr && _material != nullptr && _material->valid();
	}

	bool Renderable::render(bgfx::Encoder& encoder) const
	{
		if (!_enabled)
		{
			return false;
		}
		return _mesh->render(encoder);
	}

	ScreenRenderPass::ScreenRenderPass() noexcept
		: _priority(-RenderPassDefinition::kMaxPriority)
		, _texUniform{ bgfx::kInvalidHandle }
	{
	}

	ScreenRenderPass::~ScreenRenderPass() noexcept
	{
		bgfx::destroy(_texUniform);
	}

	ScreenRenderPass& ScreenRenderPass::setName(const std::string& name) noexcept
	{
		_name = name;
		return *this;
	}

	ScreenRenderPass& ScreenRenderPass::setPriority(int priority)
	{
		_priority = priority;
		return *this;
	}
	
	ScreenRenderPass& ScreenRenderPass::setProgram(const std::shared_ptr<Program>& prog) noexcept
	{
		_program = prog;
		_mesh = MeshData(Rectangle()).createMesh(prog->getVertexLayout());
		return *this;
	}

	ScreenRenderPass& ScreenRenderPass::setViewport(std::optional<Viewport> vp) noexcept
	{
		_viewport = vp;
		return *this;
	}

	void ScreenRenderPass::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		def.setName(_name);
		def.setPriority(_priority);
	}

	void ScreenRenderPass::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		if (!isValid(_texUniform))
		{
			_texUniform = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		}

		static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);
		if (_viewport)
		{
			_viewport->configureView(viewId);
		}
		else
		{
			bgfx::setViewRect(viewId, 0, 0, bgfx::BackbufferRatio::Equal);
		}
	}

	void ScreenRenderPass::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		auto& encoder = context.getEncoder();
		auto viewId = context.getViewId();

		auto tex = context.getResources().get<Texture>();

		if (!_mesh || !_program || !tex)
		{
			encoder.touch(viewId);
			return;
		}

		encoder.setTexture(0, _texUniform, tex->getHandle());
		_mesh->render(encoder);

		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_ALPHA
			;

		encoder.submit(viewId, _program->getHandle(), state);
	}
}