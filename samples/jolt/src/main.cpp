

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
#include <darmok/physics3d.hpp>
#include <darmok/character.hpp>
#include <darmok/imgui.hpp>
#include <darmok/freelook.hpp>
#include <darmok/text.hpp>
#include <imgui.h>
#include <glm/gtx/string_cast.hpp>

#ifdef _DEBUG
#define PHYSICS_DEBUG_RENDERER
#include <darmok/physics3d_debug.hpp>
#endif

namespace
{
	using namespace darmok;
	using namespace darmok::physics3d;

	class JoltSampleApp : public App, public ICollisionListener, public ICharacterControllerDelegate, public IImguiRenderer
	{
	public:
		void init() override
		{
			App::init();

			_scene = addComponent<SceneAppComponent>().getScene();
			auto& physics = _scene->addSceneComponent<PhysicsSystem>(getAssets().getAllocator());

			auto prog = std::make_shared<Program>(StandardProgramType::Forward);

			// camera
			{
				auto camEntity = _scene->createEntity();
				glm::vec2 winSize = getWindow().getSize();

				_camTrans = _scene->addComponent<Transform>(camEntity)
					.setPosition({ 0, 5, -10 })
					.lookAt({ 0, 0, 0 });
				_cam = _scene->addComponent<Camera>(camEntity)
					.setPerspective(60, winSize.x / winSize.y, 0.3, 1000);

				_renderer = _cam->addRenderer<ForwardRenderer>();
				_renderer->addComponent<PhongLightingComponent>();

				_freeLook = _scene->addSceneComponent<FreelookController>(*_cam);
#ifdef PHYSICS_DEBUG_RENDERER
				_physicsDebugRenderer = _cam->addRenderer<PhysicsDebugRenderer>(physics);
#ifdef DARMOK_FREETYPE
				auto font = getAssets().getFontLoader()("../../assets/noto.ttf");
				_physicsDebugRenderer->setFont(font);
#endif
				_physicsDebugRenderer->setEnabled(false);
#endif
			}

			_imgui = addComponent<ImguiAppComponent>(*this);
			ImGui::SetCurrentContext(_imgui->getContext());

			{ // lights
				auto light = _scene->createEntity();
				_scene->addComponent<Transform>(light)
					.setPosition({ 1, 2, -2 });
				_scene->addComponent<PointLight>(light);
				_scene->addComponent<AmbientLight>(_scene->createEntity(), 0.5);
			}

			{ // floor
				auto floorEntity = _scene->createEntity();
				Cube floorShape(glm::vec3(10.F, .5F, 10.F), glm::vec3(0, -0.25, 0));
				_floorBody = _scene->addComponent<PhysicsBody>(floorEntity, floorShape, PhysicsBody::MotionType::Static);
				auto floorTex = getAssets().getColorTextureLoader()(Colors::grey());
				auto floorMesh = MeshData(floorShape).createMesh(prog->getVertexLayout());
				_scene->addComponent<Renderable>(floorEntity, std::move(floorMesh), prog, floorTex);
			}

			{ // door
				auto doorEntity = _scene->createEntity();
				Cube doorShape(glm::vec3(1.5F, 2.F, 0.2F), glm::vec3(0, 1.F, 0));
				PhysicsBodyConfig config;
				config.trigger = true;
				config.shape = doorShape;
				config.motion = PhysicsBodyMotionType::Kinematic;
				_doorBody = _scene->addComponent<PhysicsBody>(doorEntity, config);
				_scene->addComponent<Transform>(doorEntity)
					.setPosition(glm::vec3(2.F, 0.F, 2.F))
					.setEulerAngles(glm::vec3(0, 90, 0));

				auto doorTex = getAssets().getColorTextureLoader()(Color(255, 1000, 100, 255));
				auto triggerTex = getAssets().getColorTextureLoader()(Colors::red());
				_doorMat = std::make_shared<Material>(prog, doorTex);
				_triggerDoorMat = std::make_shared<Material>(prog, triggerTex);
				auto doorMesh = MeshData(doorShape).createMesh(prog->getVertexLayout());
				_scene->addComponent<Renderable>(doorEntity, std::move(doorMesh), _doorMat);
			}

			{ // cubes
				auto tex = getAssets().getColorTextureLoader()(Color(100, 255, 100, 255));
				auto touchedTex = getAssets().getColorTextureLoader()(Colors::green());
				_cubeMat = std::make_shared<Material>(prog, tex);
				_touchedCubeMat = std::make_shared<Material>(prog, touchedTex);
				_cubeMesh = MeshData(_cubeShape).createMesh(prog->getVertexLayout());

				for (auto x = -5.F; x < 5.F; x += 1.1F)
				{
					for (auto z = -5.F; z < 5.F; z += 1.1F)
					{
						glm::vec3 rot{ 45 * x, 0.F, 45.F * z };
						createCube().setPosition({ x, 10.F, z }).setEulerAngles(rot);
					}
				}
			}

			{ // player
				Capsule playerShape(1.F, 0.5F, glm::vec3(0.F, 1.F, 0.F ));
				// Cube playerShape(glm::vec3(1, 2, 1), glm::vec3(0, 1, 0));
				auto playerEntity = _scene->createEntity();
				_characterCtrl = _scene->addComponent<CharacterController>(playerEntity, playerShape);
				_characterCtrl->setDelegate(*this);

				CharacterConfig characterConfig{ playerShape };
				_characterBody = _scene->addComponent<PhysicsBody>(playerEntity, characterConfig);
				_characterBody->addListener(*this);

				auto playerTex = getAssets().getColorTextureLoader()(Colors::red());
				auto playerMesh = MeshData(playerShape).createMesh(prog->getVertexLayout());
				_scene->addComponent<Renderable>(playerEntity, std::move(playerMesh), prog, playerTex);
				_characterTrans = _scene->addComponent<Transform>(playerEntity);
			}
		}

		void onContactAdded(CharacterController& character, PhysicsBody& body, const Contact& contact, ContactSettings& settings) override
		{
			// settings.canPushCharacter = false;
			// settings.canReceiveImpulses = false;
		}

		void onCollisionEnter(PhysicsBody& me, PhysicsBody& body, const Collision& collision) override
		{
			if (_doorBody == body)
			{
				_inDoor = true;
			}
			if (_floorBody == body)
			{
				return;
			}
			setMaterial(body, _doorBody == body ? _triggerDoorMat : _touchedCubeMat);
		}

		void onCollisionExit(PhysicsBody& me, PhysicsBody& body) override
		{
			if (_doorBody == body)
			{
				_inDoor = false;
			}
			if (_floorBody == body)
			{
				return;
			}
			setMaterial(body, _doorBody == body ? _doorMat : _cubeMat);
		}

		void imguiRender() override
		{
			ImGui::TextWrapped("character controller");
			if (ImGui::BeginTable("table1", 2))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextWrapped("position");
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped(glm::to_string(_characterCtrl->getPosition()).c_str());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextWrapped("ground state");
				ImGui::TableSetColumnIndex(1);
				auto groundStateName = CharacterController::getGroundStateName(_characterCtrl->getGroundState());
				ImGui::TextWrapped(groundStateName.c_str());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextWrapped("in door");
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped(_inDoor ? "true": "false");

#ifdef PHYSICS_DEBUG_RENDERER
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextWrapped("render mode");
				ImGui::TableSetColumnIndex(1);
				auto debugEnabled = _physicsDebugRenderer->isEnabled();
				ImGui::TextWrapped(debugEnabled ? "debug renderer" : "meshes");
#endif
				ImGui::EndTable();
			}
			_imguiMouse = ImGui::GetIO().WantCaptureMouse;
		}

	protected:

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

		void updateLogic(float dt) override
		{
			if (_freeLook->isEnabled())
			{
				_imgui->setInputEnabled(false);
				return;
			}
			_imgui->setInputEnabled(true);
			if (_imguiMouse)
			{
				return;
			}
			auto& mouse = getInput().getMouse();
			if (mouse.getButton(MouseButton::Left))
			{
				glm::vec2 pos = mouse.getPosition();
				pos = getWindow().windowToScreenPoint(pos);
				auto ray = _cam->screenPointToRay(glm::vec3(pos, 0.F));
				auto dist = ray.intersect(_playerMovePlane);
				if (dist)
				{
					_characterCtrl->setPosition(ray * dist.value());
				}
				_characterCtrl->setLinearVelocity(glm::vec3(0));
				return;
			}

			auto& kb = getInput().getKeyboard();
			glm::vec3 dir(0);
			if (_characterCtrl->isGrounded())
			{
				dir.x = getInput().getAxis(_moveRight, _moveLeft);
				dir.z = getInput().getAxis(_moveForward, _moveBackward);

				if (_camTrans)
				{
					auto rot = _camTrans->getEulerAngles();
					rot.x = rot.z = 0.F; // rotate only in the y axis
					dir = glm::quat(glm::radians(rot)) * dir;
				}
				_characterCtrl->setLinearVelocity(dir * 10.F);
			}
			else if (!_characterBody)
			{
				_characterCtrl->setLinearVelocity(dir);
			}
		}

	private:
		Plane _playerMovePlane = Plane(glm::vec3(0, 1, 0), 0.0);
		OptionalRef<ImguiAppComponent> _imgui;
		OptionalRef<Camera> _cam;
		OptionalRef<Transform> _camTrans;
		OptionalRef<CharacterController> _characterCtrl;
		OptionalRef<PhysicsBody> _characterBody;
		OptionalRef<Transform> _characterTrans;
		OptionalRef<PhysicsBody> _doorBody;
		OptionalRef<PhysicsBody> _floorBody;
		OptionalRef<FreelookController> _freeLook;
		OptionalRef<ForwardRenderer> _renderer;
		std::shared_ptr<Scene> _scene;

		Cube _cubeShape;
		bool _imguiMouse = false;
		bool _inDoor = false;

		std::shared_ptr<IMesh> _cubeMesh;
		std::shared_ptr<Material> _cubeMat;
		std::shared_ptr<Material> _touchedCubeMat;
		std::shared_ptr<Material> _doorMat;
		std::shared_ptr<Material> _triggerDoorMat;

#ifdef PHYSICS_DEBUG_RENDERER
		OptionalRef<PhysicsDebugRenderer> _physicsDebugRenderer;
#endif

		bool setMaterial(PhysicsBody& body, const std::shared_ptr<Material>& mat)
		{
			auto entity = _scene->getEntity(body);
			auto renderable = _scene->getComponent<Renderable>(entity);
			if (!renderable)
			{
				return false;
			}
			renderable->setMaterial(mat);
			return true;
		}

		Transform& createCube() noexcept
		{
			auto entity = _scene->createEntity();
			_scene->addComponent<PhysicsBody>(entity, _cubeShape);
			_scene->addComponent<Renderable>(entity, _cubeMesh, _cubeMat);
			return _scene->addComponent<Transform>(entity);
		}
	};
}

DARMOK_RUN_APP(JoltSampleApp);
