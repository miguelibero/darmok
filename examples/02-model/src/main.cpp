

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
			
			// addCubeManually(scene, texture);
			addCubeFromModel(scene, texture);
		}

		void addCubeFromModel(darmok::Scene& scene, const std::shared_ptr<darmok::Texture>& texture)
		{
			auto model = darmok::AssetContext::get().getModelLoader()("assets/brick.fbx");
			darmok::addModelToScene(scene, *model);

			auto material = model->getMaterials()[0].load();
			material->setColor(darmok::MaterialColorType::Diffuse, darmok::Colors::red);
			material->setTexture(darmok::MaterialTextureType::Diffuse, texture);
		}

		void addCubeManually(darmok::Scene& scene, const std::shared_ptr<darmok::Texture>& texture)
		{

			auto camEntity = scene.createEntity();
			scene.addComponent<darmok::Camera>(camEntity).setProjection(glm::pi<float>(), 4.f / 3.f);
			auto& camTrans = scene.addComponent<darmok::Transform>(camEntity);
			camTrans.setPosition(glm::vec3(0.f, 5.f, 5.f));
			camTrans.setRotation(glm::vec3(45, 0, 0));

			auto material = darmok::Material::createStandard(darmok::StandardMaterialType::Basic);
			material->setTexture(darmok::MaterialTextureType::Diffuse, texture);
			auto cubeMesh = darmok::Mesh::createCube(material);

			auto cubeEntity = scene.createEntity();
			scene.addComponent<darmok::MeshComponent>(cubeEntity, cubeMesh);
			auto& cubeTrans = scene.addComponent<darmok::Transform>(cubeEntity);
			cubeTrans.setRotation(glm::vec3(0, 0, -45));
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
