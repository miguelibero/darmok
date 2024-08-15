

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
			App::init();

			auto scene = addComponent<SceneAppComponent>().getScene();
			getAssets().getAssimpModelLoader().setConfig({
				.standardProgram = StandardProgramType::Forward
			});
			auto model = getAssets().getModelLoader()("Sponza.dml");

			auto ambientLightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(ambientLightEntity, 0.5);

			auto camEntity = scene->createEntity();
			auto& cam = scene->addComponent<Camera>(camEntity);
			cam.setWindowPerspective(60, 0.3, 1000);

			scene->addComponent<Transform>(camEntity)
				.setPosition(glm::vec3(0, 2, 0))
				.lookAt(glm::vec3(2, 2, 0));



			// cam.addRenderer<DeferredRenderer>();

			cam.addRenderer<ForwardRenderer>();


			scene->addSceneComponent<FreelookController>(cam);

			ModelSceneConfigurer configurer(*scene, getAssets());
			configurer(*model);
		}
	};
}

DARMOK_RUN_APP(DeferredSampleApp);
