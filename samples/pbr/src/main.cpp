

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/shadow.hpp>
#include <darmok/window.hpp>
#include <darmok/camera.hpp>
#include <darmok/input.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/render.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/freelook.hpp>
#include <darmok/shape.hpp>
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

	class RotateUpdater final : public ISceneComponent
	{
	public:
		RotateUpdater(Transform& trans, float speed = 50.f)
			: _trans(trans)
			, _speed(speed)
		{
		}

		void update(float dt) override
		{
			auto r = _trans.getRotation();
			r = glm::quat(glm::radians(glm::vec3(0, dt * _speed, 0))) * r;
			_trans.setRotation(r);
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
				.setWindowPerspective(60, 0.3, 20);
			
			ShadowRendererConfig shadowConfig;
			shadowConfig.mapSize = glm::vec2(512);
			shadowConfig.mapMargin = glm::vec3(0.1);
			shadowConfig.cascadeAmount = 2;
			auto& shadowRenderer = cam.addRenderer<ShadowRenderer>(shadowConfig);
			auto& forwardRender = cam.addRenderer<ForwardRenderer>();
			forwardRender.addComponent<ShadowRenderComponent>(shadowRenderer);
			forwardRender.addComponent<LightingRenderComponent>();

			_freelook = scene.addSceneComponent<FreelookController>(cam);

			auto unlitProg = std::make_shared<Program>(StandardProgramType::Unlit);
			auto debugMat = std::make_shared<Material>(unlitProg, Colors::magenta());

			auto lightRootEntity = scene.createEntity();
			auto& lightRootTrans = scene.addComponent<Transform>(lightRootEntity, glm::vec3{ 0, 1.5, -1 });
			auto lightEntity = scene.createEntity();
			auto& lightTrans = scene.addComponent<Transform>(lightEntity, lightRootTrans, glm::vec3{ 0, 1, 0 });
			scene.addSceneComponent<CircleUpdater>(lightTrans);
			scene.addComponent<PointLight>(lightEntity, 0.5);

			auto dirLightEntity = scene.createEntity();
			auto& dirLightTrans = scene.addComponent<Transform>(dirLightEntity, glm::vec3{ -1, 1, -1 })
				.lookAt(glm::vec3(0, 0, 0));
			scene.addComponent<DirectionalLight>(dirLightEntity, 0.5);
			scene.addSceneComponent<RotateUpdater>(dirLightTrans);

			auto dirLightEntity2 = scene.createEntity();
			scene.addComponent<Transform>(dirLightEntity2, glm::vec3{ 1, 1, -1 })
				.lookAt(glm::vec3(0, 0, 0));
			scene.addComponent<DirectionalLight>(dirLightEntity2, 0.5);

			auto ambientLightEntity = scene.createEntity();
			scene.addComponent<AmbientLight>(ambientLightEntity, 0.2);

			auto greenMat = std::make_shared<Material>(prog, Colors::green());

			auto goldMat = std::make_shared<Material>(prog);
			goldMat->setMetallicFactor(0.5F);
			goldMat->setBaseColor(Colors::denormalize({ 0.944F, 0.776F, 0.373F, 1.F }));

			Cube cubeShape;
			auto cubeMesh = MeshData(cubeShape).createMesh(layout);
			auto cube = scene.createEntity();
			scene.addComponent<Renderable>(cube, std::move(cubeMesh), greenMat);
			scene.addComponent<Transform>(cube, glm::vec3{ 1.F, 1.F, 0 });

			Sphere shereShape;
			MeshData shereMeshData(shereShape);
			auto sphereMesh = shereMeshData.createMesh(layout);
			auto sphere = scene.createEntity();
			scene.addComponent<Renderable>(sphere, std::move(sphereMesh), goldMat);
			_trans = scene.addComponent<Transform>(sphere, glm::vec3{ -1.F, 1.F, 0 });

			auto floorEntity = scene.createEntity();
			Cube floorShape(glm::vec3(10.F, .5F, 10.F), glm::vec3(0, -0.25, 2));
			auto floorMesh = MeshData(floorShape).createMesh(prog->getVertexLayout());
			auto floorMat = std::make_shared<Material>(prog, Colors::red());
			floorMat->setProgramDefine("SHADOW_ENABLED");
			scene.addComponent<Renderable>(floorEntity, std::move(floorMesh), floorMat);
		}

		void drawDebugFrustum(Scene& scene, Transform& camTrans, Transform& dirLightTrans, const std::shared_ptr<Program>& prog)
		{
			auto addDebugShape = [&scene, prog]<typename T>(const T & shape, const Color & color)
			{
				auto mesh = MeshData(shape, RectangleMeshType::Outline)
					.createMesh(prog->getVertexLayout());
				auto entity = scene.createEntity();
				auto mat = std::make_shared<Material>(prog, color);
				mat->setPrimitiveType(MaterialPrimitiveType::Line);
				scene.addComponent<Renderable>(entity, std::move(mesh), mat);
			};

			camTrans.update();
			auto& camView = camTrans.getWorldInverse();
			auto camProjView = glm::perspective(glm::radians(60.F), getWindow().getAspect(), 0.3F, 5.F);
			// auto camProjView = cam.getProjectionMatrix();
			camProjView = camProjView * camView;
			dirLightTrans.update();
			auto& lightView = dirLightTrans.getWorldInverse();

			Frustum frust(camProjView);
			Frustum frust2 = camProjView * dirLightTrans.getWorldMatrix();
			BoundingBox bb = frust2.getBoundingBox();
			Frustum frust4 = bb.getOrtho() * dirLightTrans.getWorldInverse();

			addDebugShape(frust, Colors::magenta());
			addDebugShape(frust4, Colors::blue());
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
