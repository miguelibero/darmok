

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
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/render.hpp>
#include <darmok/render_shadow.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/freelook.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace
{
	using namespace darmok;

	class CircleUpdater final : public ISceneComponent
	{
	public:
		CircleUpdater(Transform& trans, float speed = 1.f)
			: _trans(trans)
			, _speed(speed)
		{
		}

		void update(float dt) override
		{
			auto pos = glm::rotateZ(_trans.getPosition(), dt * _speed);
			_trans.setPosition(pos);
		}

	private:
		Transform& _trans;
		float _speed;
	};

	class PbrSampleApp : public App
	{
	public:
		void init() override
		{
			App::init();

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto prog = std::make_shared<Program>(StandardProgramType::ForwardBasic);
			auto& layout = prog->getVertexLayout();

			auto camEntity = scene.createEntity();

			auto& camTrans = scene.addComponent<Transform>(camEntity)
				.setPosition({ 0, 2, -2 })
				.lookAt({ 0, 0, 0 });
			auto& cam = scene.addComponent<Camera>(camEntity)
				.setWindowPerspective(60, 0.3, 1000);
			
			cam.addRenderer<ShadowRenderer>();
			cam.addRenderer<ForwardRenderer>()
				.addComponent<LightingRenderComponent>();

			_freelook = scene.addSceneComponent<FreelookController>(cam);

			auto unlitProg = std::make_shared<Program>(StandardProgramType::Unlit);
			auto debugMat = std::make_shared<Material>(unlitProg, Colors::magenta());

			auto lightRootEntity = scene.createEntity();
			auto& lightRootTrans = scene.addComponent<Transform>(lightRootEntity, glm::vec3{ 0, 1.5, -1 });
			auto lightEntity = scene.createEntity();
			auto& lightTrans = scene.addComponent<Transform>(lightEntity, lightRootTrans, glm::vec3{ 0, 1, 0 });
			std::shared_ptr<IMesh> lightMesh = MeshData(Sphere(0.01)).createMesh(unlitProg->getVertexLayout());
			scene.addComponent<Renderable>(lightEntity, lightMesh, debugMat);
			scene.addSceneComponent<CircleUpdater>(lightTrans);
			scene.addComponent<PointLight>(lightEntity, 1);

			{
				auto lightEntity2 = scene.createEntity();
				scene.addComponent<Transform>(lightEntity2, lightRootTrans, glm::vec3{ 0, 1, -4 });
				scene.addComponent<PointLight>(lightEntity2, 1);
			}

			scene.addComponent<AmbientLight>(lightEntity, 0.2);

			auto greenMat = std::make_shared<Material>(prog, Colors::green());

			auto goldMat = std::make_shared<Material>(prog);
			goldMat->setMetallicFactor(0.5F);
			goldMat->setBaseColor(Colors::denormalize({ 0.944F, 0.776F, 0.373F, 1.F }));

			Cube cubeShape;
			cubeShape.origin.y += 0.75F;
			auto cubeMesh = MeshData(cubeShape).createMesh(layout);
			auto cube = scene.createEntity();
			scene.addComponent<Renderable>(cube, std::move(cubeMesh), greenMat);
			scene.addComponent<Transform>(cube, glm::vec3{ 1.F, 0, 0 });

			Sphere shereShape;
			shereShape.origin.y += 0.75F;
			MeshData shereMeshData(shereShape);
			auto sphereMesh = shereMeshData.createMesh(layout);
			auto sphere = scene.createEntity();
			scene.addComponent<Renderable>(sphere, std::move(sphereMesh), goldMat);
			_trans = scene.addComponent<Transform>(sphere, glm::vec3{ -1.F, 0, 0 });

			auto floorEntity = scene.createEntity();
			Cube floorShape(glm::vec3(10.F, .5F, 10.F), glm::vec3(0, -0.25, 0));
			auto floorMesh = MeshData(floorShape).createMesh(prog->getVertexLayout());
			scene.addComponent<Renderable>(floorEntity, std::move(floorMesh), prog, Colors::red());
		}

		void update(float deltaTime) override
		{
			if (_freelook && _freelook->isEnabled())
			{
				return;
			}
			glm::vec3 dir(0);
			dir.x = getInput().getAxis(_moveRight, _moveLeft);
			dir.z = getInput().getAxis(_moveForward, _moveBackward);

			auto pos = _trans->getPosition();
			_trans->setPosition(pos + (dir * deltaTime));
		}

		OptionalRef<FreelookController> _freelook;
		OptionalRef<Transform> _trans;

		const InputDirs _moveForward = {
			KeyboardInputEvent{ KeyboardKey::Up },
			KeyboardInputEvent{ KeyboardKey::KeyW },
			GamepadInputDir{ GamepadStick::Left, InputDirType::Up }
		};
		const InputDirs _moveBackward = {
			KeyboardInputEvent{ KeyboardKey::Down },
			KeyboardInputEvent{ KeyboardKey::KeyS },
			GamepadInputDir{ GamepadStick::Left, InputDirType::Down }
		};
		const InputDirs _moveLeft = {
			KeyboardInputEvent{ KeyboardKey::Left },
			KeyboardInputEvent{ KeyboardKey::KeyA },
			GamepadInputDir{ GamepadStick::Left, InputDirType::Left }
		};
		const InputDirs _moveRight = {
			KeyboardInputEvent{ KeyboardKey::Right },
			KeyboardInputEvent{ KeyboardKey::KeyD },
			GamepadInputDir{ GamepadStick::Left, InputDirType::Right }
		};
	};

}

DARMOK_RUN_APP(PbrSampleApp);
