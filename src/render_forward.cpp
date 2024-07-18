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
		_cam = cam;
		_scene = scene;
		cam.getRenderGraph().addPass(*this);
	}

	void ForwardRenderer::shutdown() noexcept
	{
		if (_cam)
		{
			_cam->getRenderGraph().removePass(*this);
		}
		_viewId = -1;
		_cam = nullptr;
		_scene = nullptr;
	}

	void ForwardRenderer::renderPassDefine(RenderPassDefinition& def) noexcept
	{
		auto camEntity = _scene->getEntity(_cam.value());
		def.setName("Forward Renderer " + std::to_string(camEntity));
	}

	void ForwardRenderer::renderPassConfigure(bgfx::ViewId viewId) noexcept
	{
		_viewId = viewId;
	}

	void ForwardRenderer::renderPassExecute(RenderGraphResources& res) noexcept
	{
		auto& encoder = res.get<bgfx::Encoder>().value();

		beforeRenderView(_viewId);

		auto renderables = _scene->getComponentView<Renderable>();
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

	void ForwardRenderer::beforeRenderView(bgfx::ViewId viewId)
	{
		_cam->beforeRenderView(viewId);
		for (auto& comp : _components)
		{
			comp->beforeRenderView(viewId);
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

	void ForwardRenderer::addComponent(std::unique_ptr<IRenderComponent>&& comp) noexcept
	{
		_components.push_back(std::move(comp));
	}
}