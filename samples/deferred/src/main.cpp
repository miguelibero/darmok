

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
#include <darmok/texture.hpp>
#include <darmok/shadow.hpp>
#include <darmok/material.hpp>
#include <darmok/culling.hpp>
#include <darmok/shape.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/environment.hpp>

namespace
{
	using namespace darmok;

	class RotateUpdater final : public ISceneComponent
	{
	public:
		RotateUpdater(Transform& trans, float speed = 20.f) noexcept
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

            OptionalRef<SceneAppComponent> sceneComp;
            DARMOK_TRY_VALUE_PREFIX(sceneComp, _app.addComponent<SceneAppComponent>(), "adding scene component");
            auto scene = sceneComp->getScene();

            auto matConfig = MaterialRenderConfig::createDefault(true);

            std::shared_ptr<Texture> envIrradiance;
            DARMOK_TRY_VALUE_PREFIX(envIrradiance, _app.getAssets().getTextureLoader()("env_irradiance.bin"), "loading env irradiance texture");
            matConfig.defaultTextures[Material::TextureDefinition::EnvironmentIrradiance] = envIrradiance;

            std::shared_ptr<Texture> envPrefiltered;
            DARMOK_TRY_VALUE_PREFIX(envPrefiltered, _app.getAssets().getTextureLoader()("env_prefiltered.bin"), "loading env prefiltered texture");
            matConfig.defaultTextures[Material::TextureDefinition::EnvironmentPrefiltered] = envPrefiltered;

            DARMOK_TRY_PREFIX(_app.addComponent<MaterialAppComponent>(std::move(matConfig)), "adding material component");

    		_cam = createCamera(*scene);
            _freeCam = createCamera(*scene, _cam);

            OptionalRef<FreelookController> freelookRef;

            DARMOK_TRY_VALUE_PREFIX(freelookRef, scene->addSceneComponent<FreelookController>(*_cam), "adding debug freelook component");
            DARMOK_TRY_PREFIX(freelookRef->setEnabled(true), "enable freelook");

            DARMOK_TRY_PREFIX(_cam->addComponent<SkyboxRenderer>(envPrefiltered), "adding skybox component");

            std::shared_ptr<Program> prog;
            DARMOK_TRY_VALUE_PREFIX(prog, StandardProgramLoader::load(Program::Standard::Tonemap), "loading tonemap program");
            DARMOK_TRY_PREFIX(scene->getRenderChain().addStep<ScreenSpaceRenderPass>(prog, "Tonemap"), "adding tonemap step");

            auto lightEntity = scene->createEntity();
            scene->addComponent<AmbientLight>(lightEntity, 0.05);

            auto dirLightEntity = scene->createEntity();
            auto& dirLightTrans = scene->addComponent<Transform>(dirLightEntity, glm::vec3{-7.5, 3.5, 0});
            auto& dirLight = scene->addComponent<DirectionalLight>(dirLightEntity, 0.5);
            dirLight.setShadowType(LightDefinition::SoftShadow);
            scene->tryAddSceneComponent<RotateUpdater>(dirLightTrans);

            DARMOK_TRY_VALUE_PREFIX(prog, StandardProgramLoader::load(Program::Standard::ForwardBasic), "loading forward basic program");

            auto arrowMesh = std::make_shared<Mesh>(MeshData{Line{glm::vec3{0}, glm::vec3{0, 0, 1}}, Mesh::Definition::Arrow}.createMesh(prog->getVertexLayout()).value());
            scene->addComponent<Renderable>(dirLightEntity, arrowMesh, prog, Colors::magenta());

			for (auto& lightConfig : _pointLights)
			{
				auto entity = scene->createEntity();
				auto& light = scene->addComponent<PointLight>(entity, lightConfig.intensity, lightConfig.color, lightConfig.radius);
				// light.setShadowType(LightDefinition::HardShadow);
				scene->addComponent<Transform>(entity, lightConfig.position);
			}

            std::shared_ptr<Scene::Definition> sceneDef;
            DARMOK_TRY_VALUE_PREFIX(sceneDef, _app.getAssets().getSceneDefinitionLoader()("Sponza.dsc"), "loading sponza definition");
            DARMOK_TRY_PREFIX(SceneLoader{}(*sceneDef, *scene), "loading sponza into scene");

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
			float radius = 1000.F;
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

			auto& cam = scene.addComponent<Camera>(entity);
            cam.setPerspective(glm::radians(60.f), 0.3, 40);

			scene.addComponent<Transform>(entity)
				.setPosition(glm::vec3{ 0, 1, 0 })
				.lookAt(glm::vec3{ -7, 2, 0 });

			auto shadowDef = ShadowRenderer::createDefinition();
            shadowDef.set_map_size(4096);

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
        {{0.0f, 8.0f, 0.0f}, 0.5f},
        {{0.0f, 3.0f, 0.0f}, 0.5f},
        {{0.0f, 1.0f, 5.0f}, 0.1f},
	};
}

DARMOK_RUN_APP(DeferredSampleAppDelegate);
