#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/light.hpp>
#include <darmok/material.hpp>
#include <darmok/scene.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/texture.hpp>
#include "render_samplers.hpp"

namespace darmok
{
	ForwardRenderer::ForwardRenderer() noexcept
	{
	}

	ForwardRenderer::~ForwardRenderer() noexcept
	{
		// empty on purpose
	}

	void ForwardRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_cam = cam;
		_scene = scene;
		_app = app;
		_materials = app.getComponent<MaterialAppComponent>();
		if (!_materials)
		{
			_materials = app.addComponent<MaterialAppComponent>();
		}

		renderReset();
	}

	void ForwardRenderer::renderReset() noexcept
	{
		if (!_cam)
		{
			return;
		}
		auto size = _cam->getCurrentViewport().size;
		_passHandle = _cam->getRenderGraph().addPass(*this);
	}

	void ForwardRenderer::shutdown() noexcept
	{
		if (_cam)
		{
			_cam->getRenderGraph().removePass(_passHandle);
		}
		_passHandle.reset();
		_cam.reset();
		_scene.reset();
		_app.reset();
	}

	void ForwardRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		def.setName("Forward");
	}

	void ForwardRenderer::renderPassConfigure(bgfx::ViewId viewId)
	{
		if (_cam)
		{
			_cam->configureView(viewId);
		}
	}

	void ForwardRenderer::renderEntities(IRenderGraphContext& context, EntityRuntimeView& view, MaterialOpacity opacity) noexcept
	{
		auto& encoder = context.getEncoder();
		auto viewId = context.getViewId();
		for (auto entity : view)
		{
			auto renderable = _scene->getComponent<const Renderable>(entity);
			if (!renderable->valid())
			{
				continue;
			}
			if (renderable->getMaterial()->getOpacity() != opacity)
			{
				continue;
			}
			_cam->beforeRenderEntity(entity, context);
			if (!renderable->render(encoder))
			{
				continue;
			}
			_materials->renderSubmit(viewId, encoder, *renderable->getMaterial());
		}
	}

	void ForwardRenderer::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		if (!_cam || !_cam->isEnabled())
		{
			return;
		}
		_cam->beforeRenderView(context);
		auto view = _cam->createEntityView<Renderable>();
		renderEntities(context, view, MaterialOpacity::Opaque);
		renderEntities(context, view, MaterialOpacity::Cutout);
		renderEntities(context, view, MaterialOpacity::Transparent);
	}
}