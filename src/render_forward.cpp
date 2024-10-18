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
	}

	bgfx::ViewId ForwardRenderer::renderReset(bgfx::ViewId viewId) noexcept
	{
		_viewId.reset();
		if (!_cam)
		{
			return viewId;
		}

		std::string name("Forward");
		if (_cam)
		{
			auto& camName = _cam->getName();
			if (!camName.empty())
			{
				name += " " + camName;
			}
		}
		bgfx::setViewName(viewId, name.c_str());
		_cam->configureView(viewId);
		_viewId = viewId;
		return ++viewId;
	}

	void ForwardRenderer::shutdown() noexcept
	{
		_cam.reset();
		_scene.reset();
		_app.reset();
	}

	void ForwardRenderer::renderEntities(bgfx::ViewId viewId, bgfx::Encoder& encoder, const EntityRuntimeView& entities, OpacityType opacity) noexcept
	{
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
			_cam->beforeRenderEntity(entity, viewId, encoder);
			if (!renderable->render(encoder))
			{
				continue;
			}
			_materials->renderSubmit(viewId, encoder, *renderable->getMaterial());
		}
	}

	void ForwardRenderer::render() noexcept
	{
		if (!_viewId || !_cam || !_cam->isEnabled())
		{
			return;
		}
		auto viewId = _viewId.value();
		auto& encoder = *bgfx::begin();
		_cam->beforeRenderView(viewId, encoder);
		auto entities = _cam->getEntities<Renderable>();
		renderEntities(viewId, encoder, entities, OpacityType::Opaque);
		renderEntities(viewId, encoder, entities, OpacityType::Mask);
		renderEntities(viewId, encoder, entities, OpacityType::Transparent);
	}
}