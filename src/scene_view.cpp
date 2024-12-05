#include <darmok-editor/scene_view.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <darmok/imgui.hpp>
#include <darmok/scene.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/environment.hpp>
#include <darmok/light.hpp>
#include <darmok/shadow.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/culling.hpp>
#include <darmok/input.hpp>

#include <glm/glm.hpp>
#include <imgui.h>
#include <ImGuizmo.h>

namespace darmok::editor
{
    const std::string EditorSceneView::_windowName = "Scene View";

    EditorSceneView::EditorSceneView(App& app)
        : _app(app)
        , _mouseMode(MouseSceneViewMode::None)
        , _focused(false)        
        , _transGizmoMode(TransformGizmoMode::Grab)
        , _selectedEntity(entt::null)
    {
    }

    void EditorSceneView::init()
    {
    }
    
    void EditorSceneView::shutdown()
    {
        _scene.reset();
        _sceneBuffer.reset();
        _editorCam.reset();
        _focused = false;
        _mouseMode = MouseSceneViewMode::None;
    }

    void EditorSceneView::render()
    {
        ImGuizmo::BeginFrame();
    }

    bool EditorSceneView::shouldCameraRender(const Camera& cam) const noexcept
    {
        return _editorCam.ptr() == &cam;
    }

    bool EditorSceneView::shouldEntityBeSerialized(Entity entity) const noexcept
    {
        return !isEditorEntity(entity);
    }

    bool EditorSceneView::isEditorEntity(Entity entity) const noexcept
    {
        if (_scene && _editorCam && _scene->getEntity(_editorCam.value()) == entity)
        {
            return true;
        }
        return false;
    }

    EditorSceneView& EditorSceneView::setScene(const std::shared_ptr<Scene>& scene)
    {
        _scene = scene;

        scene->setDelegate(*this);

        static const std::string name = "Editor Camera";
        auto skyboxTex = _app.getAssets().getTextureLoader()("cubemap.ktx");
        auto camEntity = scene->createEntity();
        auto& cam = scene->addComponent<Camera>(camEntity);

        scene->addComponent<Transform>(camEntity)
            .setPosition(glm::vec3(10, 5, -10))
            .lookAt(glm::vec3(0))
            .setName(name);
        cam.setPerspective(60.F, 0.3F, 10000.F);
        cam.addComponent<SkyboxRenderer>(skyboxTex);
        cam.addComponent<GridRenderer>();
        cam.addComponent<LightingRenderComponent>();
        ShadowRendererConfig shadowConfig;
        shadowConfig.cascadeAmount = 3;
        cam.addComponent<ShadowRenderer>(shadowConfig);
        cam.addComponent<ForwardRenderer>();
        cam.addComponent<FrustumCuller>();

        if (_sceneBuffer)
        {
            auto& size = _sceneBuffer->getSize();
            cam.getRenderChain().setOutput(_sceneBuffer);
            cam.setBaseViewport(Viewport(size));
            _app.requestRenderReset();
        }
        else
        {
            cam.setEnabled(false);
        }
        _editorCam = cam;
        return *this;
    }

    void EditorSceneView::updateSize(const glm::uvec2& size) noexcept
    {
        if (size.x <= 0.F || size.y <= 0.F)
        {
            return;
        }
        if (_sceneBuffer && size == _sceneBuffer->getSize())
        {
            return;
        }
        _sceneBuffer = std::make_shared<FrameBuffer>(size);
        _editorCam->getRenderChain().setOutput(_sceneBuffer);
        _editorCam->setEnabled(true);
        _editorCam->setBaseViewport(Viewport(size));
        // TODO: maby a bit harsh
        _app.requestRenderReset();
    }

    void EditorSceneView::renderGizmos()
    {
        if (!_editorCam  || !_scene)
        {
            return;
        }

        ImVec2 min = ImGui::GetItemRectMin();
        ImVec2 size = ImGui::GetItemRectSize();
        ImGuizmo::SetRect(min.x, min.y, size.x, size.y);
        ImGuizmo::SetGizmoSizeClipSpace(0.2F);
        ImGuizmo::Enable(true);

        auto view = _editorCam->getViewMatrix();
        auto viewPos = min;
        ImVec2 viewSize(100, 100);
        viewPos.x += size.x - viewSize.x;
        ImGuizmo::ViewManipulate(glm::value_ptr(view), 100.0F, viewPos, viewSize, 0x101010DD);

        if (_selectedEntity == entt::null)
        {
            return;
        }

        auto trans = _scene->getComponent<Transform>(_selectedEntity);
        if (!trans)
        {
            return;
        }

        auto worldPos = trans->getWorldPosition();
        if (!_editorCam->isWorldPointVisible(worldPos))
        {
            return;
        }

        auto proj = _editorCam->getProjectionMatrix();
        auto op = ImGuizmo::TRANSLATE;
        switch (_transGizmoMode)
        {
        case TransformGizmoMode::Rotate:
            op = ImGuizmo::ROTATE;
            break;
        case TransformGizmoMode::Scale:
            op = ImGuizmo::SCALE;
            break;
        }

        auto mode = ImGuizmo::LOCAL;
        auto mtx = trans->getLocalMatrix();

        if (ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), op, mode, glm::value_ptr(mtx)))
        {
            trans->setLocalMatrix(mtx);
        }
    }

    void EditorSceneView::updateInputEvents(float deltaTime)
    {
        static const InputEvents gizmoModeGrabEvents = {
            KeyboardInputEvent{ KeyboardKey::KeyW }
        };
        static const InputEvents gizmoModeTranslateEvents = {
            KeyboardInputEvent{ KeyboardKey::KeyW }
        };
        static const InputEvents gizmoModeRotateEvents = {
            KeyboardInputEvent{ KeyboardKey::KeyE }
        };
        static const InputEvents gizmoModeScaleEvents = {
            KeyboardInputEvent{ KeyboardKey::KeyR }
        };

        auto& input = _app.getInput();
        if (input.checkEvents(gizmoModeGrabEvents))
        {
            _transGizmoMode = TransformGizmoMode::Grab;
        }
        else if (input.checkEvents(gizmoModeTranslateEvents))
        {
            _transGizmoMode = TransformGizmoMode::Translate;
        }
        else if (input.checkEvents(gizmoModeRotateEvents))
        {
            _transGizmoMode = TransformGizmoMode::Rotate;
        }
        else if (input.checkEvents(gizmoModeScaleEvents))
        {
            _transGizmoMode = TransformGizmoMode::Scale;
        }
    }
    
    void EditorSceneView::updateCamera(float deltaTime)
    {
        auto trans = _editorCam->getTransform();
        if (!trans)
        {
            return;
        }

        auto& input = _app.getInput();

        // TODO: could be configured
        static const std::array<InputAxis, 2> lookAxis = {
            InputAxis{
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Left } },
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Right } }
            },
            InputAxis{
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Down } },
                { MouseInputDir{ MouseAnalog::Position, InputDirType::Up } }
            }
        };
        static const std::array<InputAxis, 2> moveAxis = {
            InputAxis{
                { KeyboardInputEvent{ KeyboardKey::Left } },
                { KeyboardInputEvent{ KeyboardKey::Right } }
            },
            InputAxis{
                { KeyboardInputEvent{ KeyboardKey::Down }, MouseInputDir{ MouseAnalog::Scroll, InputDirType::Up } },
                { KeyboardInputEvent{ KeyboardKey::Up }, MouseInputDir{ MouseAnalog::Scroll, InputDirType::Down } }
            }
        };
        static const float lookSensitivity = 2.F;
        static const float dragSensitivity = 0.2F;
        static const float moveSensitivity = 0.05F;

        auto rot = trans->getRotation();

        glm::vec2 look;
        input.getAxis(look, lookAxis);
        look.y = Math::clamp(look.y, -90.F, 90.F);
        look = glm::radians(look) * lookSensitivity;

        if (_mouseMode == MouseSceneViewMode::Look)
        {
            rot = glm::quat(glm::vec3(0, look.x, 0)) * rot * glm::quat(glm::vec3(look.y, 0, 0));
            trans->setRotation(rot);
        }
        if (_mouseMode == MouseSceneViewMode::Drag)
        {
            glm::vec2 drag;
            input.getAxis(drag, lookAxis);
            drag *= dragSensitivity;
            auto dir = rot * glm::vec3(-drag.x, -drag.y, 0.F);
            trans->setPosition(trans->getPosition() + dir);
        }

        if (_focused)
        {
            glm::vec2 move;
            input.getAxis(move, moveAxis);
            move *= moveSensitivity;
            auto dir = rot * glm::vec3(move.x, 0.F, move.y);
            trans->setPosition(trans->getPosition() + dir);
        }
    }

    void EditorSceneView::update(float deltaTime)
    {
        updateCamera(deltaTime);
        updateInputEvents(deltaTime);
    }

    void EditorSceneView::render()
    {
        if (ImGui::Begin(_windowName.c_str()))
        {
            auto size = ImGui::GetContentRegionAvail();
            updateSize(glm::uvec2(size.x, size.y));

            if (_sceneBuffer)
            {
                ImguiTextureData texData(_sceneBuffer->getTexture()->getHandle());
                ImGui::Image(texData, size);
                renderGizmos();
            }

            _focused = ImGui::IsWindowFocused();
            if (ImGui::IsWindowHovered())
            {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left) && _transGizmoMode == TransformGizmoMode::Grab)
                {
                    _mouseMode = MouseSceneViewMode::Drag;
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                {
                    _mouseMode = MouseSceneViewMode::Look;
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                {
                    _mouseMode = MouseSceneViewMode::Drag;
                }
                else
                {
                    _mouseMode = MouseSceneViewMode::None;
                }
            }
            else
            {
                _mouseMode = MouseSceneViewMode::None;
            }
        }
        ImGui::End();
    }

}