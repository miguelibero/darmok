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
#include <darmok/render_scene.hpp>
#include <darmok/input.hpp>
#include <darmok/program.hpp>
#include <darmok/model_assimp.hpp>
#include <darmok/freelook.hpp>
#include <darmok/text.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace
{
	using namespace darmok;

	class CircleUpdater final : public ISceneComponent
	{
	public:
		CircleUpdater(Transform& trans, float speed = 1.f)
			: _trans(trans)
			, _speed(speed)
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
			renderer.addComponent<LightingRenderComponent>();
			renderer.addComponent<SkeletalAnimationRenderComponent>();
			_freeLook = scene.addSceneComponent<FreelookController>(cam);

			auto unlitProg = std::make_shared<Program>(StandardProgramType::Unlit);
			auto debugMat = std::make_shared<Material>(unlitProg, Colors::magenta());
			debugMat->setProgramDefine("TEXTURE_DISABLED");

			auto lightRootEntity = scene.createEntity();
			auto& lightRootTrans = scene.addComponent<Transform>(lightRootEntity, glm::vec3{ 0, 1.5, -1 });
			auto lightEntity = scene.createEntity();
			auto& lightTrans = scene.addComponent<Transform>(lightEntity, lightRootTrans, glm::vec3{ 0, 1, 0 });
			std::shared_ptr<IMesh> lightMesh = MeshData(Sphere(0.01)).createMesh(unlitProg->getVertexLayout());
			scene.addComponent<Renderable>(lightEntity, lightMesh, debugMat);
			scene.addSceneComponent<CircleUpdater>(lightTrans);
			scene.addComponent<PointLight>(lightEntity, 5);
			scene.addComponent<AmbientLight>(lightEntity, 0.5);

			auto skel = getAssets().getSkeletonLoader()("skeleton.ozz");

			auto animEntity = scene.createEntity();
			auto& animTrans = scene.addComponent<Transform>(animEntity);

			auto animConfig = getAssets().getSkeletalAnimatorConfigLoader()("animator.json");

			auto anims = animConfig.loadAnimations(getAssets().getSkeletalAnimationLoader());
			_animator = scene.addComponent<SkeletalAnimator>(animEntity, skel, anims, animConfig);
			_animator->play("locomotion");

			auto skelEntity = scene.createEntity();
			scene.addComponent<Transform>(skelEntity, animTrans, glm::vec3(-1, 0, 0));
			
			auto boneMat = std::make_shared<Material>(prog, Colors::grey());
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
