

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
#include <darmok/character.hpp>

namespace
{
	using namespace darmok;
	using namespace darmok::physics3d;

	class JoltSampleApp : public App, public ICollisionListener
	{
	public:
		void init() override
		{
			App::init();

			_scene = addComponent<SceneAppComponent>().getScene();
			_scene->addComponent<PhysicsSystem>(getAssets().getAllocator());

			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);

			// camera
			{
				auto camEntity = _scene->createEntity();
				glm::vec2 winSize = getWindow().getSize();

				_scene->addComponent<Transform>(camEntity)
					.setPosition({ 0, 5, -10 })
					.lookAt({ 0, 0, 0 });
				_cam = _scene->addComponent<Camera>(camEntity)
					.setPerspective(60, winSize.x / winSize.y, 0.3, 1000);

				_cam->addComponent<PhongLightingComponent>();
				_cam->setRenderer<ForwardRenderer>();
			}

			{ // lights
				auto light = _scene->createEntity();
				_scene->addComponent<Transform>(light)
					.setPosition({ 1, 2, -2 });
				_scene->addComponent<PointLight>(light);
				_scene->addComponent<AmbientLight>(_scene->createEntity(), 0.5);
			}

			MeshCreator meshCreator(prog->getVertexLayout());

			{ // floor
				auto floorEntity = _scene->createEntity();
				auto floorTex = getAssets().getColorTextureLoader()(Color(172, 172, 124, 255));
				auto floorMat = std::make_shared<Material>(prog, floorTex);
				Cuboid floorShape(glm::vec3(10.F, .5F, 10.F), glm::vec3(0, -0.25, 0));
				_floorBody = _scene->addComponent<PhysicsBody>(floorEntity, floorShape, PhysicsBody::MotionType::Static);
				auto floorMesh = meshCreator.createCuboid(floorShape);
				_scene->addComponent<Renderable>(floorEntity, floorMesh, floorMat);
			}

			{ // door trigger

				auto doorEntity = _scene->createEntity();
				Cuboid doorShape(glm::vec3(1.5F, 2.F, 0.2F), glm::vec3(0, 1.F, 0));
				PhysicsBodyConfig config;
				config.trigger = true;
				config.shape = doorShape;
				config.motion = PhysicsBodyMotionType::Kinematic;
				_doorBody = _scene->addComponent<PhysicsBody>(doorEntity, config);
				auto doorMesh = meshCreator.createCuboid(doorShape);
				auto doorTex = getAssets().getColorTextureLoader()(Color(255, 200, 200, 255));
				_doorMat = std::make_shared<Material>(prog, doorTex);
				auto triggerTex = getAssets().getColorTextureLoader()(Colors::red());
				_triggerDoorMat = std::make_shared<Material>(prog, triggerTex);
				_scene->addComponent<Renderable>(doorEntity, doorMesh, _doorMat);
				_scene->addComponent<Transform>(doorEntity)
					.setPosition(glm::vec3(2.F, 0.F, 2.F))
					.setEulerAngles(glm::vec3(0, 90, 0));
			}

			{ // cubes

				_cubeMesh = meshCreator.createCuboid();
				auto greenTex = getAssets().getColorTextureLoader()(Colors::green());
				auto darkGreenTex = getAssets().getColorTextureLoader()(Color(0, 100, 0, 255));
				_cubeMat = std::make_shared<Material>(prog, darkGreenTex);
				_touchedCubeMat = std::make_shared<Material>(prog, greenTex);

				for (auto x = -5.F; x < 5.F; x += 1.1F)
				{
					for (auto z = -5.F; z < 5.F; z += 1.1F)
					{
						glm::vec3 rot{ 45 * x, 0.F, 45.F * z };
						createCube().setPosition({ x, 10.F, z }).setEulerAngles(rot);
					}
				}
			}

			{ // player
				auto redTex = getAssets().getColorTextureLoader()(Colors::red());
				auto redMat = std::make_shared<Material>(prog);
				redMat->setTexture(MaterialTextureType::Diffuse, redTex);

				Capsule playerShape(1.F, 0.5F, glm::vec3(0.F, 1.F, 0.F ));
				// Cuboid playerShape(glm::vec3(1, 2, 1), glm::vec3(0, 1, 0));
				auto playerMesh = meshCreator.createShape(playerShape);
				auto playerEntity = _scene->createEntity();
				_scene->addComponent<Renderable>(playerEntity, playerMesh, redMat);
				_characterCtrl = _scene->addComponent<CharacterController>(playerEntity, playerShape);

				CharacterConfig characterConfig{ playerShape };
				_characterBody = _scene->addComponent<PhysicsBody>(playerEntity, characterConfig);
				_characterBody->addListener(*this);
				_characterTrans = _scene->addComponent<Transform>(playerEntity);
			}
		}

		void onCollisionEnter(PhysicsBody& body1, PhysicsBody& body2, const Collision& collision) override
		{
			if (_doorBody == body2)
			{
				getRenderable(body2).setMaterial(_triggerDoorMat);
			}
			else if (_floorBody != body2)
			{
				getRenderable(body2).setMaterial(_touchedCubeMat);
			}
		}

		void onCollisionExit(PhysicsBody& body1, PhysicsBody& body2) override
		{
			if (_doorBody == body2)
			{
				getRenderable(body2).setMaterial(_doorMat);
			}
			else if (_floorBody != body2)
			{
				getRenderable(body2).setMaterial(_cubeMat);
			}
		}
	protected:

		void updateLogic(float dt) override
		{
			auto& mouse = getInput().getMouse();
			if (mouse.getButton(MouseButton::Left))
			{
				glm::vec2 pos = mouse.getPosition();
				pos = getWindow().windowToScreenPoint(pos);
				auto ray = _cam->screenPointToRay(glm::vec3(pos, 0.F));
				auto dist = ray.intersect(_playerMovePlane);
				if (dist)
				{
					_characterCtrl->setPosition(ray * dist.value());
				}
				return;
			}

			auto& kb = getInput().getKeyboard();
			glm::vec3 speed(0);
			if (kb.getKey(KeyboardKey::Right) || kb.getKey(KeyboardKey::KeyD))
			{
				speed = { 1, 0, 0 };
			}
			else if (kb.getKey(KeyboardKey::Left) || kb.getKey(KeyboardKey::KeyA))
			{
				speed = { -1, 0, 0 };
			}
			else if (kb.getKey(KeyboardKey::Down) || kb.getKey(KeyboardKey::KeyS))
			{
				speed = { 0, 0, -1 };
			}
			else if (kb.getKey(KeyboardKey::Up) || kb.getKey(KeyboardKey::KeyW))
			{
				speed = { 0, 0, 1 };
			}
			_characterCtrl->setLinearVelocity(speed * 10.F);
		}

	private:
		Plane _playerMovePlane;
		OptionalRef<Camera> _cam;
		OptionalRef<CharacterController> _characterCtrl;
		OptionalRef<PhysicsBody> _characterBody;
		OptionalRef<Transform> _characterTrans;
		OptionalRef<PhysicsBody> _floorBody;
		OptionalRef<PhysicsBody> _doorBody;
		std::shared_ptr<Scene> _scene;
		std::shared_ptr<Material> _cubeMat;
		std::shared_ptr<Material> _touchedCubeMat;
		std::shared_ptr<Material> _doorMat;
		std::shared_ptr<Material> _triggerDoorMat;
		Cuboid _cubeShape;
		std::shared_ptr<IMesh> _cubeMesh;

		Renderable& getRenderable(PhysicsBody& rigidBody)
		{
			auto entity = _scene->getEntity(rigidBody);
			return _scene->getComponent<Renderable>(entity).value();
		}

		Transform& createCube() noexcept
		{
			auto entity = _scene->createEntity();
			_scene->addComponent<PhysicsBody>(entity, _cubeShape);
			_scene->addComponent<Renderable>(entity, _cubeMesh, _cubeMat);
			return _scene->addComponent<Transform>(entity);
		}
	};
}

DARMOK_RUN_APP(JoltSampleApp);
