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
		_cam->getRenderGraph().addPass(*this);
	}

	void ForwardRenderer::shutdown() noexcept
	{
		if (_cam)
		{
			_cam->getRenderGraph().removePass(*this);
		}
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

	void ForwardRenderer::renderEntities(IRenderGraphContext& context, const std::vector<Entity>& entities, OpacityType opacity) noexcept
	{
		auto& encoder = context.getEncoder();
		auto viewId = context.getViewId();
		for (auto entity : entities)
		{
			auto renderable = _scene->getComponent<const Renderable>(entity);
			if (!renderable->valid())
			{
				continue;
			}
			if (renderable->getMaterial()->getOpacityType() != opacity)
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
		auto entities = _cam->getEntities<Renderable>();
		renderEntities(context, entities, OpacityType::Opaque);
		renderEntities(context, entities, OpacityType::Mask);
		renderEntities(context, entities, OpacityType::Transparent);
	}
}