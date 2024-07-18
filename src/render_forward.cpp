#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/light.hpp>
#include <darmok/material.hpp>
#include <darmok/scene.hpp>
#include <darmok/render.hpp>

namespace darmok
{
	ForwardRenderer::ForwardRenderer() noexcept
		: _viewId(-1)
	{
	}

	void ForwardRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		Renderer::init(cam, scene, app);
		cam.getRenderGraph().addPass(*this);
	}

	void ForwardRenderer::shutdown() noexcept
	{
		if (_cam)
		{
			_cam->getRenderGraph().removePass(*this);
		}
		_viewId = -1;
		Renderer::shutdown();
	}

	void ForwardRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		auto camEntity = _scene->getEntity(_cam.value());
		def.setName("Forward Renderer " + std::to_string(camEntity));
	}

	void ForwardRenderer::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		_viewId = viewId;

		static const uint16_t clearFlags = BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL;
		bgfx::setViewClear(viewId, clearFlags, 1.F, 0U);

		_cam->configureView(viewId);
	}

	void ForwardRenderer::renderPassExecute(RenderGraphResources& res) noexcept
	{
		auto& encoder = res.get<bgfx::Encoder>().value();

		beforeRenderView(_viewId);

		auto renderables = _cam->createEntityView<Renderable>();
		for (auto entity : renderables)
		{
			auto renderable = _scene->getComponent<const Renderable>(entity);
			if (!renderable->valid())
			{
				continue;
			}
			beforeRenderEntity(entity, encoder);
			renderable->render(encoder, _viewId);
		}
	}
}