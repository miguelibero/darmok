

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

	template<typename T>
	using unexpected = tl::unexpected<T>;

	class RotateUpdater final : public ISceneComponent
	{
	public:
		RotateUpdater(Transform& trans, float speed = 100.f)
			: _trans{ trans }
			, _speed{ speed }
		{
		}

		expected<void, std::string> update(float dt) noexcept override
		{
			_trans.rotate(glm::radians(glm::vec3{ 0, 0, dt * _speed }));
			return {};
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

		expected<void, std::string> init() noexcept override
		{
			auto scene = _app.tryAddComponent<SceneAppComponent>()->getScene();

			auto defResult = _app.getAssets().getSceneDefinitionLoader()("human.dsc");
			if(!defResult)
			{
				return unexpected{ std::string{ "error loading scene definition: "} + defResult.error() };
			}

			auto sceneDef = defResult.value();
			auto result = SceneLoader{}(*sceneDef, *scene);
			if (!result)
			{
				return unexpected{ std::string{ "error loading scene: "} + result.error() };
			}

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

			return {};
		}
	private:
		App& _app;
	};
}

DARMOK_RUN_APP(AssimpSampleAppDelegate);
