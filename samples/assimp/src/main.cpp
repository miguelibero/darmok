

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/scene_assimp.hpp>
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
			: _trans{ trans }
			, _speed{ speed }
		{
		}

		void update(float dt) override
		{
			_trans.rotate(glm::radians(glm::vec3{ 0, 0, dt * _speed }));
		}

	private:
		Transform& _trans;
		float _speed;
	};

	class AssimpSampleAppDelegate final : public IAppDelegate
	{
	public:
		AssimpSampleAppDelegate(App& app)
			: _app{ app }
		{
		}

		void init() override
		{
			auto scene = _app.addComponent<SceneAppComponent>().getScene();
			_app.getAssets().getSceneLoader()(*scene, "human.pb");

			auto ambientLightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(ambientLightEntity, 0.5);
			
			for (auto entity : scene->getComponents<Transform>())
			{
				auto& trans = *scene->getComponent<Transform>(entity);
				if (trans.getName() == "human")
				{
					scene->addSceneComponent<RotateUpdater>(trans, 100.f);
				}
			}
			for (auto entity : scene->getComponents<Camera>())
			{
				auto& cam = *scene->getComponent<Camera>(entity);
				cam.addComponent<ForwardRenderer>();
				cam.addComponent<LightingRenderComponent>();
			}
		}
	private:
		App& _app;
	};
}

DARMOK_RUN_APP(AssimpSampleAppDelegate);
