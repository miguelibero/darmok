

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/window.hpp>
#include <darmok/camera.hpp>
#include <darmok/input.hpp>
#include <darmok/program.hpp>
#include <darmok/program_standard.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/render.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/physics3d.hpp>

namespace
{
	using namespace darmok;

	class JoltSampleApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);

			auto scene = addComponent<SceneAppComponent>().getScene();
			scene->addLogicUpdater<Physics3dSystem>(getAssets().getAllocator());

			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);

			auto camEntity = scene->createEntity();
			glm::vec2 winSize = getWindow().getSize();

			scene->addComponent<Transform>(camEntity)
				.setPosition({ 0, 10, -10 })
				.lookAt({ 0, 0, 0 });
			auto& cam = scene->addComponent<Camera>(camEntity)
				.setPerspective(60, winSize.x / winSize.y, 0.3, 1000);
			
			cam.addComponent<PhongLightingComponent>();
			cam.setRenderer<ForwardRenderer>();

			auto light = scene->createEntity();
			scene->addComponent<Transform>(light)
				.setPosition({ 1, 1, -2 });
			scene->addComponent<PointLight>(light);

			auto floorEntity = scene->createEntity();
			scene->addComponent<RigidBody3d>(floorEntity, Plane::standard(), RigidBody3d::MotionType::Static);

			auto greenTex = getAssets().getColorTextureLoader()(Colors::green());
			auto greenMat = std::make_shared<Material>(prog);
			greenMat->setTexture(MaterialTextureType::Diffuse, greenTex);

			MeshCreator meshCreator(prog->getVertexLayout());
			auto cubeMesh = meshCreator.createCuboid();
			auto cubeEntity = scene->createEntity();
			scene->addComponent<Renderable>(cubeEntity, cubeMesh, greenMat);
			scene->addComponent<Transform>(cubeEntity)
				.setPosition({ 1.5F, 10.F, 0.F });
			scene->addComponent<RigidBody3d>(cubeEntity, Cuboid::standard(), 1.F);

			auto redTex = getAssets().getColorTextureLoader()(Colors::red());
			auto redMat = std::make_shared<Material>(prog);
			redMat->setTexture(MaterialTextureType::Diffuse, redTex);

			auto sphereMesh = meshCreator.createSphere();
			auto sphereEntity = scene->createEntity();
			scene->addComponent<Renderable>(sphereEntity, sphereMesh, redMat);
			scene->addComponent<RigidBody3d>(sphereEntity, Sphere::standard(), RigidBody3d::MotionType::Kinematic);
			auto& trans = scene->addComponent<Transform>(sphereEntity);

			auto speed = 0.1F;

			auto moveAtSpeed = [&trans, speed](const glm::vec3& d) {
				auto pos = trans.getPosition();
				pos += d * speed;
				trans.setPosition(pos);
			};
			auto moveRight = [moveAtSpeed]() { moveAtSpeed({ 1, 0, 0 });};
			auto moveLeft = [moveAtSpeed]() { moveAtSpeed({ -1, 0, 0 }); };
			auto moveForward = [moveAtSpeed]() { moveAtSpeed({ 0, 0, 1 }); };
			auto moveBack = [moveAtSpeed]() { moveAtSpeed({ 0, 0, -1 }); };

			auto moveMouse = [this, &cam, &trans]()
			{
					auto pos = getInput().getMouse().getPosition();
					pos = getWindow().windowToScreenPoint(pos);
					auto ray = cam.screenPointToRay(glm::vec3(pos, 0.F));
					auto dist = ray.intersect(Plane::standard());
					if (dist)
					{
						trans.setPosition(ray * dist.value());
					}
			};

			getInput().addBindings("movement", {
				{ KeyboardBindingKey{ KeyboardKey::Right}, false, moveRight},
				{ KeyboardBindingKey{ KeyboardKey::KeyD}, false, moveRight},
				{ KeyboardBindingKey{ KeyboardKey::Left}, false, moveLeft},
				{ KeyboardBindingKey{ KeyboardKey::KeyA}, false, moveLeft},
				{ KeyboardBindingKey{ KeyboardKey::Up}, false, moveForward},
				{ KeyboardBindingKey{ KeyboardKey::KeyW}, false, moveForward},
				{ KeyboardBindingKey{ KeyboardKey::Down}, false, moveBack},
				{ KeyboardBindingKey{ KeyboardKey::KeyS}, false, moveBack},
				{ MouseBindingKey{ MouseButton::Left}, false, moveMouse},
			});
		}
	};
}

DARMOK_RUN_APP(JoltSampleApp);
