

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
#include <darmok/program.hpp>
#include <darmok/render_forward.hpp>

namespace
{
	using namespace darmok;

	class PbrApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto& scene = addComponent<SceneAppComponent>().getScene();
			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);
			auto& layout = prog->getVertexLayout();
			auto& registry = scene.getRegistry();

			auto camEntity = registry.create();
			glm::vec2 winSize = getWindow().getSize();

			registry.emplace<Transform>(camEntity)
				.setPosition({ 0, 2, -2 })
				.setRotation({ 45, 0, 0 });
			auto& cam = registry.emplace<Camera>(camEntity)
				.setProjection(60, winSize.x / winSize.y, 0.3, 1000);
			
			auto& lighting = cam.addComponent<PhongLightingComponent>();

			cam.setRenderer<ForwardRenderer>(prog, lighting);

			auto light = registry.create();
			registry.emplace<Transform>(light)
				.setPosition({ 1, 1, -2 });
			registry.emplace<PointLight>(light);

			auto greenTex = getAssets().getColorTextureLoader()(Colors::green);
			auto greenMat = std::make_shared<Material>();
			greenMat->setTexture(MaterialTextureType::Diffuse, greenTex);

			auto cubeMesh = Mesh::createCube(layout);
			cubeMesh->setMaterial(greenMat);
			auto cube = registry.create();
			registry.emplace<MeshComponent>(cube, cubeMesh);
			registry.emplace<Transform>(cube)
				.setPosition({ 1.5F, 0, 0 });

			auto redTex = getAssets().getColorTextureLoader()(Colors::red);
			auto redMat = std::make_shared<Material>();
			redMat->setTexture(MaterialTextureType::Diffuse, redTex);

			auto sphereMesh = Mesh::createSphere(layout);
			sphereMesh->setMaterial(redMat);
			auto sphere = registry.create();
			registry.emplace<MeshComponent>(sphere, sphereMesh);
			auto& trans = registry.emplace<Transform>(sphere);

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

DARMOK_MAIN(PbrApp);
