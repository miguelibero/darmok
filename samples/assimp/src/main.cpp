

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/render_forward.hpp>

namespace
{
	using namespace darmok;

	class RotateUpdater final : public ISceneLogicUpdater
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

	class ModelApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);

			auto model = getAssets().getModelLoader()("human.fbx");

			ModelSceneConfigurer configurer(scene.getRegistry(), prog->getVertexLayout(), getAssets());
			configurer.run(model, [&scene, prog](const auto& node, Entity entity) {
				auto& registry = scene.getRegistry();
				if (node->getName() == "human")
				{
					auto& trans = registry.get_or_emplace<Transform>(entity);
					scene.addLogicUpdater<RotateUpdater>(trans, 100.f);
				}
				auto cam = scene.getComponentInChildren<Camera>(entity);
				if (cam)
				{
					cam->addComponent<PhongLightingComponent>();
					cam->setRenderer<ForwardRenderer>(prog);
				}
			});
		}
	};
}

DARMOK_RUN_APP(ModelApp);
