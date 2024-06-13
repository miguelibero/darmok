

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

			_scene = addComponent<SceneAppComponent>().getScene();
			_scene->addLogicUpdater<Physics3dSystem>(getAssets().getAllocator());

			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);

			auto camEntity = _scene->createEntity();
			glm::vec2 winSize = getWindow().getSize();

			_scene->addComponent<Transform>(camEntity)
				.setPosition({ 0, 10, -10 })
				.lookAt({ 0, 0, 0 });
			auto& cam = _scene->addComponent<Camera>(camEntity)
				.setPerspective(60, winSize.x / winSize.y, 0.3, 1000);
			
			cam.addComponent<PhongLightingComponent>();
			cam.setRenderer<ForwardRenderer>();

			auto light = _scene->createEntity();
			_scene->addComponent<Transform>(light)
				.setPosition({ 1, 2, -2 });
			_scene->addComponent<PointLight>(light);

			_scene->addComponent<AmbientLight>(_scene->createEntity(), 0.5);

			auto floorEntity = _scene->createEntity();
			_scene->addComponent<RigidBody3d>(floorEntity, Plane::standard(), RigidBody3d::MotionType::Static);

			auto greenTex = getAssets().getColorTextureLoader()(Colors::green());
			_cubeMat = std::make_shared<Material>(prog, greenTex);
			_meshCreator = std::make_unique<MeshCreator>(prog->getVertexLayout());
			_cubeMesh = _meshCreator->createCuboid();

			for (auto x = -5.F; x < 5.F; x += 1.1F)
			{
				for (auto z = -5.F; z < 5.F; z += 1.1F)
				{
					glm::vec3 rot{ 45*x, 0.F, 45.F*z };
					createCube().setPosition({ x, 10.F, z }).setEulerAngles(rot);
				}
			}

			auto redTex = getAssets().getColorTextureLoader()(Colors::red());
			auto redMat = std::make_shared<Material>(prog);
			redMat->setTexture(MaterialTextureType::Diffuse, redTex);

			auto sphereMesh = _meshCreator->createSphere();
			auto sphereEntity = _scene->createEntity();
			_scene->addComponent<Renderable>(sphereEntity, sphereMesh, redMat);
			auto& rigidBody = _scene->addComponent<RigidBody3d>(sphereEntity, Sphere::standard(), RigidBody3d::MotionType::Kinematic);
			_scene->addComponent<Transform>(sphereEntity, glm::vec3{ 0.F, 0.5F, 0.F});

			auto speed = 0.1F;

			auto moveAtSpeed = [&rigidBody, speed](const glm::vec3& d) {
				auto pos = rigidBody.getPosition();
				pos += d * speed;
				rigidBody.setPosition(pos);
			};
			auto moveRight = [moveAtSpeed]() { moveAtSpeed({ 1, 0, 0 });};
			auto moveLeft = [moveAtSpeed]() { moveAtSpeed({ -1, 0, 0 }); };
			auto moveForward = [moveAtSpeed]() { moveAtSpeed({ 0, 0, 1 }); };
			auto moveBack = [moveAtSpeed]() { moveAtSpeed({ 0, 0, -1 }); };

			Plane movePlane;
			// ball has a radius of 0.5f;
			movePlane.origin.y = 0.5f;

			auto moveMouse = [this, &cam, &rigidBody, movePlane]()
			{
			    auto pos = getInput().getMouse().getPosition();
			    pos = getWindow().windowToScreenPoint(pos);
			    auto ray = cam.screenPointToRay(glm::vec3(pos, 0.F));
			    auto dist = ray.intersect(movePlane);
			    if (dist)
			    {
					rigidBody.setPosition(ray * dist.value());
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

	private:
		std::unique_ptr<MeshCreator> _meshCreator;
		std::shared_ptr<Scene> _scene;
		std::shared_ptr<Material> _cubeMat;
		std::shared_ptr<IMesh> _cubeMesh;

		Transform& createCube() noexcept
		{
			auto entity = _scene->createEntity();
			_scene->addComponent<RigidBody3d>(entity, Cuboid::standard(), 1.F);
			_scene->addComponent<Renderable>(entity, _cubeMesh, _cubeMat);
			return _scene->addComponent<Transform>(entity);
		}


	};
}

DARMOK_RUN_APP(JoltSampleApp);
