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
	{
	}

	ForwardRenderer& ForwardRenderer::setInvalidMaterial(const std::shared_ptr<Material>& mat) noexcept
	{
		_invalidMaterial = mat;
		return *this;
	}

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

	bgfx::ViewId ForwardRenderer::render(bgfx::ViewId viewId) const
	{
		if (!_cam)
		{
			return viewId;
		}

		auto encoder = bgfx::begin();

		auto camEntity = _scene->getEntity(_cam.value());
		std::string name = _name + " " + std::to_string(camEntity);
		bgfx::setViewName(viewId, &name.front(), name.size());

		_cam->beforeRenderView(*encoder, viewId);

		auto& registry = _scene->getRegistry();
		auto renderables = _cam->createEntityView<Renderable>(registry);
		for (auto entity : renderables)
		{
			auto& renderable = registry.get<const Renderable>(entity);
			if (renderable.valid())
			{
				_cam->beforeRenderEntity(entity, *encoder, viewId);
				renderable.render(*encoder, viewId);
			}
		}
		_cam->afterRenderView(*encoder, viewId);

		bgfx::end(encoder);

		return ++viewId;
	}
}