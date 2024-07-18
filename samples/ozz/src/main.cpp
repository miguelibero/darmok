#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/scene.hpp>
#include <darmok/model.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/render.hpp>
#include <darmok/input.hpp>
#include <darmok/program.hpp>
#include <darmok/model_assimp.hpp>
#include <darmok/freelook.hpp>
#include <darmok/text.hpp>

namespace
{
	using namespace darmok;

	class OzzSampleApp : public App
	{
	public:
		void init()
		{
			App::init();

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			scene.addSceneComponent<SkeletalAnimationSceneComponent>();

			auto prog = std::make_shared<Program>(StandardProgramType::Forward);
			
			auto camEntity = scene.createEntity();
			scene.addComponent<Transform>(camEntity)
				.setPosition(glm::vec3(0.f, 2, -2))
				.lookAt(glm::vec3(0, 1, 0));

			auto& cam = scene.addComponent<Camera>(camEntity)
				.setPerspective(60, getWindow().getSize(), 0.3, 1000);
			auto& renderer = cam.addRenderer<ForwardRenderer>();
			renderer.addComponent<PhongLightingComponent>();
			renderer.addComponent<SkeletalAnimationRenderComponent>();
			_freeLook = scene.addSceneComponent<FreelookController>(cam);

			auto lightEntity = scene.createEntity();
			scene.addComponent<Transform>(lightEntity, glm::vec3{ 50, 50, -100 });
			scene.addComponent<PointLight>(lightEntity);

			scene.addComponent<AmbientLight>(scene.createEntity(), 0.8);

			auto skel = getAssets().getSkeletonLoader()("skeleton.ozz");			

			auto animEntity = scene.createEntity();
			auto& animTrans = scene.addComponent<Transform>(animEntity);

			auto animConfig = getAssets().getSkeletalAnimatorConfigLoader()("animator.json");

			auto anims = animConfig.loadAnimations(getAssets().getSkeletalAnimationLoader());
			_animator = scene.addComponent<SkeletalAnimator>(animEntity, skel, anims, animConfig);
			_animator->play("locomotion");

			auto skelEntity = scene.createEntity();
			scene.addComponent<Transform>(skelEntity, animTrans, glm::vec3(-1, 0, 0));
			
			auto boneTex = getAssets().getColorTextureLoader()(Colors::grey());
			auto boneMat = std::make_shared<Material>(prog, boneTex);
			auto& renderSkel = scene.addComponent<RenderableSkeleton>(skelEntity, boneMat);
#ifdef DARMOK_FREETYPE
			renderSkel.setFont(getAssets().getFontLoader()("../assets/noto.ttf"));
			cam.addRenderer<TextRenderer>();
#endif

			auto modelTex = getAssets().getTextureLoader()("BasicMotionsTexture.png");
			auto model = getAssets().getModelLoader()("model.dml");

			auto skinEntity = scene.createEntity();
			scene.addComponent<Transform>(skinEntity, animTrans, glm::vec3(1, 0, 0));

			ModelSceneConfigurer configurer(scene, getAssets());
			configurer.setParent(skinEntity);
			configurer(*model, [&scene, modelTex](const auto& node, Entity entity)
			{
				auto renderable = scene.getComponent<Renderable>(entity);
				if (renderable)
				{
					auto mat = renderable->getMaterial();
					mat->setProgramDefine("SKINNING_ENABLED");
					mat->setTexture(MaterialTextureType::Diffuse, modelTex);
				}
			});

			auto talk = [this]() { _animator->play("talk"); };

			getInput().addBindings("talk", {
				{ KeyboardBindingKey{ KeyboardKey::Return }, true, talk },
				{ GamepadBindingKey{ GamepadButton::A, 0 }, true, talk },
			});
		}

	protected:

		void updateLogic(float deltaTime) noexcept override
		{
			App::updateLogic(deltaTime);

			if (_freeLook->isEnabled())
			{
				return;
			}

			auto& kb = getInput().getKeyboard();
			glm::vec2 dir(0);
			if (kb.getKey(KeyboardKey::Up) || kb.getKey(KeyboardKey::KeyW))
			{
				dir.y += 1;
			}
			if (kb.getKey(KeyboardKey::Down) || kb.getKey(KeyboardKey::KeyS))
			{
				dir.y -= 1;
			}
			if (kb.getKey(KeyboardKey::Right) || kb.getKey(KeyboardKey::KeyD))
			{
				dir.x += 1;
			}
			if (kb.getKey(KeyboardKey::Left) || kb.getKey(KeyboardKey::KeyA))
			{
				dir.x -= 1;
			}

			auto gp = getInput().getGamepad(0);
			if (gp)
			{
				dir += glm::vec2(gp->getStick(GamepadStick::Left));
			}

			_animator->setBlendPosition(dir);
			if (dir != glm::vec2(0))
			{
				_animator->play("locomotion");
			}
		}
	private:
		float _animTime;
		OptionalRef<SkeletalAnimator> _animator;
		OptionalRef<FreelookController> _freeLook;
	};
}

DARMOK_RUN_APP(OzzSampleApp);
