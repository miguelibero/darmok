

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/mesh.hpp>

#include <glm/ext/matrix_clip_space.hpp>

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

			auto texture = darmok::AssetContext::get().getTextureLoader()("assets/brick.png");
			
			auto model = darmok::AssetContext::get().getModelLoader()("assets/brick.fbx");
			darmok::addModelToScene(scene, *model);

			auto material = model->getMaterials()[0].load();
			material->setColor(darmok::MaterialColorType::Diffuse, darmok::Colors::red);
			material->setTexture(darmok::MaterialTextureType::Diffuse, texture);
		}

		void beforeRender(bgfx::ViewId viewId) override
		{
			bgfx::setViewClear(viewId
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
			);
		}
	};

}

DARMOK_IMPLEMENT_MAIN(ModelScene);
