

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/window.hpp>
#include <darmok/camera.hpp>
#include <darmok/input.hpp>
#include <darmok/render_forward.hpp>

namespace
{
	using namespace darmok;

	class PbrScene : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<SceneAppComponent>().getScene();
			auto& lights = scene.addLogicUpdater<LightRenderUpdater>();
			auto& renderer = scene.addRenderer<ForwardRenderer>(lights);
			auto& progDef = renderer.getProgramDefinition();

			auto cam = scene.createEntity();
			glm::vec2 winSize = getWindow().getSize();

			scene.addComponent<Transform>(cam)
				.setPosition({ 0, 2, -2 })
				.setRotation({ 45, 0, 0 });

			scene.addComponent<Camera>(cam)
				.setProjection(60, winSize.x / winSize.y, 0.3, 1000);

			auto light = scene.createEntity();
			scene.addComponent<Transform>(light)
				.setPosition({ 1, 1, -2 });
			scene.addComponent<PointLight>(light);

			auto tex = getAssets().getColorTextureLoader()(Colors::red);
			auto mat = std::make_shared<Material>(progDef);
			mat->setColor(MaterialColorType::Specular, Colors::blue);

			mat->setTexture(MaterialTextureType::Diffuse, tex);
			auto sphereMesh = Mesh::createSphere(mat);
			auto sphere = scene.createEntity();
			scene.addComponent<MeshComponent>(sphere, sphereMesh);
			auto& trans = scene.addComponent<Transform>(sphere);

			auto cubeMesh = Mesh::createCube(mat);
			auto cube = scene.createEntity();
			scene.addComponent<MeshComponent>(cube, cubeMesh);
			scene.addComponent<Transform>(cube)
				.setPosition({ 1.5F, 0, 0 });

			auto speed = 0.01F;

			auto move = [&trans, speed](const glm::vec3& d) {
				auto pos = trans.getPosition();
				pos += d * speed;
				trans.setPosition(pos);
			};
			auto moveRight = [move]() { move({ 1, 0, 0 });};
			auto moveLeft = [move]() { move({ -1, 0, 0 }); };
			auto moveForward = [move]() { move({ 0, 0, 1 }); };
			auto moveBack = [move]() { move({ 0, 0, -1 }); };

			getInput().addBindings("movement", {
				{ KeyboardBindingKey{ KeyboardKey::Right}, false, moveRight},
				{ KeyboardBindingKey{ KeyboardKey::KeyD}, false, moveRight},
				{ KeyboardBindingKey{ KeyboardKey::Left}, false, moveLeft},
				{ KeyboardBindingKey{ KeyboardKey::KeyA}, false, moveLeft},
				{ KeyboardBindingKey{ KeyboardKey::Up}, false, moveForward},
				{ KeyboardBindingKey{ KeyboardKey::KeyW}, false, moveForward},
				{ KeyboardBindingKey{ KeyboardKey::Down}, false, moveBack},
				{ KeyboardBindingKey{ KeyboardKey::KeyS}, false, moveBack},
			});
		}
	};

}

DARMOK_MAIN(PbrScene);
