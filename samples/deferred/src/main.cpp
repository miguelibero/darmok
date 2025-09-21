

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/camera.hpp>
#include <darmok/program.hpp>
#include <darmok/freelook.hpp>
#include <darmok/render_deferred.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/shadow.hpp>
#include <darmok/material.hpp>
#include <darmok/culling.hpp>
#include <darmok/shape.hpp>
#include <darmok/scene_serialize.hpp>

namespace
{
	using namespace darmok;

	class RotateUpdater final : public ISceneComponent
	{
	public:
		RotateUpdater(Transform& trans, float speed = 50.f)
			: _trans{ trans }
			, _speed{ speed }
		{
		}

		void update(float dt) override
		{
			auto r = _trans.getRotation();
			r = glm::quat{ glm::radians(glm::vec3{ dt * _speed, 0, 0 }) } *r;
			_trans.setRotation(r);
		}

	private:
		Transform& _trans;
		float _speed;
	};


	class DeferredSampleAppDelegate final : public IAppDelegate, IFreelookListener
	{
	public:
		DeferredSampleAppDelegate(App& app)
			: _app{ app }
			, _mouseVel{ 0 }
		{
		}

		void init() override
		{
			_app.setResetFlag(BGFX_RESET_SRGB_BACKBUFFER);
			_app.setResetFlag(BGFX_RESET_MSAA_X4);
			_app.setResetFlag(BGFX_RESET_MAXANISOTROPY);
			_app.setDebugFlag(BGFX_DEBUG_TEXT);

			auto scene = _app.addComponent<SceneAppComponent>().getScene();

			_cam = createCamera(*scene);
			_freeCam = createCamera(*scene, _cam);

			auto& freelook = scene->addSceneComponent<FreelookController>(*_freeCam);
			freelook.addListener(*this);

			scene->getRenderChain().addStep<ScreenSpaceRenderPass>(
				StandardProgramLoader::load(Program::Standard::Tonemap), "Tonemap");

			auto lightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(lightEntity, 0.05);

			auto dirLightEntity = scene->createEntity();
			auto& dirLightTrans = scene->addComponent<Transform>(dirLightEntity, glm::vec3{ -7.5, 3.5, 0 })
				.lookDir(glm::vec3{ 0, -1, 0 }, glm::vec3{ 0, 0, 1 });
			auto& dirLight = scene->addComponent<DirectionalLight>(dirLightEntity, 0.5);
			dirLight.setShadowType(LightDefinition::SoftShadow);
			scene->addSceneComponent<RotateUpdater>(dirLightTrans);

			auto prog = StandardProgramLoader::load(Program::Standard::ForwardBasic);
			auto arrowMesh = std::make_shared<Mesh>(MeshData{ Line{}, Mesh::Definition::Arrow }.createMesh(prog->getVertexLayout()));
			scene->addComponent<Renderable>(dirLightEntity, arrowMesh, prog, Colors::magenta());

			for (auto& lightConfig : _pointLights)
			{
				auto entity = scene->createEntity();
				auto& light = scene->addComponent<PointLight>(entity, lightConfig.intensity, lightConfig.color, lightConfig.radius);
				light.setShadowType(LightDefinition::HardShadow);
				scene->addComponent<Transform>(entity, lightConfig.position);
			}

			auto result = _app.getAssets().getSceneLoader()(*scene, "Sponza.dsc");
			assert(result);

			_mouseVel = glm::vec2{ 0 };
		}

	protected:

		void render() const override
		{
			bgfx::dbgTextPrintf(1, 1, 0x01f, "mouse velocity %f %f", _mouseVel.x, _mouseVel.y);
		}

		void update(float deltaTime) override
		{
			auto& mouse = _app.getInput().getMouse();
			auto vel = mouse.getVelocity() * 0.0004F;
			_mouseVel = glm::max(_mouseVel, glm::abs(vel));
		}

	private:
		App& _app;
		glm::vec2 _mouseVel;
		OptionalRef<Camera> _cam;
		OptionalRef<Camera> _freeCam;

		struct PointLightConfig final
		{
			glm::vec3 position;
			float intensity = 1.F;
			float radius = 1.F;
			Color3 color = Colors::white3();
		};

		static const std::vector<PointLightConfig> _pointLights;

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

		Camera& createCamera(Scene& scene, OptionalRef<Camera> mainCamera = nullptr)
		{
			auto entity = scene.createEntity();

			auto farPlane = mainCamera ? 40 : 20;
			auto& cam = scene.addComponent<Camera>(entity);
			cam.setPerspective(glm::radians(60.f), 0.3, farPlane);

			scene.addComponent<Transform>(entity)
				.setPosition(glm::vec3{ 0, 1, 0 })
				.lookAt(glm::vec3{ -7, 2, 0 });

			ShadowRendererConfig shadowConfig;
			// shadowConfig.mapMargin = glm::vec3(0.1);
			shadowConfig.cascadeAmount = 2;

			cam.addComponent<ForwardRenderer>();
			// cam.addComponent<OcclusionCuller>();
			cam.addComponent<FrustumCuller>();
			cam.addComponent<LightingRenderComponent>();
			cam.addComponent<ShadowRenderer>(shadowConfig);

			if (mainCamera)
			{
				cam.addComponent<CullingDebugRenderer>(mainCamera);
				cam.setEnabled(false);
				if (auto shadow = mainCamera->getComponent<ShadowRenderer>())
				{
					cam.addComponent<ShadowDebugRenderer>(shadow.value());
				}
			}

			return cam;
		}
	};

	const std::vector<DeferredSampleAppDelegate::PointLightConfig> DeferredSampleAppDelegate::_pointLights = {
		{{ -5.0f, 0.3f, 0.0f }, 10.F, 50.F, Colors::blue3()},
		{{ 0.0f, 0.3f, 0.0f }, 10.F, 50.F},
		{{ 5.0f, 0.3f, 0.0f }, 10.F, 50.F, Colors::red3()},
	};
}

DARMOK_RUN_APP(DeferredSampleAppDelegate);
