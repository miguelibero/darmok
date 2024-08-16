

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/model_assimp.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/freelook.hpp>
#include <darmok/render_deferred.hpp>
#include <darmok/render_forward.hpp>

namespace
{
	using namespace darmok;

	class DeferredSampleApp : public App
	{
	public:
		void init() override
		{
			setResetFlag(BGFX_RESET_SRGB_BACKBUFFER);
			setResetFlag(BGFX_RESET_MSAA_X4);
			setResetFlag(BGFX_RESET_MAXANISOTROPY);

			App::init();

			auto scene = addComponent<SceneAppComponent>().getScene();
			getAssets().getAssimpModelLoader().setConfig({
				.standardProgram = StandardProgramType::Forward
			});
			auto model = getAssets().getModelLoader()("Sponza.dml");

			auto camEntity = scene->createEntity();
			auto& cam = scene->addComponent<Camera>(camEntity);
			cam.setWindowPerspective(60, 0.3, 1000);

			scene->addComponent<Transform>(camEntity)
				.setPosition(glm::vec3(0, 1, 0))
				.lookAt(glm::vec3(-7, 2, 0));

			auto lightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(lightEntity, 0.5);

			for (auto& lightConfig : _pointLights)
			{
				auto entity = scene->createEntity();
				scene->addComponent<PointLight>(entity, lightConfig.intensity, lightConfig.color);
				scene->addComponent<Transform>(entity, lightConfig.position);
			}

			// cam.addRenderer<DeferredRenderer>();
			cam.addRenderer<ForwardRenderer>()
				.addComponent<LightingRenderComponent>();

			scene->addSceneComponent<FreelookController>(cam);

			ModelSceneConfigurer configurer(*scene, getAssets());
			configurer.setTextureFlags(BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC);
			configurer(*model);
		}

		struct PointLightConfig final
		{
			glm::vec3 position;
			float intensity = 1.F;
			Color3 color = Colors::white3();
		};

		static const std::vector<PointLightConfig> _pointLights;
	};

	const std::vector<DeferredSampleApp::PointLightConfig> DeferredSampleApp::_pointLights = {
		{{ -5.0f, 0.3f, 0.0f }, 50.F, Colors::blue3()},
		{{ 0.0f, 0.3f, 0.0f }, 50.F},
		{{ 5.0f, 0.3f, 0.0f }, 50.F, Colors::red3()},
	};
}

DARMOK_RUN_APP(DeferredSampleApp);
