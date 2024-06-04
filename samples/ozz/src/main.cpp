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
			
			auto& registry = scene.getRegistry();

			auto camEntity = registry.create();
			registry.emplace<Transform>(camEntity)
				.setPosition(glm::vec3(0.f, 2, -2))
				.lookAt(glm::vec3(0, 1, 0));

			auto& cam = registry.emplace<Camera>(camEntity)
				.setPerspective(60, getWindow().getSize(), 0.3, 1000);
			cam.setRenderer<ForwardRenderer>();
			cam.addComponent<PhongLightingComponent>();
			cam.addComponent<SkeletalAnimationCameraComponent>();

			auto lightEntity = registry.create();
			registry.emplace<Transform>(lightEntity, glm::vec3{ 50, 50, -100 });
			registry.emplace<PointLight>(lightEntity);
			registry.emplace<AmbientLight>(registry.create(), 0.8);

			auto skel = getAssets().getSkeletonLoader()("skeleton.ozz");			
			auto runAnim = getAssets().getSkeletalAnimationLoader()("BasicMotions@Run01 - Forwards.ozz");
			auto idleAnim = getAssets().getSkeletalAnimationLoader()("BasicMotions@Idle01.ozz");

			auto skelEntity = registry.create();
			registry.emplace<Transform>(skelEntity, glm::vec3(-1, 0, 0));

			SkeletalAnimatorConfig animConfig;
			animConfig.addState(runAnim, "run");
			animConfig.addState(idleAnim, "idle");
			animConfig.addTransition(runAnim->getName(), idleAnim->getName(), {
				.exitTime = 0.9F,
				.duration = 0.1F
				});
			animConfig.addTransition(idleAnim->getName(), runAnim->getName(), {
				.exitTime = 0.9F,
				.duration = 0.1F
			});

			_animator = registry.emplace<SkeletalAnimator>(skelEntity, skel, animConfig);
			_animator->play("idle");

			auto boneTex = getAssets().getColorTextureLoader()(Colors::grey());
			auto boneMat = std::make_shared<Material>(prog, boneTex);
			registry.emplace<RenderableSkeleton>(skelEntity, boneMat);

			auto modelTex = getAssets().getTextureLoader()("BasicMotionsTexture.png");
			auto model = getAssets().getModelLoader()("BasicMotionsDummyModelBin.fbx");

			auto skinEntity = registry.create();
			registry.emplace<Transform>(skinEntity, glm::vec3(1, 0, 0));

			ModelSceneConfigurer configurer(scene.getRegistry(), prog, getAssets());
			configurer.setParent(skinEntity);
			configurer.run(model, [&registry, modelTex, skel, &animConfig](const auto& node, Entity entity) {
				auto renderable = registry.try_get<Renderable>(entity);
				if (renderable != nullptr)
				{
					auto mat = renderable->getMaterial();
					if (mat != nullptr)
					{
						mat->setTexture(MaterialTextureType::Diffuse, modelTex);
					}
					auto& animator = registry.emplace<SkeletalAnimator>(entity, skel, animConfig);
					animator.play("idle");
				}
			});
		}
	protected:

		void updateLogic(float deltaTime)
		{
			App::updateLogic(deltaTime);
			auto state = _animator->getCurrentState();
			if (state && state->getNormalizedTime() > 0.9)
			{
				_animator->play(state->getName() == "run" ? "idle" : "run");
			}
		}

	private:
		OptionalRef<SkeletalAnimator> _animator;
	};
}

DARMOK_RUN_APP(OzzSampleApp);
