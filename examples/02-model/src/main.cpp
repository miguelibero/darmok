

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
			auto r = _trans.getRotation() + glm::vec3(0, dt * _speed, 0);
			_trans.setRotation(r);
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

			auto& scene = addComponent<SceneAppComponent>().getScene();
			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);

			auto model = getAssets().getModelLoader()("human.fbx");
			model->addToScene(scene, prog->getVertexLayout(), [&scene, prog](const ModelNode& node, Entity entity) {
				auto& registry = scene.getRegistry();
				if (node.getName() == "human")
				{
					auto& trans = registry.get_or_emplace<Transform>(entity);
					scene.addLogicUpdater<RotateUpdater>(trans, 100.f);
				}
				if (node.getCamera().hasValue())
				{
					auto& cam = registry.get_or_emplace<Camera>(entity);
					auto& lighting = cam.addComponent<PhongLightingComponent>();
					cam.setRenderer<ForwardRenderer>(prog, lighting);
				}
			});
		}
	};
}

DARMOK_MAIN(ModelApp);
