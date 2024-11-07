

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
#include <darmok/render_chain.hpp>
#include <darmok/freelook.hpp>
#include <darmok/shape.hpp>
#include <darmok/environment.hpp>
#include <darmok/culling.hpp>
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

	class PbrSampleAppDelegate : public IAppDelegate, IFreelookListener, IInputEventListener
	{
	public:
		PbrSampleAppDelegate(App& app)
			: _app(app)
		{
			// app.setRendererType(bgfx::RendererType::Direct3D12);
		}

		void init() override
		{
			_app.setResetFlag(BGFX_RESET_VSYNC);

			auto& scene = *_app.addComponent<SceneAppComponent>().getScene();
			auto prog = std::make_shared<Program>(StandardProgramType::Forward);
			auto unlitProg = std::make_shared<Program>(StandardProgramType::Unlit);
			auto& layout = prog->getVertexLayout();

			_cam = createCamera(scene);
			_freeCam = createCamera(scene, _cam);

			_freelook = scene.addSceneComponent<FreelookController>(*_freeCam);
			_freelook->addListener(*this);

			scene.getRenderChain().addStep<ScreenSpaceRenderPass>(
				std::make_shared<Program>(StandardProgramType::Tonemap), "Tonemap");

			auto lightRootEntity = scene.createEntity();
			auto& lightRootTrans = scene.addComponent<Transform>(lightRootEntity, glm::vec3{ 0, 1.5, -1 });
			auto lightEntity = scene.createEntity();
			auto& lightTrans = scene.addComponent<Transform>(lightEntity, lightRootTrans, glm::vec3{ 0, 1, 0 });
			scene.addSceneComponent<CircleUpdater>(lightTrans);
			scene.addComponent<PointLight>(lightEntity, 0.5);

			auto dirLightEntity = scene.createEntity();
			auto& dirLightTrans = scene.addComponent<Transform>(dirLightEntity, glm::vec3{ -2, 2, -2 })
				.lookDir(glm::vec3(0, -1, -1));
			auto& dirLight = scene.addComponent<DirectionalLight>(dirLightEntity, 0.5);
			// dirLight.setShadowType(ShadowType::Soft);
			_rotateUpdaters.emplace_back(scene.addSceneComponent<RotateUpdater>(dirLightTrans));

			MeshData arrowMeshData(Line(), LineMeshType::Arrow);
			std::shared_ptr<IMesh> arrowMesh = arrowMeshData.createMesh(unlitProg->getVertexLayout());
			scene.addComponent<Renderable>(dirLightEntity, arrowMesh, unlitProg, Colors::magenta());
			scene.addComponent<BoundingBox>(dirLightEntity, arrowMeshData.getBounds());

			auto spotLightEntity = scene.createEntity();
			auto& spotLightTrans = scene.addComponent<Transform>(spotLightEntity, glm::vec3{1, 1, -1} * 4.F)
				.lookAt(glm::vec3(0, 0, 0));
			auto& spotLight = scene.addComponent<SpotLight>(spotLightEntity, 100).setConeAngle(glm::radians(15.F));
			spotLight.setShadowType(ShadowType::Hard);
			scene.addComponent<Renderable>(spotLightEntity, arrowMesh, unlitProg, Colors::cyan());
			scene.addComponent<BoundingBox>(spotLightEntity, arrowMeshData.getBounds());
			// _rotateUpdaters.emplace_back(scene.addSceneComponent<RotateUpdater>(spotLightTrans, -25.F));

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
			scene.addComponent<BoundingBox>(cube, cubeShape);
			scene.addComponent<Transform>(cube, glm::vec3{ 1.F, 1.F, 0 });

			Sphere shereShape;
			MeshData shereMeshData(shereShape);
			auto sphereMesh = shereMeshData.createMesh(layout);
			auto sphere = scene.createEntity();
			scene.addComponent<Renderable>(sphere, std::move(sphereMesh), goldMat);
			scene.addComponent<BoundingBox>(sphere, shereShape);
			_trans = scene.addComponent<Transform>(sphere, glm::vec3{ -1.F, 1.F, 0 });

			auto floorEntity = scene.createEntity();
			Cube floorShape(glm::vec3(10.F, .5F, 10.F), glm::vec3(0, -0.25, 2));
			auto floorMesh = MeshData(floorShape).createMesh(prog->getVertexLayout());
			auto floorMat = std::make_shared<Material>(prog, Colors::red());
			floorMat->setProgramDefine("SHADOW_ENABLED");
			scene.addComponent<Renderable>(floorEntity, std::move(floorMesh), floorMat);
			scene.addComponent<BoundingBox>(floorEntity, floorShape);

			_app.getInput().addListener("pause", { _pauseEvent }, *this);
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
			dir.x = _app.getInput().getAxis(_moveRight, _moveLeft);
			dir.z = _app.getInput().getAxis(_moveForward, _moveBackward);

			auto pos = _trans->getPosition();
			_trans->setPosition(pos + (dir * deltaTime));
		}

	private:
		App& _app;
		OptionalRef<Camera> _cam;
		OptionalRef<Camera> _freeCam;
		OptionalRef<FreelookController> _freelook;
		OptionalRef<Transform> _trans;
		std::vector<std::reference_wrapper<RotateUpdater>> _rotateUpdaters;

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
			if (tag == "pause")
			{
				for(auto& updater : _rotateUpdaters)
				{
					updater.get().togglePaused();
				}
			}
		}

		Camera& createCamera(Scene& scene, OptionalRef<Camera> mainCamera = nullptr)
		{
			auto entity = scene.createEntity();

			scene.addComponent<Transform>(entity)
				.setPosition({ 0, 4, -4 })
				.lookAt({ 0, 0, 0 });

			auto farPlane = mainCamera ? 100 : 20;
			auto& cam = scene.addComponent<Camera>(entity)
				.setViewportPerspective(60, 0.3, farPlane);

			auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx");
			cam.addComponent<SkyboxRenderer>(skyboxTex);
			cam.addComponent<LightingRenderComponent>();

			ShadowRendererConfig shadowConfig;
			shadowConfig.cascadeAmount = 3;
			cam.addComponent<ShadowRenderer>(shadowConfig);
			cam.addComponent<ForwardRenderer>();
			cam.addComponent<FrustumCuller>();
			// cam.addComponent<OcclusionCuller>();

			if (mainCamera)
			{
				cam.addComponent<CullingDebugRenderer>(mainCamera.value());
				cam.setEnabled(false);
				if (auto shadow = mainCamera->getComponent<ShadowRenderer>())
				{
					cam.addComponent<ShadowDebugRenderer>(shadow.value());
				}
			}

			return cam;
		}
	};

}

DARMOK_RUN_APP(PbrSampleAppDelegate);
