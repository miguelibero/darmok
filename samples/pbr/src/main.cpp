

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
			, _paused(false)
		{
		}

		void togglePaused() noexcept
		{
			_paused = !_paused;
		}

		void update(float dt) noexcept override
		{
			if (_paused)
			{
				return;
			}
			auto r = _trans.getRotation();
			r = glm::quat(glm::radians(glm::vec3(0, dt * _speed, 0))) * r;
			_trans.setRotation(r);
		}

	private:
		Transform& _trans;
		float _speed;
		bool _paused;
	};

	class PbrSampleApp : public App, IFreelookListener, IInputEventListener
	{
	public:
		void init() override
		{
			setResetFlag(BGFX_RESET_VSYNC);
			App::init();

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto prog = std::make_shared<Program>(StandardProgramType::Forward);
			auto& layout = prog->getVertexLayout();

			_cam = createCamera(scene);
			auto shadowRenderer = _cam->getComponent<ShadowRenderer>();

			_freeCam = createCamera(scene, shadowRenderer);

			_freelook = scene.addSceneComponent<FreelookController>(*_freeCam);
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
			auto& dirLightTrans = scene.addComponent<Transform>(dirLightEntity, glm::vec3{ -2, 2, -2 })
				.lookDir(glm::vec3(0, -1, -1));
			scene.addComponent<DirectionalLight>(dirLightEntity, 0.5);
			_rotateUpdater = scene.addSceneComponent<RotateUpdater>(dirLightTrans);

			std::shared_ptr<IMesh> arrowMesh = MeshData(Line(), LineMeshType::Arrow).createMesh(prog->getVertexLayout());
			scene.addComponent<Renderable>(dirLightEntity, arrowMesh, prog, Colors::magenta());

			/*
			auto dirLightEntity2 = scene.createEntity();
			scene.addComponent<Transform>(dirLightEntity2, glm::vec3{ 1, 1, -1 })
				.lookAt(glm::vec3(0, 0, 0));
			scene.addComponent<DirectionalLight>(dirLightEntity2, 0.5);
			*/

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

			getInput().addListener("pause", _pauseEvent, *this);
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
		OptionalRef<Camera> _freeCam;
		OptionalRef<FreelookController> _freelook;
		OptionalRef<Transform> _trans;
		OptionalRef<RotateUpdater> _rotateUpdater;

		const InputEvent _pauseEvent = KeyboardInputEvent{ KeyboardKey::KeyP };

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

		void onFreelookEnable(bool enabled) noexcept override
		{
			if (_freeCam)
			{
				_freeCam->setEnabled(enabled);
			}
			if (_cam)
			{
				_cam->setEnabled(!enabled);
			}
		}

		void onInputEvent(const std::string& tag) noexcept
		{
			if (tag == "pause" && _rotateUpdater)
			{
				_rotateUpdater->togglePaused();
			}
		}

		Camera& createCamera(Scene& scene, OptionalRef<ShadowRenderer> debugShadow = nullptr)
		{
			auto entity = scene.createEntity();

			scene.addComponent<Transform>(entity)
				.setPosition({ 0, 4, -4 })
				.lookAt({ 0, 0, 0 });

			auto farPlane = debugShadow ? 100 : 20;
			auto& cam = scene.addComponent<Camera>(entity)
				.setWindowPerspective(60, 0.3, farPlane);
			if (debugShadow)
			{
				cam.setEnabled(false);
			}

			ShadowRendererConfig shadowConfig;
			shadowConfig.cascadeAmount = 3;

			auto& fwdRender = cam.addComponent<ForwardRenderer>();

			auto skyboxTex = getAssets().getTextureLoader()("cubemap.ktx");
			cam.addComponent<SkyboxRenderComponent>(skyboxTex);

			cam.addComponent<LightingCameraComponent>();

			if (debugShadow)
			{
				cam.addComponent<ShadowDebugRenderComponent>(debugShadow.value());
			}

			auto& shadowRenderer = cam.addComponent<ShadowRenderer>(shadowConfig);
			cam.addComponent<ShadowRenderComponent>(shadowRenderer);

			return cam;
		}
	};

}

DARMOK_RUN_APP(PbrSampleApp);
