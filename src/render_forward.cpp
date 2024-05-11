#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/light.hpp>
#include <darmok/material.hpp>
#include <darmok/scene.hpp>

namespace darmok
{

	ForwardRenderer::ForwardRenderer(const std::shared_ptr<Program>& program) noexcept
		: _program(program)
	{
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

	bgfx::ViewId ForwardRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
		if (!_cam || _program == nullptr)
		{
			return viewId;
		}

		_cam->beforeRenderView(encoder, viewId);

		auto& registry = _scene->getRegistry();
		auto renderables = _cam->createEntityView<Renderable>(registry);
		auto rendered = false;
		for (auto entity : renderables)
		{
			auto& renderable = registry.get<const Renderable>(entity);
			auto mesh = renderable.getMesh();
			if (mesh == nullptr)
			{
				continue;
			}
			auto mat = renderable.getMaterial();
			if (mat == nullptr)
			{
				continue;
			}

			rendered = true;
			_cam->beforeRenderEntity(entity, encoder, viewId);

			const void* transMtx = nullptr;
			auto trans = registry.try_get<const Transform>(entity);
			if (trans != nullptr)
			{
				transMtx = glm::value_ptr(trans->getWorldMatrix());
			}
			encoder.setTransform(transMtx);

			uint64_t state = mat->beforeRender(encoder, viewId);
			mesh->render(encoder, viewId);
			encoder.setState(state);
			encoder.submit(viewId, _program->getHandle());
		}
		_cam->afterRenderView(encoder, viewId);

		if (!rendered)
		{
			return viewId;
		}


		return ++viewId;
	}
}