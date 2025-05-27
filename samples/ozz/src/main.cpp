#include <darmok/app.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/window.hpp>
#include <darmok/asset.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/scene.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/shape.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/input.hpp>
#include <darmok/program.hpp>
#include <darmok/scene_assimp.hpp>
#include <darmok/freelook.hpp>
#include <darmok/text.hpp>
#include <darmok/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace
{
	using namespace darmok;

	class CircleUpdater final : public ISceneComponent
	{
	public:
		CircleUpdater(Transform& trans, float speed = 1.f)
			: _trans{ trans }
			, _speed{ speed }
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

	class OzzSampleAppDelegate final : public IAppDelegate, public IInputEventListener
	{
	public:
		OzzSampleAppDelegate(App& app)
			: _app{ app }
			, _animTime{ 0.F }
		{
		}

		void init() override
		{
			_app.setDebugFlag(BGFX_DEBUG_TEXT);

			auto& scene = *_app.addComponent<SceneAppComponent>().getScene();
			scene.addSceneComponent<SkeletalAnimationSceneComponent>();

			auto prog = StandardProgramLoader::load(StandardProgramLoader::Type::Forward);
			
			auto camEntity = scene.createEntity();
			scene.addComponent<Transform>(camEntity)
				.setPosition({ 0.f, 2, -2 })
				.lookAt({ 0, 1, 0 });

			auto& cam = scene.addComponent<Camera>(camEntity)
				.setPerspective(60, 0.3, 1000);
			cam.addComponent<ForwardRenderer>();
			cam.addComponent<LightingRenderComponent>();
			cam.addComponent<SkeletalAnimationRenderComponent>();
			_freeLook = scene.addSceneComponent<FreelookController>(cam);

			auto unlitProg = StandardProgramLoader::load(StandardProgramLoader::Type::Unlit);
			auto debugMat = std::make_shared<Material>(unlitProg, Colors::magenta());

			auto lightRootEntity = scene.createEntity();
			auto& lightRootTrans = scene.addComponent<Transform>(lightRootEntity, glm::vec3{ 0, 1.5, -1 });
			auto lightEntity = scene.createEntity();
			auto& lightTrans = scene.addComponent<Transform>(lightEntity, glm::vec3{ 0, 1, 0 });
			lightTrans.setParent(lightRootTrans);
			std::shared_ptr<IMesh> lightMesh = MeshData{ Sphere{0.1} }.createMesh(unlitProg->getVertexLayout());
			scene.addComponent<Renderable>(lightEntity, lightMesh, debugMat);
			scene.addSceneComponent<CircleUpdater>(lightTrans);
			scene.addComponent<PointLight>(lightEntity, 5).setRange(5);
			scene.addComponent<AmbientLight>(lightEntity, 0.5);

			auto skel = _app.getAssets().getSkeletonLoader()("skeleton.ozz").value();

			auto animEntity = scene.createEntity();
			auto& animTrans = scene.addComponent<Transform>(animEntity);

			auto animDef = _app.getAssets().getSkeletalAnimatorDefinitionLoader()("animator.json").value();
			auto anims = SkeletalAnimatorUtils::loadAnimations(*animDef, _app.getAssets().getSkeletalAnimationLoader());
			_animator = scene.addComponent<SkeletalAnimator>(animEntity, skel, anims, *animDef);

			_animator->play("run");
			// _animator->setPlaybackSpeed(0.05);

			auto skelEntity = scene.createEntity();
			auto& skelTrans = scene.addComponent<Transform>(skelEntity, glm::vec3{ -1, 0, 0 });
			skelTrans.setParent(animTrans);
			
			auto boneMat = std::make_shared<Material>(prog, Colors::grey());
			auto& renderSkel = scene.addComponent<RenderableSkeleton>(skelEntity, boneMat);
#ifdef DARMOK_FREETYPE
			renderSkel.setFont(_app.getAssets().getFontLoader()("../../assets/noto.ttf").value());
			cam.addComponent<TextRenderer>();
#endif
			auto baseTex = _app.getAssets().getTextureLoader()("BasicMotions_DummyType01-BaseColor.png").value();
			auto metalTex = _app.getAssets().getTextureLoader()("BasicMotions_DummyType01-Metallic.png").value();
			auto normalTex = _app.getAssets().getTextureLoader()("BasicMotions_DummyType01-Normal.png").value();
			auto ambientTex = _app.getAssets().getTextureLoader()("BasicMotions_DummyType01-AmbientOcclusion.png").value();

			auto mat = std::make_shared<Material>(prog, Colors::fromNumber(0xFAB137FF));
			mat->programDefines = { "SKINNING_ENABLED" };
			mat->textures[Material::TextureType::BaseColor] = baseTex;
			mat->textures[Material::TextureType::MetallicRoughness] = metalTex;
			mat->textures[Material::TextureType::Normal] = normalTex;
			mat->textures[Material::TextureType::Occlusion] = ambientTex;
			mat->occlusionStrength = 0.75F;
			mat->metallicFactor = 1.F;

			auto skinRoot = _app.getAssets().getSceneLoader()(scene, "scene.pb").value();
			
			auto skinEntity = scene.createEntity();
			auto& skinTrans = scene.addComponent<Transform>(skinEntity, glm::vec3{ 1, 0, 0 });
			skinTrans.setParent(animTrans);
			scene.getOrAddComponent<Transform>(skinRoot).setParent(skinTrans);

			for (auto entity : scene.getComponents<Renderable>())
			{
				if (entity >= skinRoot)
				{
					scene.getComponent<Renderable>(entity)->setMaterial(mat);
				}
			}
			
			_app.getInput().addListener("talk", _talkInputs, *this);
		}

		void onInputEvent(const std::string& tag) override
		{
			_animator->play(tag);
		}

		void render() const override
		{
			const bgfx::Stats* stats = bgfx::getStats();

			bgfx::dbgTextPrintf(0, 1, 0x0f, "Blend position: %s", glm::to_string(_animator->getBlendPosition()).c_str());
		}

		void update(float deltaTime) noexcept override
		{
			if (_freeLook->isEnabled())
			{
				return;
			}

			glm::vec2 dir{
				_app.getInput().getAxis(_moveRight, _moveLeft),
				_app.getInput().getAxis(_moveForward, _moveBackward)
			};

			_animator->setBlendPosition(dir);
			if (glm::length(dir) > 0.1F)
			{
				_animator->play("locomotion");
			}
		}

	private:
		App& _app;
		float _animTime;
		OptionalRef<SkeletalAnimator> _animator;
		OptionalRef<FreelookController> _freeLook;

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
	};
}

DARMOK_RUN_APP(OzzSampleAppDelegate);
