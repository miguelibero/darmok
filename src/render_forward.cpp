#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/light.hpp>

namespace darmok
{

	ForwardRenderer::ForwardRenderer(const std::shared_ptr<Program>& program, const OptionalRef<ILightingComponent>& lighting) noexcept
		: _program(program)
		, _lighting(lighting)
	{
	}

	void ForwardRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
	{
		_cam = cam;
		_scene = scene;
		_app = app;
	}

	bgfx::ViewId ForwardRenderer::render(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
		if (!_cam || _program == nullptr)
		{
			return viewId;
		}

		auto& registry = _scene->getRegistry();
		auto meshes = _cam->createEntityView<MeshComponent>(registry);
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
			for (auto& mesh : meshes)
			{
				auto mat = mesh->getMaterial();
				if (mat == nullptr)
				{
					continue;
				}

				Transform::bgfxConfig(entity, encoder, registry);

				if (_lighting)
				{
					_lighting->bgfxConfig(encoder, viewId);
				}

				mat->bgfxConfig(encoder);
				mesh->bgfxConfig(encoder, viewId);

				uint64_t state = BGFX_STATE_WRITE_RGB
					| BGFX_STATE_WRITE_A
					| BGFX_STATE_WRITE_Z
					| BGFX_STATE_DEPTH_TEST_LEQUAL // TODO: should be less?
					| BGFX_STATE_CULL_CCW
					| BGFX_STATE_MSAA
					| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
					;
				
				if (mat->getPrimitiveType() == MaterialPrimitiveType::Line)
				{
					state |= BGFX_STATE_PT_LINES;
				}

				encoder.setState(state);
				encoder.submit(viewId, _program->getHandle());
			}
		}
		if (!rendered)
		{
			return viewId;
		}

		return ++viewId;
	}
}