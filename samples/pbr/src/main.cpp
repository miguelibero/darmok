

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/mesh.hpp>
#include <darmok/transform.hpp>
#include <darmok/light.hpp>
#include <darmok/window.hpp>
#include <darmok/camera.hpp>
#include <darmok/input.hpp>
#include <darmok/program.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/render.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/freelook.hpp>

namespace
{
	using namespace darmok;

	class PbrSampleApp : public App
	{
	public:
		void init() override
		{
			App::init();

			auto scene = addComponent<SceneAppComponent>().getScene();
			auto prog = std::make_shared<Program>(StandardProgramType::Forward);
			auto& layout = prog->getVertexLayout();
			auto& registry = scene->getRegistry();

			auto camEntity = registry.create();

			auto& camTrans = registry.emplace<Transform>(camEntity)
				.setPosition({ 0, 2, -2 })
				.setEulerAngles({ 45, 0, 0 });
			auto& cam = registry.emplace<Camera>(camEntity)
				.setWindowPerspective(60, 0.3, 1000);
			
			cam.addRenderer<ForwardRenderer>()
				.addComponent<PhongLightingComponent>();

			_freelook = scene->addSceneComponent<FreelookController>(cam);

			auto light = registry.create();
			registry.emplace<Transform>(light)
				.setPosition({ 1, 1, -2 });
			registry.emplace<PointLight>(light);

			auto greenTex = getAssets().getColorTextureLoader()(Colors::green());
			auto greenMat = std::make_shared<Material>(prog);
			greenMat->setTexture(MaterialTextureType::Diffuse, greenTex);

			auto cubeMesh = MeshData(Cube()).createMesh(layout);
			auto cube = registry.create();
			registry.emplace<Renderable>(cube, std::move(cubeMesh), greenMat);
			registry.emplace<Transform>(cube)
				.setPosition({ 1.5F, 0, 0 });

			auto redTex = getAssets().getColorTextureLoader()(Colors::red());
			auto redMat = std::make_shared<Material>(prog);
			redMat->setTexture(MaterialTextureType::Diffuse, redTex);

			auto sphereMesh = MeshData(Sphere()).createMesh(layout);
			auto sphere = registry.create();
			registry.emplace<Renderable>(sphere, std::move(sphereMesh), redMat);
			_trans = registry.emplace<Transform>(sphere);
		}

		void update(float deltaTime) override
		{
			if (_freelook && _freelook->isEnabled())
			{
				return;
			}
			glm::vec3 dir(0);
			dir.x = getInput().getAxis(_moveRight, _moveLeft);
			dir.z = getInput().getAxis(_moveForward, _moveBackward);

			auto pos = _trans->getPosition();
			_trans->setPosition(pos + (dir * deltaTime));
		}

		OptionalRef<FreelookController> _freelook;
		OptionalRef<Transform> _trans;

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

DARMOK_RUN_APP(PbrSampleApp);
