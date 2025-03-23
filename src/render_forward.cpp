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
#include <darmok/scene_filter.hpp>
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
		_materials = app.getOrAddComponent<MaterialAppComponent>();
	}

	bgfx::ViewId ForwardRenderer::renderReset(bgfx::ViewId viewId) noexcept
	{
		_viewId.reset();
		if (!_cam)
		{
			return viewId;
		}
		_cam->configureView(viewId, "Forward");
		_viewId = viewId;
		return ++viewId;
	}

	void ForwardRenderer::shutdown() noexcept
	{
		_cam.reset();
		_scene.reset();
		_app.reset();
	}

	void ForwardRenderer::render() noexcept
	{
		if (!_scene || !_viewId || !_cam || !_cam->isEnabled())
		{
			return;
		}
		auto viewId = _viewId.value();
		auto& encoder = *bgfx::begin();
		_cam->beforeRenderView(viewId, encoder);
		auto entities = _cam->getEntities<Renderable>();
		for (auto entity : entities)
		{
			auto renderable = _scene->getComponent<const Renderable>(entity);
			if (!renderable->valid())
			{
				continue;
			}
			if (_cam->shouldEntityBeCulled(entity))
			{
				continue;
			}
			_cam->beforeRenderEntity(entity, viewId, encoder);
			if (!renderable->render(encoder))
			{
				continue;
			}
			_materials->renderSubmit(viewId, encoder, *renderable->getMaterial());
		}
		bgfx::end(&encoder);
	}

	void ForwardRenderer::bindMeta() noexcept
	{
		ReflectionSerializeUtils::metaSerialize<ForwardRenderer>();
		CameraReflectionUtils::metaCameraComponent<ForwardRenderer>("ForwardRenderer")
			.ctor();
	}
}