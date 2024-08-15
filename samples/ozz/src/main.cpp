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
#include <glm/gtx/string_cast.hpp>

namespace
{
	using namespace darmok;

	class OzzSampleApp : public App, public IInputEventListener
	{
	public:
		void init()
		{
			App::init();
			setDebugFlag(BGFX_DEBUG_TEXT);

			auto& scene = *addComponent<SceneAppComponent>().getScene();
			scene.addSceneComponent<SkeletalAnimationSceneComponent>();

			auto prog = std::make_shared<Program>(StandardProgramType::Forward);
			
			auto camEntity = scene.createEntity();
			scene.addComponent<Transform>(camEntity)
				.setPosition(glm::vec3(0.f, 2, -2))
				.lookAt(glm::vec3(0, 1, 0));

			auto& cam = scene.addComponent<Camera>(camEntity)
				.setWindowPerspective(60, 0.3, 1000);
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
			renderSkel.setFont(getAssets().getFontLoader()("../../assets/noto.ttf"));
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
					mat->setTexture(modelTex);
				}
			});

			getInput().addListener("talk", _talkInputs, *this);
		}

		void onInputEvent(const std::string& tag) override
		{
			_animator->play(tag);
		}

		void render() const override
		{
			App::render();

			const bgfx::Stats* stats = bgfx::getStats();

			bgfx::dbgTextPrintf(0, 1, 0x0f, "Blend position: %s", glm::to_string(_animator->getBlendPosition()).c_str());

		}

	protected:

		const InputEvents _talkInputs = {
			KeyboardInputEvent{ KeyboardKey::Return },
			GamepadInputEvent{ GamepadButton::A },
		};
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

		void update(float deltaTime) noexcept override
		{
			App::update(deltaTime);

			if (_freeLook->isEnabled())
			{
				return;
			}

			glm::vec2 dir(
				getInput().getAxis(_moveRight, _moveLeft),
				getInput().getAxis(_moveForward, _moveBackward)
			);

			_animator->setBlendPosition(dir);
			if (glm::length(dir) > 0.1F)
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
