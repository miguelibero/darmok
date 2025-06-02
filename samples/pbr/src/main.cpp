

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
			: _trans{ trans }
			, _speed{ speed }
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
			: _trans{ trans }
			, _speed{ speed }
			, _paused{ false }
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
			r = glm::quat{ glm::radians(glm::vec3{0.f, dt * _speed, 0.f}) } *r;
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
			: _app{ app }
		{
			// app.setRendererType(bgfx::RendererType::Direct3D12);
		}

		void init() override
		{
			_app.setResetFlag(BGFX_RESET_VSYNC);

			auto& scene = *_app.addComponent<SceneAppComponent>().getScene();
			auto prog = StandardProgramLoader::load(Program::Standard::Forward);
			auto unlitProg = StandardProgramLoader::load(Program::Standard::Unlit);
			auto& layout = prog->getVertexLayout();

			_cam = createCamera(scene);
			_freeCam = createCamera(scene, _cam);

			_freelook = scene.addSceneComponent<FreelookController>(*_freeCam);
			_freelook->addListener(*this);

			scene.getRenderChain().addStep<ScreenSpaceRenderPass>(
				StandardProgramLoader::load(Program::Standard::Tonemap), "Tonemap");

			MeshData debugArrowMeshData{ Line{}, Mesh::Definition::Arrow };
			std::shared_ptr<IMesh> debugArrowMesh = debugArrowMeshData.createMesh(unlitProg->getVertexLayout());

			MeshData debugSphereMeshData{ Sphere{0.02f} };
			std::shared_ptr<IMesh> debugSphereMesh = debugSphereMeshData.createMesh(unlitProg->getVertexLayout());

			auto lightRootEntity = scene.createEntity();
			auto& lightRootTrans = scene.addComponent<Transform>(lightRootEntity);
			lightRootTrans.setPosition(glm::vec3{ 0.f, 1.5f, 2.f });
			auto pointLightEntity = scene.createEntity();
			auto& pointLightTrans = scene.addComponent<Transform>(pointLightEntity, glm::vec3{ 0.f, 1.f, 0.f });
			pointLightTrans.setParent(lightRootTrans);
			scene.addSceneComponent<CircleUpdater>(pointLightTrans);
			auto& pointLight = scene.addComponent<PointLight>(pointLightEntity, 100);
			pointLight.setShadowType(LightDefinition::HardShadow);
			scene.addComponent<Renderable>(pointLightEntity, debugSphereMesh, unlitProg, Colors::green());
			scene.addComponent<BoundingBox>(pointLightEntity, debugSphereMeshData.getBounds());

			auto dirLightEntity = scene.createEntity();
			auto& dirLightTrans = scene.addComponent<Transform>(dirLightEntity, glm::vec3{ -2.f, 2.f, -2.f })
				.lookDir(glm::vec3{ 0.f, -1.f, -1.f });
			auto& dirLight = scene.addComponent<DirectionalLight>(dirLightEntity, 1);
			dirLight.setShadowType(LightDefinition::SoftShadow);
			_rotateUpdaters.emplace_back(scene.addSceneComponent<RotateUpdater>(dirLightTrans));
			scene.addComponent<Renderable>(dirLightEntity, debugArrowMesh, unlitProg, Colors::magenta());
			scene.addComponent<BoundingBox>(dirLightEntity, debugArrowMeshData.getBounds());

			auto spotLightEntity = scene.createEntity();
			auto& spotLightTrans = scene.addComponent<Transform>(spotLightEntity, glm::vec3{ 1.f, 1.f, -1.f } * 2.f)
				.lookAt(glm::vec3{ 0.f });
			auto& spotLight = scene.addComponent<SpotLight>(spotLightEntity, 100).setConeAngle(glm::radians(15.f));
			spotLight.setShadowType(LightDefinition::HardShadow);
			scene.addComponent<Renderable>(spotLightEntity, debugArrowMesh, unlitProg, Colors::cyan());
			scene.addComponent<BoundingBox>(spotLightEntity, debugArrowMeshData.getBounds());
			_rotateUpdaters.emplace_back(scene.addSceneComponent<RotateUpdater>(spotLightTrans, -50.f));

			auto ambientLightEntity = scene.createEntity();
			scene.addComponent<AmbientLight>(ambientLightEntity, 0.2);

			auto greenMat = std::make_shared<Material>(prog, Colors::green());

			auto goldMat = std::make_shared<Material>(prog, Colors::denormalize({ 0.944f, 0.776f, 0.373f, 1.f }));
			goldMat->metallicFactor = 0.5f;

			Cube cubeShape;
			auto cubeMesh = MeshData{ cubeShape }.createMesh(layout);
			auto cube = scene.createEntity();
			scene.addComponent<Renderable>(cube, std::move(cubeMesh), greenMat);
			scene.addComponent<BoundingBox>(cube, cubeShape);
			scene.addComponent<Transform>(cube, glm::vec3{ 1.f, 1.f, 0 });

			Sphere shereShape;
			MeshData shereMeshData{ shereShape };
			auto sphereMesh = shereMeshData.createMesh(layout);
			auto sphere = scene.createEntity();
			scene.addComponent<Renderable>(sphere, std::move(sphereMesh), goldMat);
			scene.addComponent<BoundingBox>(sphere, shereShape);
			_trans = scene.addComponent<Transform>(sphere, glm::vec3{ -1.f, 1.f, 0 });

			auto floorEntity = scene.createEntity();
			Cube floorShape(glm::vec3{ 10.f, .5f, 10.f }, glm::vec3{ 0, -0.25, 2 });
			auto floorMesh = MeshData{ floorShape }.createMesh(prog->getVertexLayout());
			auto floorMat = std::make_shared<Material>(prog, Colors::red());
			floorMat->programDefines.insert("SHADOW_ENABLED");
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
			glm::vec3 dir{ 0.f };
			dir.x = _app.getInput().getAxis(_moveLeft, _moveRight);
			dir.z = _app.getInput().getAxis(_moveBackward, _moveForward);

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
				.setPosition({ 0.f, 4.f, -4.f })
				.lookAt({ 0.f, 0.f, 0.f });

			auto farPlane = mainCamera ? 100.f: 20.f;
			auto& cam = scene.addComponent<Camera>(entity)
				.setPerspective(glm::radians(60.f), 0.3f, farPlane);

			auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx").value();
			cam.addComponent<SkyboxRenderer>(skyboxTex);
			cam.addComponent<GridRenderer>();
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
