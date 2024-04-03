#include <darmok/render_forward.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/camera.hpp>
#include <darmok/program_def.hpp>

namespace darmok
{
	ForwardRenderer::ForwardRenderer(OptionalRef<LightRenderUpdater> lights)
		: _lights(lights)
	{
	}

	void ForwardRenderer::init(Scene& scene, App& app) noexcept
	{
		CameraSceneRenderer::init(scene, app);

		auto prog = _lights ? StandardProgramType::ForwardPhong : StandardProgramType::Unlit;
		_progDef = ProgramDefinition::createStandard(prog);
		_program = Program::createStandard(prog);
	}

	const ProgramDefinition& ForwardRenderer::getProgramDefinition() const noexcept
	{
		return _progDef;
	}

	bgfx::ViewId ForwardRenderer::render(const Camera& cam, bgfx::Encoder& encoder, bgfx::ViewId viewId)
	{
		if (_program == nullptr)
		{
			return viewId;
		}

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
			for (auto& mesh : meshes)
			{
				auto mat = mesh->getMaterial();
				if (mat == nullptr || mat->getProgramDefinition() != _progDef)
				{
					continue;
				}

				Transform::bgfxConfig(entity, encoder, registry);

				if (_lights)
				{
					_lights->bgfxConfig(cam, _progDef, encoder);
				}

				uint64_t state = BGFX_STATE_WRITE_RGB
					| BGFX_STATE_WRITE_A
					| BGFX_STATE_WRITE_Z
					| BGFX_STATE_DEPTH_TEST_LEQUAL // TODO: should be less?
					| BGFX_STATE_CULL_CCW
					| BGFX_STATE_MSAA
					| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
					;
				
				mat->bgfxConfig(encoder);
				if (mat->getPrimitiveType() == MaterialPrimitiveType::Line)
				{
					state |= BGFX_STATE_PT_LINES;
				}

				mesh->bgfxConfig(encoder, viewId);
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