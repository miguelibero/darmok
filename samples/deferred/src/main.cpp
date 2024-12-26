

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/model.hpp>
#include <darmok/model_assimp.hpp>
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

namespace
{
	using namespace darmok;

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
			r = glm::quat(glm::radians(glm::vec3(dt * _speed, 0, 0))) * r;
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
			: _app(app)
		{
		}

		void init() override
		{
			_app.setResetFlag(BGFX_RESET_SRGB_BACKBUFFER);
			_app.setResetFlag(BGFX_RESET_MSAA_X4);
			_app.setResetFlag(BGFX_RESET_MAXANISOTROPY);
			_app.setDebugFlag(BGFX_DEBUG_TEXT);

			auto scene = _app.addComponent<SceneAppComponent>().getScene();
			_app.getAssets().getAssimpModelLoader().setConfig({
				.standardProgram = StandardProgramType::Forward
			});
			auto model = _app.getAssets().getModelLoader()("Sponza.dml");

			_cam = createCamera(*scene);
			_freeCam = createCamera(*scene, _cam);

			auto& freelook = scene->addSceneComponent<FreelookController>(*_freeCam);
			freelook.addListener(*this);

			scene->getRenderChain().addStep<ScreenSpaceRenderPass>(
				StandardProgramLoader::load(StandardProgramType::Tonemap), "Tonemap");

			auto lightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(lightEntity, 0.05);

			auto dirLightEntity = scene->createEntity();
			auto& dirLightTrans = scene->addComponent<Transform>(dirLightEntity, glm::vec3{ -7.5, 3.5, 0 })
				.lookDir(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
			auto& dirLight = scene->addComponent<DirectionalLight>(dirLightEntity, 0.5);
			dirLight.setShadowType(ShadowType::Soft);
			scene->addSceneComponent<RotateUpdater>(dirLightTrans);

			auto prog = StandardProgramLoader::load(StandardProgramType::ForwardBasic);
			std::shared_ptr<IMesh> arrowMesh = MeshData(Line(), LineMeshType::Arrow).createMesh(prog->getVertexLayout());
			scene->addComponent<Renderable>(dirLightEntity, arrowMesh, prog, Colors::magenta());

			for (auto& lightConfig : _pointLights)
			{
				auto entity = scene->createEntity();
				auto& light = scene->addComponent<PointLight>(entity, lightConfig.intensity, lightConfig.color, lightConfig.radius);
				light.setShadowType(ShadowType::Hard);
				scene->addComponent<Transform>(entity, lightConfig.position);
			}

			ModelSceneConfigurer configurer(*scene, _app.getAssets());
			configurer.setTextureFlags(BGFX_TEXTURE_NONE | BGFX_SAMPLER_MIN_ANISOTROPIC | BGFX_SAMPLER_MAG_ANISOTROPIC);
			auto modelEntity = configurer(*model);

			for (auto renderable : scene->getComponentsInChildren<Renderable>(modelEntity))
			{
				if (auto mat = renderable.get().getMaterial())
				{
					mat->setProgramDefine("SHADOW_ENABLED");
				}
			}

			_mouseVel = glm::vec2(0);
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
			cam.setPerspective(60, 0.3, farPlane);

			scene.addComponent<Transform>(entity)
				.setPosition(glm::vec3(0, 1, 0))
				.lookAt(glm::vec3(-7, 2, 0));

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
