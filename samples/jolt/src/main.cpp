

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
#include <darmok/program_standard.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/render.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/physics3d.hpp>
#include <darmok/physics3d_debug.hpp>
#include <darmok/character.hpp>
#include <darmok/imgui.hpp>
#include <darmok/freelook.hpp>
#include <imgui.h>
#include <glm/gtx/string_cast.hpp>

#ifndef _NDEBUG
#define PHYSICS_DEBUG_RENDERER
#endif

namespace
{
	using namespace darmok;
	using namespace darmok::physics3d;

	class JoltSampleApp : public App, public ICharacterControllerListener, public IImguiRenderer
	{
	public:
		void init() override
		{
			App::init();

			_scene = addComponent<SceneAppComponent>().getScene();
			auto& physics = _scene->addSceneComponent<PhysicsSystem>(getAssets().getAllocator());

			auto prog = getAssets().getStandardProgramLoader()(StandardProgramType::ForwardPhong);

			// camera
			{
				auto camEntity = _scene->createEntity();
				glm::vec2 winSize = getWindow().getSize();

				_camTrans = _scene->addComponent<Transform>(camEntity)
					.setPosition({ 0, 5, -10 })
					.lookAt({ 0, 0, 0 });
				_cam = _scene->addComponent<Camera>(camEntity)
					.setPerspective(60, winSize.x / winSize.y, 0.3, 1000);

				_cam->addComponent<PhongLightingComponent>();
				_cam->setRenderer<ForwardRenderer>();
				_freeLook = _scene->addSceneComponent<FreelookController>(*_camTrans);
#ifdef PHYSICS_DEBUG_RENDERER
				_physicsDebugRenderer = _cam->addComponent<PhysicsDebugRenderer>(physics);

				getInput().addBindings("physics", {
					{ KeyboardBindingKey { KeyboardKey::F7 }, true,
						[this]() { switchMode(); }
					}
				});
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
				_characterCtrl->addListener(*this);

				CharacterConfig characterConfig{ playerShape };
				_characterBody = _scene->addComponent<PhysicsBody>(playerEntity, characterConfig);

				auto playerTex = getAssets().getColorTextureLoader()(Colors::red());
				auto playerMesh = MeshData(playerShape).createMesh(prog->getVertexLayout());
				_scene->addComponent<Renderable>(playerEntity, std::move(playerMesh), prog, playerTex);
				_characterTrans = _scene->addComponent<Transform>(playerEntity);
			}

#ifdef PHYSICS_DEBUG_RENDERER
			switchMode();
#endif
		}

		void onCollisionEnter(CharacterController& ctrl, PhysicsBody& body, const Collision& collision) override
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

		void onCollisionExit(CharacterController& ctrl, PhysicsBody& body) override
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
				return;
			}

			auto& kb = getInput().getKeyboard();
			if (_characterCtrl->isGrounded())
			{
				glm::vec3 dir(0);
				if (kb.getKey(KeyboardKey::Right) || kb.getKey(KeyboardKey::KeyD))
				{
					dir.x += 1;
				}
				else if (kb.getKey(KeyboardKey::Left) || kb.getKey(KeyboardKey::KeyA))
				{
					dir.x -= 1;
				}
				else if (kb.getKey(KeyboardKey::Up) || kb.getKey(KeyboardKey::KeyW))
				{
					dir.z += 1;
				}
				else if (kb.getKey(KeyboardKey::Down) || kb.getKey(KeyboardKey::KeyS))
				{
					dir.z -= 1;
				}
				if (_camTrans)
				{
					dir = _camTrans->getRotation() * dir;
				}
				_characterCtrl->setLinearVelocity(dir * 10.F);
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

		void switchMode()
		{
			if (!_physicsDebugRenderer)
			{
				return;
			}
			auto renderablesEnabled = !_physicsDebugRenderer->isEnabled();
			_physicsDebugRenderer->setEnabled(renderablesEnabled);
			renderablesEnabled = !renderablesEnabled;
			auto view = _scene->getComponentView<Renderable>();
			for (auto [entity, renderable] : view.each())
			{
				renderable.setEnabled(renderablesEnabled);
			}
		}
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
