

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

	class AssimpSampleAppDelegate final : public IAppDelegate
	{
	public:
		AssimpSampleAppDelegate(App& app)
			: _app(app)
		{
		}

		void init() override
		{
			auto scene = _app.addComponent<SceneAppComponent>().getScene();
			_app.getAssets().getAssimpModelLoader().setConfig({
				.standardProgram = StandardProgramType::ForwardBasic
			});
			auto model = _app.getAssets().getModelLoader()("human.dml");

			auto ambientLightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(ambientLightEntity, 0.5);

			ModelSceneConfigurer configurer(*scene, _app.getAssets());
			configurer(*model, [scene](const auto& node, Entity entity) {
				if (node.name == "human")
				{
					auto& trans = scene->getOrAddComponent<Transform>(entity);
					scene->addSceneComponent<RotateUpdater>(trans, 100.f);
				}
				auto cam = scene->getComponentInChildren<Camera>(entity);
				if (cam)
				{
					cam->addComponent<ForwardRenderer>();
					cam->addComponent<LightingRenderComponent>();
				}
			});
		}
	private:
		App& _app;
	};
}

DARMOK_RUN_APP(AssimpSampleAppDelegate);
