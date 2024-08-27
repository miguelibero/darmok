

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
#include <darmok/render_forward.hpp>

namespace
{
	using namespace darmok;

	class RotateUpdater final : public ISceneComponent
	{
	public:
		RotateUpdater(Transform& trans, float speed = 100.f)
			: _trans(trans)
			, _speed(speed)
		{
		}

		void update(float dt) override
		{
			_trans.rotate(glm::vec3(0, 0, dt * _speed));
		}

	private:
		Transform& _trans;
		float _speed;
	};

	class AssimpSampleApp : public App
	{
	public:
		void init() override
		{
			App::init();

			auto scene = addComponent<SceneAppComponent>().getScene();
			getAssets().getAssimpModelLoader().setConfig({
				.standardProgram = StandardProgramType::ForwardBasic
			});
			auto model = getAssets().getModelLoader()("human.dml");

			auto ambientLightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(ambientLightEntity, 0.5);

			ModelSceneConfigurer configurer(*scene, getAssets());
			configurer(*model, [scene](const auto& node, Entity entity) {
				auto& registry = scene->getRegistry();
				if (node.name == "human")
				{
					auto& trans = registry.get_or_emplace<Transform>(entity);
					scene->addSceneComponent<RotateUpdater>(trans, 100.f);
				}
				auto cam = scene->getComponentInChildren<Camera>(entity);
				if (cam)
				{
					cam->addRenderer<ForwardRenderer>()
						.addComponent<LightingRenderComponent>();
				}
			});
		}
	};
}

DARMOK_RUN_APP(AssimpSampleApp);
