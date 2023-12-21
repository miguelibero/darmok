

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
			scene.addRenderer<darmok::MeshRenderer>();

			auto camEntity = scene.createEntity();
			scene.addComponent<darmok::Camera>(camEntity);

			auto model = darmok::AssetContext::get().getModelLoader()("assets/ImpChar.fbx");

			darmok::addModelToScene(scene, model);
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
