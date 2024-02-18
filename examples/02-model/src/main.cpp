

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/mesh.hpp>

namespace
{
	class ModelScene : public darmok::App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<darmok::SceneAppComponent>().getScene();
			auto& renderer = scene.addRenderer<darmok::MeshRenderer>();

			auto model = darmok::AssetContext::get().getModelLoader()("assets/brick.fbx");
			
			auto& cam2 = model->getRootNode().getChild("Camera").value();
			auto& cam = model->getCameras().get("Camera").value();
			auto camEntity = scene.createEntity();
			scene.addComponent<darmok::Camera>(camEntity, cam.getProjection());
			auto& t = scene.addComponent<darmok::Transform>(camEntity, cam2.getTransform());
			
			auto& cube = model->getRootNode().getChild("Cube").value();
			darmok::addModelNodeToScene(scene, cube);

			auto material = model->getMaterials()[0].load();
			material->setColor(darmok::MaterialColorType::Diffuse, darmok::Colors::red);
			auto texture = darmok::AssetContext::get().getTextureLoader()("assets/brick.png");
			material->addTexture(darmok::MaterialTexture(texture));
		}

		void updateLogic(float dt) override
		{
		}

		void beforeRender(bgfx::ViewId viewId) override
		{
			bgfx::setViewClear(viewId
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
			);
		}
	};

}

DARMOK_IMPLEMENT_MAIN(ModelScene);
