

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/assimp.hpp>
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

			auto assimpScene = getAssets().getAssimpLoader()("human.fbx");
			assimpScene->addToScene(scene, prog->getVertexLayout(), [&scene, prog](const AssimpNode& node, Entity entity) {
				auto& registry = scene.getRegistry();
				if (node.getName() == "human")
				{
					auto& trans = registry.get_or_emplace<Transform>(entity);
					scene.addLogicUpdater<RotateUpdater>(trans, 100.f);
				}
				if (node.getCamera().hasValue())
				{
					auto& cam = registry.get_or_emplace<Camera>(entity);
					cam.addComponent<PhongLightingComponent>();
					cam.setRenderer<ForwardRenderer>(prog);
				}
			});
		}
	};
}

DARMOK_MAIN(ModelApp);
