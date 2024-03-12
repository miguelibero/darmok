

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>

#include <glm/ext/matrix_clip_space.hpp>

namespace
{
	class RotateUpdater final : public darmok::ISceneLogicUpdater
	{
	public:
		RotateUpdater(darmok::Transform& trans, float speed = 100.f)
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
		darmok::Transform& _trans;
		float _speed;
	};

	class ModelScene : public darmok::App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<darmok::SceneAppComponent>().getScene();
			scene.addRenderer<darmok::MeshRenderer>();

			auto model = getAssets().getModelLoader()("assets/human.fbx");
			model->addToScene(scene, [&scene](const darmok::ModelNode& node, darmok::Entity entity){
				if (node.getName() == "human")
				{
					auto& trans = scene.getComponent<darmok::Transform>(entity);
					scene.addLogicUpdater<RotateUpdater>(trans, 100.f);
				}
			});
		}
	};

}

DARMOK_MAIN(ModelScene);
