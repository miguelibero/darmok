

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
	template<typename T>
	using unexpected = tl::unexpected<T>;

	class RotateUpdater final : public ISceneComponent
	{
	public:
		RotateUpdater(Transform& trans, float speed = 50.f) noexcept
			: _trans{ trans }
			, _speed{ speed }
		{
		}

		expected<void, std::string> update(float dt) noexcept override
		{
			auto r = _trans.getRotation();
			r = glm::quat{ glm::radians(glm::vec3{ dt * _speed, 0, 0 }) } *r;
			_trans.setRotation(r);
			return {};
		}

	private:
		Transform& _trans;
		float _speed;
	};


	class DeferredSampleAppDelegate final : public IAppDelegate, IFreelookListener
	{
	public:
		DeferredSampleAppDelegate(App& app) noexcept
			: _app{ app }
			, _mouseVel{ 0 }
		{
		}

		expected<void, std::string> init() noexcept override
		{
			_app.setResetFlag(BGFX_RESET_SRGB_BACKBUFFER);
			_app.setResetFlag(BGFX_RESET_MSAA_X4);
			_app.setResetFlag(BGFX_RESET_MAXANISOTROPY);
			_app.setDebugFlag(BGFX_DEBUG_TEXT);

			auto sceneResult = _app.addComponent<SceneAppComponent>();
			if(!sceneResult)
			{
				return unexpected{ std::move(sceneResult).error() };
			}
			auto scene = sceneResult.value().get().getScene();

			_cam = createCamera(*scene);
			_freeCam = createCamera(*scene, _cam);

			auto freelookResult = scene->addSceneComponent<FreelookController>(*_freeCam);
			if(!freelookResult)
			{
				return unexpected{ std::move(freelookResult).error() };
			}
			auto& freelook = freelookResult.value().get();
			freelook.addListener(*this);

			auto progResult = StandardProgramLoader::load(Program::Standard::Tonemap);
			if (!progResult)
			{
				return unexpected{ std::move(progResult).error() };
			}
			auto tonemapResult = scene->getRenderChain().addStep<ScreenSpaceRenderPass>(
				progResult.value(), "Tonemap");
			if (!tonemapResult)
			{
				return unexpected{ std::move(tonemapResult).error() };
			}

			auto lightEntity = scene->createEntity();
			scene->addComponent<AmbientLight>(lightEntity, 0.05);

			auto dirLightEntity = scene->createEntity();
			auto& dirLightTrans = scene->addComponent<Transform>(dirLightEntity, glm::vec3{ -7.5, 3.5, 0 })
				.lookDir(glm::vec3{ 0, -1, 0 }, glm::vec3{ 0, 0, 1 });
			auto& dirLight = scene->addComponent<DirectionalLight>(dirLightEntity, 0.5);
			dirLight.setShadowType(LightDefinition::SoftShadow);
			scene->tryAddSceneComponent<RotateUpdater>(dirLightTrans);

			progResult = StandardProgramLoader::load(Program::Standard::ForwardBasic);
			if (!progResult)
			{
				return unexpected{ std::move(progResult).error() };
			}
			auto prog = progResult.value();
			auto arrowMesh = std::make_shared<Mesh>(MeshData{ Line{}, Mesh::Definition::Arrow }.createMesh(prog->getVertexLayout()).value());
			scene->addComponent<Renderable>(dirLightEntity, arrowMesh, prog, Colors::magenta());

			for (auto& lightConfig : _pointLights)
			{
				auto entity = scene->createEntity();
				auto& light = scene->addComponent<PointLight>(entity, lightConfig.intensity, lightConfig.color, lightConfig.radius);
				light.setShadowType(LightDefinition::HardShadow);
				scene->addComponent<Transform>(entity, lightConfig.position);
			}

			auto sceneDefResult = _app.getAssets().getSceneDefinitionLoader()("Sponza.dsc");
			if(!sceneDefResult)
			{
				return unexpected{ std::move(sceneDefResult).error() };
			}
			auto result = SceneLoader{}(*sceneDefResult.value(), *scene);
			if(!result)
			{
				return unexpected{ std::move(result).error() };
			}

			_mouseVel = glm::vec2{ 0 };

			return {};
		}

	protected:

		expected<void, std::string> render() const noexcept override
		{
			bgfx::dbgTextPrintf(1, 1, 0x01f, "mouse velocity %f %f", _mouseVel.x, _mouseVel.y);
			return {};
		}

		expected<void, std::string> update(float deltaTime) noexcept override
		{
			auto& mouse = _app.getInput().getMouse();
			auto vel = mouse.getVelocity() * 0.0004F;
			_mouseVel = glm::max(_mouseVel, glm::abs(vel));
			return {};
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

		expected<void, std::string> onFreelookEnable(bool enabled) noexcept override
		{
			if (_freeCam)
			{
				_freeCam->setEnabled(enabled);
			}
			if (_cam)
			{
				_cam->setEnabled(!enabled);
			}
			return {};
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

			auto shadowDef = ShadowRenderer::createDefinition();
			shadowDef.set_cascade_amount(2);

			cam.tryAddComponent<ForwardRenderer>();
			// cam.tryAddComponent<OcclusionCuller>();
			cam.tryAddComponent<FrustumCuller>();
			cam.tryAddComponent<LightingRenderComponent>();
			cam.tryAddComponent<ShadowRenderer>(shadowDef);

			if (mainCamera)
			{
				cam.tryAddComponent<CullingDebugRenderer>(mainCamera);
				cam.tryAddComponent<ShadowDebugRenderer>(mainCamera);
				cam.setEnabled(false);
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
