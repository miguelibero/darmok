

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
#include <darmok/render_scene.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/physics3d_character.hpp>
#include <darmok/imgui.hpp>
#include <darmok/freelook.hpp>
#include <darmok/text.hpp>
#include <imgui.h>
#include <glm/gtx/string_cast.hpp>

#ifdef _DEBUG
#define PHYSICS_DEBUG_RENDER
#include <darmok/physics3d_debug.hpp>
#endif

namespace
{
	using namespace darmok;
	using namespace darmok::physics3d;

	class JoltSampleAppDelegate final : public IAppDelegate, public ICollisionListener, public ICharacterDelegate, public IImguiRenderer
	{
	public:
		JoltSampleAppDelegate(App& app)
			: _app{ app }
		{
		}

		void init() override
		{
			_scene = _app.addComponent<SceneAppComponent>().getScene();
			_scene->addSceneComponent<PhysicsSystem>(_app.getAssets().getAllocator());

			auto prog = StandardProgramLoader::load(Program::Standard::ForwardBasic);

			// camera
			{
				auto camEntity = _scene->createEntity();

				_camTrans = _scene->addComponent<Transform>(camEntity)
					.setPosition({ 0.f, 5.f, -10.f })
					.lookAt({ 0.f, 0.f, 0.f });
				_cam = _scene->addComponent<Camera>(camEntity)
					.setPerspective(glm::radians(60.f), 0.3f, 1000.f);

				_cam->addComponent<ForwardRenderer>();
				_cam->addComponent<LightingRenderComponent>();

				_freeLook = _scene->addSceneComponent<FreelookController>(*_cam);
#ifdef PHYSICS_DEBUG_RENDER
				PhysicsDebugConfig physicsDebugconfig;
#ifdef DARMOK_FREETYPE
				physicsDebugconfig.render.font = _app.getAssets().getFontLoader()("../../assets/noto.ttf").value();
#endif
				_physicsDebugRender = _cam->addComponent<PhysicsDebugRenderer>(physicsDebugconfig);
				_physicsDebugRender->setEnabled(false);
#endif
			}

			_imgui = _app.addComponent<ImguiAppComponent>(*this);
			ImGui::SetCurrentContext(_imgui->getContext());

			{ // lights
				auto light = _scene->createEntity();
				_scene->addComponent<Transform>(light)
					.setPosition({ 1, 2, -2 })
					.lookDir({ -1, -1, 0 });
				_scene->addComponent<PointLight>(light, 0.5f)
					.setRange(5);
				_scene->addComponent<DirectionalLight>(light, 0.5f);
				_scene->addComponent<AmbientLight>(_scene->createEntity(), 0.5);
			}

			{ // floor
				auto floorEntity = _scene->createEntity();
				Cube floorShape{ {10.f, .5f, 10.f}, {0.f, -0.25f, 0.f} };
				_floorBody = _scene->addComponent<PhysicsBody>(floorEntity, floorShape, PhysicsBody::Definition::Static);
				auto floorMesh = MeshData{ floorShape }.createSharedMesh(prog->getVertexLayout());
				_scene->addComponent<Renderable>(floorEntity, std::move(floorMesh), prog, Colors::grey());
			}

			{ // door
				auto doorEntity = _scene->createEntity();
				Cube doorShape{ {1.5f, 2.f, 0.2f}, {0, 1.f, 0} };
				auto def = PhysicsBody::createDefinition();
				def.set_trigger(true);
				*def.mutable_shape()->mutable_cube() = darmok::convert(doorShape);
				def.set_motion(PhysicsBody::Definition::Kinematic);
				_doorBody = _scene->addComponent<PhysicsBody>(doorEntity, def);
				_scene->addComponent<Transform>(doorEntity)
					.setPosition({ 2.f, 0.f, 2.f })
					.setEulerAngles(glm::radians(glm::vec3{ 0, 90, 0 }));

				_doorMat = std::make_shared<Material>(prog, Color{ 255, 100, 100, 255 });
				_triggerDoorMat = std::make_shared<Material>(prog, Colors::red());
				auto doorMesh = MeshData{ doorShape }.createSharedMesh(prog->getVertexLayout());
				_scene->addComponent<Renderable>(doorEntity, std::move(doorMesh), _doorMat);
			}

			{ // cubes
				_cubeMat = std::make_shared<Material>(prog, Color{ 100, 255, 100, 255 });
				_touchedCubeMat = std::make_shared<Material>(prog, Colors::green());
				_cubeMesh = MeshData{ _cubeShape }.createSharedMesh(prog->getVertexLayout());

				for (auto x = -5.f; x < 5.f; x += 1.1f)
				{
					for (auto z = -5.f; z < 5.f; z += 1.1f)
					{
						glm::vec3 rot{ 45 * x, 0.f, 45.f * z };
						createCube().setPosition({ x, 10.f, z }).setEulerAngles(glm::radians(rot));
					}
				}
			}

			{ // player
				Capsule playerShape{1.f, 0.5f, { 0.f, 1.f, 0.f }};
				auto playerEntity = _scene->createEntity();
				_characterCtrl = _scene->addComponent<CharacterController>(playerEntity, playerShape);
				_characterCtrl->setDelegate(*this);

				auto charDef = PhysicsBody::createCharacterDefinition();
				*charDef.mutable_base()->mutable_shape()->mutable_capsule() = darmok::convert(playerShape);
				
				_characterBody = _scene->addComponent<PhysicsBody>(playerEntity, charDef);
				_characterBody->addListener(*this);

				auto playerMesh = MeshData{ playerShape }.createSharedMesh(prog->getVertexLayout());
				_scene->addComponent<Renderable>(playerEntity, std::move(playerMesh), prog, Colors::red());
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
				ImGui::TextWrapped("%s", glm::to_string(_characterCtrl->getPosition()).c_str());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextWrapped("ground state");
				ImGui::TableSetColumnIndex(1);
				auto groundStateName = CharacterController::getGroundStateName(_characterCtrl->getGroundState());
				ImGui::TextWrapped("%s", groundStateName.c_str());

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextWrapped("in door");
				ImGui::TableSetColumnIndex(1);
				ImGui::TextWrapped(_inDoor ? "true": "false");

#ifdef PHYSICS_DEBUG_RENDER
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextWrapped("render mode");
				ImGui::TableSetColumnIndex(1);
				auto debugEnabled = _physicsDebugRender->isEnabled();
				ImGui::TextWrapped("%s", debugEnabled ? "debug renderer" : "meshes");
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

		void update(float dt) override
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
			auto& mouse = _app.getInput().getMouse();
			if (mouse.getButton(MouseButton::Left))
			{
				glm::vec2 pos = mouse.getPosition();
				pos = _app.getWindow().windowToScreenPoint(pos);
				auto ray = _cam->screenPointToRay({ pos, 0.f });
				auto dist = ray.intersect(_playerMovePlane);
				if (dist)
				{
					_characterCtrl->setPosition(ray * dist.value());
				}
				_characterCtrl->setLinearVelocity(glm::vec3{ 0 });
				return;
			}

			auto& kb = _app.getInput().getKeyboard();
			glm::vec3 dir{ 0 };
			if (_characterCtrl->isGrounded())
			{
				dir.x = _app.getInput().getAxis(_moveLeft, _moveRight);
				dir.z = _app.getInput().getAxis(_moveBackward, _moveForward);

				if (_camTrans)
				{
					auto rot = _camTrans->getEulerAngles();
					rot.x = rot.z = 0.f; // rotate only in the y axis
					dir = glm::quat{ glm::radians(rot) } *dir;
				}
				_characterCtrl->setLinearVelocity(dir * 10.f);
			}
			else if (!_characterBody)
			{
				_characterCtrl->setLinearVelocity(dir);
			}
		}

	private:
		App& _app;
		Plane _playerMovePlane{ { 0, 1, 0 }, 0.0 };
		OptionalRef<ImguiAppComponent> _imgui;
		OptionalRef<Camera> _cam;
		OptionalRef<Transform> _camTrans;
		OptionalRef<CharacterController> _characterCtrl;
		OptionalRef<PhysicsBody> _characterBody;
		OptionalRef<Transform> _characterTrans;
		OptionalRef<PhysicsBody> _doorBody;
		OptionalRef<PhysicsBody> _floorBody;
		OptionalRef<FreelookController> _freeLook;
		std::shared_ptr<Scene> _scene;

		Cube _cubeShape;
		bool _imguiMouse = false;
		bool _inDoor = false;

		std::shared_ptr<Mesh> _cubeMesh;
		std::shared_ptr<Material> _cubeMat;
		std::shared_ptr<Material> _touchedCubeMat;
		std::shared_ptr<Material> _doorMat;
		std::shared_ptr<Material> _triggerDoorMat;

#ifdef PHYSICS_DEBUG_RENDER
		OptionalRef<PhysicsDebugRenderer> _physicsDebugRender;
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

DARMOK_RUN_APP(JoltSampleAppDelegate);
