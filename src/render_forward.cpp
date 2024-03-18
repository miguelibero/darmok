#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/camera.hpp>

namespace darmok
{
	ForwardRenderer::ForwardRenderer(OptionalRef<LightRenderUpdater> lights)
		: _lights(lights)
	{
	}

	bgfx::ViewId ForwardRenderer::render(const Camera& cam, bgfx::Encoder& encoder, bgfx::ViewId viewId)
	{
		auto& registry = _scene->getRegistry();
		auto meshes = cam.createEntityView<MeshComponent>(registry);
		auto rendered = false;
		for (auto entity : meshes)
		{
			auto& comp = registry.get<const MeshComponent>(entity);
			auto& meshes = comp.getMeshes();
			if (meshes.empty())
			{
				continue;
			}
			rendered = true;
			Transform::bgfxConfig(entity, encoder, registry);
			for (auto& mesh : meshes)
			{
				auto mat = mesh->getMaterial();
				if (_lights && mat)
				{
					_lights->bgfxConfig(cam, *mat, encoder);
				}
				mesh->render(encoder, viewId);
			}
		}
		if (!rendered)
		{
			return viewId;
		}

		return ++viewId;
	}
}