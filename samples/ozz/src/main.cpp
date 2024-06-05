#include <darmok/app.hpp>
#include <darmok/cegui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/scene.hpp>
#include <darmok/model.hpp>
#include <darmok/program.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/render.hpp>

namespace
{
	using namespace darmok;

	class OzzSampleApp : public App
	{
	public:
		void init(const std::vector<std::string>& args) override
		{
			App::init(args);
			
			auto& scene = *addComponent<SceneAppComponent>().getScene();
			auto& skelUpdater = scene.addLogicUpdater<SkeletalAnimationUpdater>();

			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);
			
			auto camEntity = scene.createEntity();
			scene.addComponent<Transform>(camEntity)
				.setPosition(glm::vec3(0.f, 2, -2))
				.lookAt(glm::vec3(0, 1, 0));

			auto& cam = scene.addComponent<Camera>(camEntity)
				.setPerspective(60, getWindow().getSize(), 0.3, 1000);
			cam.setRenderer<ForwardRenderer>();
			cam.addComponent<PhongLightingComponent>();
			cam.addComponent<SkeletalAnimationCameraComponent>();

			auto lightEntity = scene.createEntity();
			scene.addComponent<Transform>(lightEntity, glm::vec3{ 50, 50, -100 });
			scene.addComponent<PointLight>(lightEntity);

			scene.addComponent<AmbientLight>(scene.createEntity(), 0.8);

			auto skel = getAssets().getSkeletonLoader()("skeleton.ozz");			
			auto runAnim = getAssets().getSkeletalAnimationLoader()("BasicMotions@Run01 - Forwards.ozz");
			auto idleAnim = getAssets().getSkeletalAnimationLoader()("BasicMotions@Idle01.ozz");

			auto animEntity = scene.createEntity();
			auto& animTrans = scene.addComponent<Transform>(animEntity);
			SkeletalAnimatorConfig animConfig;
			animConfig.addState(runAnim, "run");
			animConfig.addState(idleAnim, "idle");
			animConfig.addTransition("run", "idle", {
				.exitTime = 0.5F,
				.duration = 0.5F
			});
			animConfig.addTransition("idle", "run", {
				.exitTime = 0.5F,
				.duration = 0.5F
			});

			_animator = scene.addComponent<SkeletalAnimator>(animEntity, skel, animConfig);
			_animator->play("idle");

			auto skelEntity = scene.createEntity();
			scene.addComponent<Transform>(skelEntity, animTrans, glm::vec3(-1, 0, 0));
			
			auto boneTex = getAssets().getColorTextureLoader()(Colors::grey());
			auto boneMat = std::make_shared<Material>(prog, boneTex);
			scene.addComponent<RenderableSkeleton>(skelEntity, boneMat);

			auto modelTex = getAssets().getTextureLoader()("BasicMotionsTexture.png");
			auto model = getAssets().getModelLoader()("BasicMotionsDummyModelBin.fbx");

			auto skinEntity = scene.createEntity();
			scene.addComponent<Transform>(skinEntity, animTrans, glm::vec3(1, 0, 0));

			ModelSceneConfigurer configurer(scene.getRegistry(), prog, getAssets());
			configurer.setParent(skinEntity);
			configurer.run(model, [&scene, modelTex](const auto& node, Entity entity)
			{
				auto renderable = scene.getComponent<Renderable>(entity);
				if (renderable)
				{
					auto mat = renderable->getMaterial();
					mat->setTexture(MaterialTextureType::Diffuse, modelTex);
				}
			});
		}

	protected:

		void updateLogic(float deltaTime) noexcept override
		{
			App::updateLogic(deltaTime);
			_animTime += deltaTime;
			if (_animTime > 2.F)
			{
				auto state = _animator->getCurrentState();
				_animTime = 0.F;
				_animator->play(!state || state->getName() == "idle" ? "run" : "idle");
			}
		}
	private:
		float _animTime;
		OptionalRef<SkeletalAnimator> _animator;
	};
}

DARMOK_RUN_APP(OzzSampleApp);
