

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/render_forward.hpp>

namespace
{
	class PbrScene : public darmok::App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<darmok::SceneAppComponent>().getScene();
			auto& lights = scene.addLogicUpdater<darmok::LightRenderUpdater>();
			auto& renderer = scene.addRenderer<darmok::ForwardRenderer>(lights);
			auto& progDef = renderer.getProgramDefinition();

			auto tex = getAssets().getColorTextureLoader()(darmok::Colors::red);
			auto mat = std::make_shared<darmok::Material>(progDef);
			mat->setTexture(darmok::MaterialTextureType::Diffuse, tex);
			auto sphereMesh = darmok::Mesh::createSphere(mat);
			auto sphere = scene.createEntity();
			scene.addComponent<darmok::MeshComponent>(sphere, sphereMesh);
		}
	};

}

DARMOK_MAIN(PbrScene);
