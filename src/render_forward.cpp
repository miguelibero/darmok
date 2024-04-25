#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/light.hpp>

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
		auto meshComponents = _cam->createEntityView<MeshComponent>(registry);
		auto rendered = false;
		for (auto entity : meshComponents)
		{
			auto& comp = registry.get<const MeshComponent>(entity);
			auto& meshes = comp.getMeshes();
			if (meshes.empty())
			{
				continue;
			}
			rendered = true;
			_cam->beforeRenderEntity(entity, encoder, viewId);

			auto trans = registry.try_get<Transform>(entity);
			if (trans != nullptr)
			{
				trans->beforeRender(encoder, viewId);
			}
			for (auto& mesh : meshes)
			{
				auto mat = mesh->getMaterial();
				if (mat == nullptr)
				{
					continue;
				}

				_cam->beforeRenderMesh(*mesh, encoder, viewId);
				uint64_t state = mat->beforeRender(encoder, viewId);
				mesh->render(encoder, viewId);

				encoder.setState(state);
				encoder.submit(viewId, _program->getHandle());
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