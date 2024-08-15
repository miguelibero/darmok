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
		_materials = addComponent<MaterialRenderComponent>();
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
		cam.getRenderGraph().addPass(*this);
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
		if (_cam)
		{
			_cam->getRenderGraph().addPass(*this);
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

	void ForwardRenderer::beforeRenderView(bgfx::ViewId viewId, bgfx::Encoder& encoder)
	{
		_cam->beforeRenderView(viewId, encoder);
		for (auto& comp : _components)
		{
			comp->beforeRenderView(viewId, encoder);
		}
	}

	void ForwardRenderer::beforeRenderEntity(Entity entity, bgfx::Encoder& encoder)
	{
		_cam->beforeRenderEntity(entity, encoder);
		for (auto& comp : _components)
		{
			comp->beforeRenderEntity(entity, encoder);
		}
	}

	void ForwardRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		auto camEntity = _scene->getEntity(_cam.value());
		def.setName("Forward");
		def.getWriteResources().add<Texture>();
	}

	void ForwardRenderer::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		_viewId = viewId;

		_cam->configureView(viewId);
	}

	void ForwardRenderer::renderPassExecute(IRenderGraphContext& context) noexcept
	{
		auto& encoder = context.getEncoder();
		beforeRenderView(_viewId, encoder);

		auto renderables = _cam->createEntityView<Renderable>();
		for (auto entity : renderables)
		{
			auto renderable = _scene->getComponent<const Renderable>(entity);
			if (!renderable->valid())
			{
				continue;
			}
			beforeRenderEntity(entity, encoder);
			if (!renderable->render(encoder))
			{
				continue;
			}
			_materials->renderSubmit(_viewId, encoder, *renderable->getMaterial());
		}
	}
}