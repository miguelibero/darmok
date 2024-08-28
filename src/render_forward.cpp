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
		: _viewId(-1)
	{
		_materials = addComponent<MaterialRenderComponent>();
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
		for (auto& comp : _components)
		{
			comp->init(cam, scene, app);
		}
		renderReset();
	}

	void ForwardRenderer::update(float deltaTime)
	{
		for (auto& comp : _components)
		{
			comp->update(deltaTime);
		}
	}

	void ForwardRenderer::renderReset() noexcept
	{
		if (!_cam)
		{
			return;
		}
		auto size = _cam->getCurrentViewport().size;
		_cam->getRenderGraph().addPass(*this);
		for (auto& comp : _components)
		{
			comp->renderReset();
		}
	}

	void ForwardRenderer::shutdown() noexcept
	{
		if (_cam)
		{
			_cam->getRenderGraph().removePass(*this);
		}
		for (auto itr = _components.rbegin(); itr != _components.rend(); ++itr)
		{
			(*itr)->shutdown();
		}

		_viewId = -1;
		_cam.reset();
		_scene.reset();
		_app.reset();
	}

	void ForwardRenderer::addComponent(std::unique_ptr<IRenderComponent>&& comp) noexcept
	{
		if (_cam)
		{
			comp->init(_cam.value(), _scene.value(), _app.value());
		}
		_components.push_back(std::move(comp));
	}

	void ForwardRenderer::beforeRenderView(IRenderGraphContext& context)
	{
		_cam->beforeRenderView(context.getViewId());
		for (auto& comp : _components)
		{
			comp->beforeRenderView(context);
		}
	}

	void ForwardRenderer::beforeRenderEntity(Entity entity, IRenderGraphContext& context)
	{
		_cam->beforeRenderEntity(entity, context.getEncoder());
		for (auto& comp : _components)
		{
			comp->beforeRenderEntity(entity, context);
		}
	}

	void ForwardRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		def.setName("Forward");
		for (auto& comp : _components)
		{
			comp->renderPassDefine(def);
		}
	}

	void ForwardRenderer::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		_viewId = viewId;
		_cam->configureView(viewId);
		for (auto& comp : _components)
		{
			comp->renderPassConfigure(viewId);
		}
	}

	void ForwardRenderer::renderEntities(IRenderGraphContext& context, EntityRuntimeView& view, MaterialOpacity opacity) noexcept
	{
		auto& encoder = context.getEncoder();
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
			beforeRenderEntity(entity, context);
			if (!renderable->render(encoder))
			{
				continue;
			}
			_materials->renderSubmit(_viewId, encoder, *renderable->getMaterial());
		}
	}

	void ForwardRenderer::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		if (!_cam->isEnabled())
		{
			return;
		}
		beforeRenderView(context);
		auto view = _cam->createEntityView<Renderable>();
		renderEntities(context, view, MaterialOpacity::Opaque);
		renderEntities(context, view, MaterialOpacity::Cutout);
		renderEntities(context, view, MaterialOpacity::Transparent);
	}
}