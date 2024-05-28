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
	void ForwardRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_cam = cam;
		_scene = scene;
		_app = app;
	}

	void ForwardRenderer::shutdown() noexcept
	{
		_cam = nullptr;
		_scene = nullptr;
		_app = nullptr;
	}

	const std::string ForwardRenderer::_name = "Forward Renderer";

	bgfx::ViewId ForwardRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
		if (!_cam)
		{
			return viewId;
		}

		auto camEntity = _scene->getEntity(_cam.value());
		std::string name = _name + " " + std::to_string(camEntity);
		bgfx::setViewName(viewId, name.c_str());

		_cam->beforeRenderView(encoder, viewId);

		auto& registry = _scene->getRegistry();
		auto renderables = _cam->createEntityView<Renderable>(registry);
		auto rendered = false;
		for (auto entity : renderables)
		{
			auto& renderable = registry.get<const Renderable>(entity);
			if (renderable)
			{
				rendered = true;
				_cam->beforeRenderEntity(entity, encoder, viewId);
				renderable.render(encoder, viewId);
			}
		}
		_cam->afterRenderView(encoder, viewId);

		if (!rendered)
		{
			return viewId;
		}

		return ++viewId;
	}
}