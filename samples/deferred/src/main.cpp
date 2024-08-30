

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


	class DeferredSampleApp : public App, IFreelookListener
	{
	public:
		void init() override
		{
			setResetFlag(BGFX_RESET_SRGB_BACKBUFFER);
			setResetFlag(BGFX_RESET_MSAA_X4);
			setResetFlag(BGFX_RESET_MAXANISOTROPY);
			setDebugFlag(BGFX_DEBUG_TEXT);

			App::init();

			auto scene = addComponent<SceneAppComponent>().getScene();
			getAssets().getAssimpModelLoader().setConfig({
				.standardProgram = StandardProgramType::Forward
			});
			auto model = getAssets().getModelLoader()("Sponza.dml");

			_cam = createCamera(*scene);
			auto shadowRenderer = _cam->getComponent<ShadowRenderer>();

			_freeCam = createCamera(*scene, shadowRenderer);
			_freeCam->setEnabled(false);

			auto& freelook = scene->addSceneComponent<FreelookController>(*_freeCam);
			freelook.addListener(*this);

			// scene->getRenderChain().addStep<ScreenSpaceRenderPass>(
			// 	std::make_shared<Program>(StandardProgramType::Tonemap), "Tonemap");

			auto lightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(lightEntity, 0.05);

			auto dirLightEntity = scene->createEntity();
			auto& dirLightTrans = scene->addComponent<Transform>(dirLightEntity, glm::vec3{ -7.5, 3.5, 0 })
				.lookDir(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
			scene->addComponent<DirectionalLight>(dirLightEntity, 0.5);
			scene->addSceneComponent<RotateUpdater>(dirLightTrans);

			auto prog = std::make_shared<Program>(StandardProgramType::ForwardBasic);
			std::shared_ptr<IMesh> arrowMesh = MeshData(Line(), LineMeshType::Arrow).createMesh(prog->getVertexLayout());
			scene->addComponent<Renderable>(dirLightEntity, arrowMesh, prog, Colors::magenta());

			for (auto& lightConfig : _pointLights)
			{
				auto entity = scene->createEntity();
				scene->addComponent<PointLight>(entity, lightConfig.intensity, lightConfig.color, lightConfig.radius);
				scene->addComponent<Transform>(entity, lightConfig.position);
			}

			ModelSceneConfigurer configurer(*scene, getAssets());
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
			App::render();
			bgfx::dbgTextPrintf(1, 1, 0x01f, "mouse velocity %f %f", _mouseVel.x, _mouseVel.y);
		}

		void update(float deltaTime) override
		{
			App::update(deltaTime);
			auto& mouse = getInput().getMouse();
			auto vel = mouse.getVelocity() * 0.0004F;
			_mouseVel = glm::max(_mouseVel, glm::abs(vel));
		}

	private:
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

		Camera& createCamera(Scene& scene, OptionalRef<ShadowRenderer> debugShadow = nullptr)
		{
			auto entity = scene.createEntity();

			auto farPlane = debugShadow ? 40 : 20;
			auto& cam = scene.addComponent<Camera>(entity);
			cam.setWindowPerspective(60, 0.3, farPlane);

			scene.addComponent<Transform>(entity)
				.setPosition(glm::vec3(0, 1, 0))
				.lookAt(glm::vec3(-7, 2, 0));

			ShadowRendererConfig shadowConfig;
			// shadowConfig.mapMargin = glm::vec3(0.1);
			shadowConfig.cascadeAmount = 2;

			auto& fwdRender = cam.addComponent<ForwardRenderer>();

			fwdRender.addComponent<LightingCameraComponent>();

			if (debugShadow)
			{
				fwdRender.addComponent<ShadowDebugRenderComponent>(debugShadow.value());
			}

			auto& shadowRenderer = cam.addComponent<ShadowRenderer>(shadowConfig);
			fwdRender.addComponent<ShadowRenderComponent>(shadowRenderer);

			return cam;
		}
	};

	const std::vector<DeferredSampleApp::PointLightConfig> DeferredSampleApp::_pointLights = {
		{{ -5.0f, 0.3f, 0.0f }, 10.F, 5.F, Colors::blue3()},
		{{ 0.0f, 0.3f, 0.0f }, 10.F, 5.F},
		{{ 5.0f, 0.3f, 0.0f }, 10.F, 5.F, Colors::red3()},
	};
}

DARMOK_RUN_APP(DeferredSampleApp);
