

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
#include <darmok/render_scene.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/freelook.hpp>
#include <darmok/shape.hpp>
#include <darmok/environment.hpp>
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

	class PbrSampleApp : public App, IFreelookListener
	{
	public:
		void init() override
		{
			setResetFlag(BGFX_RESET_VSYNC);
			App::init();

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto prog = std::make_shared<Program>(StandardProgramType::Forward);
			auto& layout = prog->getVertexLayout();

			auto camData = createCamera(scene);
			_cam = camData.camera.get();

			auto freeCamData = createCamera(scene, camData.shadowRenderer.get());
			auto& freeCam = freeCamData.camera.get();
			freeCam.setEnabled(false);

			_freelook = scene.addSceneComponent<FreelookController>(freeCam);
			_freelook->addListener(*this);

			scene.getRenderChain().addStep<ScreenSpaceRenderPass>(
				std::make_shared<Program>(StandardProgramType::Tonemap), "Tonemap");

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

		void onFreelookEnable(bool enabled) noexcept override
		{
			if (_cam)
			{
				_cam->setEnabled(!enabled);
			}
		}

		void update(float deltaTime) noexcept override
		{
			if (_freelook && _freelook->isEnabled())
			{
				return;
			}
			if (!_trans)
			{
				return;
			}
			glm::vec3 dir(0);
			dir.x = getInput().getAxis(_moveRight, _moveLeft);
			dir.z = getInput().getAxis(_moveForward, _moveBackward);

			auto pos = _trans->getPosition();
			_trans->setPosition(pos + (dir * deltaTime));
		}

	private:

		OptionalRef<Camera> _cam;
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

		struct CameraData
		{
			std::reference_wrapper<Camera> camera;
			std::reference_wrapper<ShadowRenderer> shadowRenderer;
		};

		CameraData createCamera(Scene& scene, OptionalRef<ShadowRenderer> debugShadow = nullptr)
		{
			auto entity = scene.createEntity();

			scene.addComponent<Transform>(entity)
				.setPosition({ 0, 2, -2 })
				.lookAt({ 0, 0, 0 });

			auto farPlane = debugShadow ? 1000 : 5;
			auto& cam = scene.addComponent<Camera>(entity)
				.setWindowPerspective(60, 0.3, farPlane);

			ShadowRendererConfig shadowConfig;
			// shadowConfig.mapMargin = glm::vec3(0.1);
			shadowConfig.cascadeAmount = 2;

			auto& shadowRenderer = cam.addRenderer<ShadowRenderer>(shadowConfig);
			auto& fwdRender = cam.addRenderer<ForwardRenderer>();

			auto skyboxTex = getAssets().getTextureLoader()("cubemap.ktx");
			fwdRender.addComponent<SkyboxRenderComponent>(skyboxTex);

			fwdRender.addComponent<ShadowRenderComponent>(shadowRenderer);
			fwdRender.addComponent<LightingRenderComponent>();

			if (debugShadow)
			{
				fwdRender.addComponent<ShadowDebugRenderComponent>(debugShadow.value());
			}

			return { cam, shadowRenderer };
		}
	};

}

DARMOK_RUN_APP(PbrSampleApp);
