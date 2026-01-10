#include <darmok-editor/scene_view.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/app.hpp>
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
#include <darmok/shape.hpp>
#include <darmok/mesh.hpp>
#include <darmok/glm.hpp>
#include <darmok/math.hpp>

#include <imgui.h>

namespace darmok::editor
{
    expected<void, std::string> TransformGizmo::init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept
    {
        _lastMousePosition.reset();
        _cam = cam;
        _scene = scene;
        _renderer = renderer;

        auto progResult = StandardProgramLoader::load(Program::Standard::Unlit);
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        auto prog = progResult.value();
        auto& layout = prog->getVertexLayout();

        {
            auto meshResult = MeshData{ Line{}, Mesh::Definition::Arrow }.createMesh(layout);
            if (!meshResult)
            {
                return unexpected{ std::move(meshResult).error() };
            }
            _transMesh = std::make_unique<Mesh>(std::move(meshResult).value());
        }
        {
            MeshData meshData{ Circle{ 1.f }, Mesh::Definition::FillOutline };
            auto meshResult = meshData.createMesh(layout);
            if (!meshResult)
            {
                return unexpected{ std::move(meshResult).error() };
            }
            _rotMesh = std::make_unique<Mesh>(std::move(meshResult).value());
        }
        {
            MeshData meshData{ Cube{ glm::vec3{ 0.1f }, glm::vec3{ 0.f, 1.f, 0.f } } };
            meshData += MeshData{ Cylinder{ 1.f, 0.01f, glm::vec3{ 0.f, .5f, 0.f } } };
            auto meshResult = meshData.createMesh(layout);
            if (!meshResult)
            {
                return unexpected{ std::move(meshResult).error() };
            }
            _scaleMesh = std::make_unique<Mesh>(std::move(meshResult).value());
        }

        return {};
    }

    expected<void, std::string> TransformGizmo::shutdown() noexcept
    {
        _transMesh.reset();
        _rotMesh.reset();
        _scaleMesh.reset();
        _cam.reset();
        _scene.reset();
        _renderer.reset();
        return {};
    }

    expected<void, std::string> TransformGizmo::update(float deltaTime) noexcept
    {
        if (!_renderer)
        {
            return unexpected<std::string>{"uninitialized renderer"};
        }
        if (!_scene)
        {
            return unexpected<std::string>{"uninitialized scene"};
        }
        auto trans = _scene->getComponent<Transform>(_renderer->getSelectedEntity());
        if (!trans)
        {
            _mode = Mode::Grab;
            return {};
        }

        static const InputEvents grabEvents = {
            Keyboard::createInputEvent(Keyboard::Definition::KeyW)
        };
        static const InputEvents translateEvents = {
            Keyboard::createInputEvent(Keyboard::Definition::KeyW)
        };
        static const InputEvents rotateEvents = {
            Keyboard::createInputEvent(Keyboard::Definition::KeyE)
        };
        static const InputEvents scaleEvents = {
            Keyboard::createInputEvent(Keyboard::Definition::KeyR)
        };

        auto& input = _renderer->getEditorApp().getInput();
        if (input.checkEvents(grabEvents))
        {
            _mode = Mode::Grab;
        }
        else if (input.checkEvents(translateEvents))
        {
            _mode = Mode::Translate;
        }
        else if (input.checkEvents(rotateEvents))
        {
            _mode = Mode::Rotate;
        }
        else if (input.checkEvents(scaleEvents))
        {
            _mode = Mode::Scale;
        }

        auto inProgress = input.getMouse().getButton(Mouse::Definition::ButtonLeft);
        if (inProgress)
        {
            auto pos = input.getMouse().getPosition();
            if (_lastMousePosition)
            {
                auto diff = *_lastMousePosition - pos;
            }
            _lastMousePosition = pos;
        }

        return {};
    }

    expected<void, std::string> TransformGizmo::render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_scene || !_cam || !_renderer)
        {
            return {};
        }
        auto entity = _renderer->getSelectedEntity();
        if (entity == entt::null)
        {
            return {};
        }

        auto renderAxes = [this, entity](const std::unique_ptr<Mesh>& mesh) -> expected<void, std::string>
        {
            if (!mesh)
            {
                return unexpected<std::string>{"missing mesh"};
            }
            auto result = _renderer->renderMesh(entity, *mesh, Colors::green());
            if (!result)
            {
                return result;
            }
            glm::quat rot{ glm::radians(glm::vec3{-90, 0, -90}) };
            result = _renderer->renderMesh(entity, *mesh, Colors::red(), glm::mat4_cast(rot));
            if (!result)
            {
                return result;
            }
            rot = glm::quat{ glm::radians(glm::vec3{90, 90, 0}) };
            return _renderer->renderMesh(entity, *mesh, Colors::blue(), glm::mat4_cast(rot));
        };

        auto renderPlanes = [this, entity](const std::unique_ptr<Mesh>& mesh) -> expected<void, std::string>
        {
            if (!mesh)
            {
                return unexpected<std::string>{"missing mesh"};
            }
            auto result = _renderer->renderMesh(entity, *mesh, Colors::red(), Material::Definition::Line);
            if (!result)
            {
                return result;
            }
            glm::quat rot{ glm::radians(glm::vec3{0, 90, 0}) };
            result = _renderer->renderMesh(entity, *mesh, Colors::blue(), Material::Definition::Line, glm::mat4_cast(rot));
            if (!result)
            {
                return result;
            }
            rot = glm::quat{ glm::radians(glm::vec3{90, 0, 0}) };
            return _renderer->renderMesh(entity, *mesh, Colors::green(), Material::Definition::Line, glm::mat4_cast(rot));
        };

        switch (_mode)
        {
        case Mode::Translate:
            return renderAxes(_transMesh);
        case Mode::Rotate:
            return renderPlanes(_rotMesh);
        case Mode::Scale:
            return renderAxes(_scaleMesh);
        }
       
        return {};
    }

    TransformGizmo::Mode TransformGizmo::getMode() const noexcept
    {
        return _mode;
    }

    void TransformGizmo::setMode(Mode mode) noexcept
    {
        _mode = mode;
    }

    expected<void, std::string> CameraGizmo::init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept
    {
        _scene = scene;
        _renderer = renderer;
        return {};
    }

    expected<void, std::string> CameraGizmo::shutdown() noexcept
    {
        _scene.reset();
        _renderer.reset();
        return {};
    }

    expected<void, std::string> CameraGizmo::render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_renderer)
        {
            return unexpected<std::string>{"renderer not initialized"};
        }
        if (!_scene)
        {
            return unexpected<std::string>{"scene not initialized"};
        }
        auto& layout = _renderer->getVertexLayout();
        if (layout.getStride() == 0)
        {
            return unexpected<std::string>{"empty vertex layout"};
        }
        auto entity = _renderer->getSelectedEntity();
        auto cam = _scene->getComponent<Camera>(entity);
        if (!cam)
        {
            return {};
        }
        MeshData meshData{ Frustum{ cam->getProjectionMatrix() }, Mesh::Definition::FillOutline };
        auto meshResult = meshData.createMesh(layout, { .type = Mesh::Definition::Transient });
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }
        return _renderer->renderMesh(entity, meshResult.value(), Colors::red(), Material::Definition::Line);
    }   

    SceneGizmosRenderer::SceneGizmosRenderer(EditorApp& app) noexcept
        : _app{ app }
    {
    }

    expected<void, std::string> SceneGizmosRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;

        auto progResult = StandardProgramLoader::load(Program::Standard::Unlit);
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        _program = progResult.value();

        std::vector<std::string> errors;
        for (auto& gizmo : _gizmos)
        {
            auto result = gizmo->init(cam, scene, *this);
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        return StringUtils::joinExpectedErrors(errors);
    }

    expected<void, std::string> SceneGizmosRenderer::update(float deltaTime) noexcept
    {
        std::vector<std::string> errors;
        for (auto& gizmo : _gizmos)
        {
            auto result = gizmo->update(deltaTime);
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        return StringUtils::joinExpectedErrors(errors);
    }
    
    expected<void, std::string> SceneGizmosRenderer::shutdown() noexcept
    {
        std::vector<std::string> errors;
        for (auto& gizmo : _gizmos)
        {
            auto result = gizmo->shutdown();
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        _cam.reset();
        _scene.reset();
        return StringUtils::joinExpectedErrors(errors);
    }

    expected<bgfx::ViewId, std::string> SceneGizmosRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        _cam->configureView(viewId, "Editor Gizmos");
        // bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
        return ++viewId;
    }

    expected<void, std::string> SceneGizmosRenderer::render() noexcept
    {
        if (!_viewId)
        {
            return unexpected<std::string>{"view not configured"};
        }
        std::vector<std::string> errors;
        _encoder = bgfx::begin();
        if (_cam)
        {
            _cam->setViewTransform(*_viewId);
        }
        for (auto& gizmo : _gizmos)
        {
            auto result = gizmo->render(*_viewId, *_encoder);
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        bgfx::end(_encoder.ptr());
        return StringUtils::joinExpectedErrors(errors);
    }

    EditorApp& SceneGizmosRenderer::getEditorApp() noexcept
    {
        return _app;
    }

    const EditorApp& SceneGizmosRenderer::getEditorApp() const noexcept
    {
        return _app;
    }

    Entity SceneGizmosRenderer::getSelectedEntity() const noexcept
    {
        auto entityId = _app.getSelectedEntity();
        return _app.getProject().getComponentLoadContext().getEntity(entityId);
    }

    const bgfx::VertexLayout& SceneGizmosRenderer::getVertexLayout() const noexcept
    {
        if (!_program)
        {
            static const bgfx::VertexLayout layout;
            return layout;
        }
        return _program->getVertexLayout();
    }

    expected<void, std::string> SceneGizmosRenderer::renderMesh(Entity entity, const Mesh& mesh, const Material& material, std::optional<glm::mat4> transform) noexcept
    {
        _cam->setEntityTransform(entity, *_encoder, transform);
        auto result = mesh.render(*_encoder);
        if (!result)
        {
            return result;
        }
        material.renderSubmit(*_viewId, *_encoder);
        return {};
    }

    expected<void, std::string> SceneGizmosRenderer::renderMesh(Entity entity, const Mesh& mesh, const Color& color, std::optional<glm::mat4> transform) noexcept
    {
        return renderMesh(entity, mesh, color, Material::Definition::Triangle, transform);
    }

    expected<void, std::string> SceneGizmosRenderer::renderMesh(Entity entity, const Mesh& mesh, const Color& color, Material::PrimitiveType prim, std::optional<glm::mat4> transform) noexcept
    {
        if (!_program)
        {
            unexpected<std::string>{"program not loaded"};
        }
        Material material{ _program, color };
        material.depthTest = Material::Definition::DepthNoTest;
        material.writeDepth = false;
        material.primitiveType = prim;
        // material.twoSided = true;
        return renderMesh(entity, mesh, material, transform);
    }

    expected<void, std::string> SceneGizmosRenderer::add(std::unique_ptr<ISceneGizmo> gizmo) noexcept
    {
        if (_cam && _scene)
        {
            auto result = gizmo->init(*_cam, *_scene, *this);
            if (!result)
            {
                return result;
            }
        }
        _gizmos.push_back(std::move(gizmo));
        return {};
    }

    const std::string EditorSceneView::_windowName = "Scene View";

    EditorSceneView::EditorSceneView(EditorApp& app) noexcept
        : _app{ app }
        , _mouseMode{ MouseMode::None }
        , _focused{ false }
        , _selectedEntity{ entt::null }
    {
    }

    expected<void, std::string> EditorSceneView::init(std::shared_ptr<Scene> scene, Camera& cam) noexcept
    {
        DARMOK_TRY_VALUE(_gizmos, cam.addComponent<SceneGizmosRenderer>(_app));
        DARMOK_TRY_VALUE(_transformGizmo, _gizmos->add<TransformGizmo>());
        DARMOK_TRY(_gizmos->add<CameraGizmo>());

        _scene = scene;
        _cam = cam;
        
        return {};
    }

    expected<void, std::string> EditorSceneView::addGizmo(std::unique_ptr<ISceneGizmo> gizmo) noexcept
    {
        if (!_gizmos)
        {
            return unexpected{ "not initialized" };
        }
        return _gizmos->add(std::move(gizmo));
    }
    
    expected<void, std::string> EditorSceneView::shutdown() noexcept
    {
        _gizmos.reset();
        _scene.reset();
        _cam.reset();
        _focused = false;
        _mouseMode = MouseMode::None;
        _transformGizmo.reset();
        return {};
    }

    EditorSceneView& EditorSceneView::selectEntity(Entity entity) noexcept
    {
        _selectedEntity = entity;
        return *this;
    }

    bool EditorSceneView::focusEntity(Entity entity) noexcept
    {
        if (!_scene || !_cam)
        {
            return false;
        }
        entity = entity == entt::null ? _selectedEntity : entity;
        auto trans = _scene->getComponent<Transform>(entity);
        if (!trans)
        {
            return false;
        }
        auto camEntity = _scene->getEntity(*_cam);
        auto camTrans = _scene->getComponent<Transform>(camEntity);
        if (!camTrans)
        {
            return false;
        }

        static constexpr float distance = 3.0f;
        static constexpr float height = 0.5f;
        
        auto pos = trans->getWorldPosition();

        glm::vec3 camPos{ pos - camTrans->getForward() * distance + glm::vec3{ 0, height, 0 } };
        camTrans->setPosition(camPos);
        return true;
    }

    EditorSceneView::MouseMode EditorSceneView::getMouseMode() const noexcept
    {
        return _mouseMode;
    }

    EditorSceneView::TransformMode EditorSceneView::getTransformMode() const noexcept
    {
        if (_transformGizmo)
        {
            return _transformGizmo->getMode();
        }
        return TransformMode::Grab;
    }

    EditorSceneView& EditorSceneView::setTransformMode(TransformMode mode) noexcept
    {
        if (_transformGizmo)
        {
            _transformGizmo->setMode(mode);
        }
        return *this;
    }

    const std::string& EditorSceneView::getWindowName() noexcept
    {
        return _windowName;
    }

    expected<void, std::string> EditorSceneView::updateSize(const glm::uvec2& size) noexcept
    {
        if (!_cam)
        {
            return {};
        }
        auto result = _cam->setRenderOutputSize(size);
        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        if (result.value())
        {
            _app.requestRenderReset();
        }
        return {};
    }

    void EditorSceneView::updateCamera(float deltaTime) noexcept
    {
        auto trans = _cam->getTransform();
        if (!trans)
        {
            return;
        }

        auto& input = _app.getInput();

        // TODO: could be configured
        static const std::array<InputAxis, 2> lookAxis = {
            InputAxis{
                { Mouse::createInputDir(Input::Definition::DirLeft) },
                { Mouse::createInputDir(Input::Definition::DirRight) },
            },
            InputAxis{
                { Mouse::createInputDir(Input::Definition::DirDown) },
                { Mouse::createInputDir(Input::Definition::DirUp) },
            }
        };
        static const std::array<InputAxis, 2> moveAxis = {
            InputAxis{
                { Keyboard::createInputDir(Keyboard::Definition::KeyLeft) },
                { Keyboard::createInputDir(Keyboard::Definition::KeyRight) },
            },
            InputAxis{
                { Keyboard::createInputDir(Keyboard::Definition::KeyDown), Mouse::createInputDir(Input::Definition::DirUp, Mouse::Definition::AnalogScroll) },
                { Keyboard::createInputDir(Keyboard::Definition::KeyUp), Mouse::createInputDir(Input::Definition::DirDown, Mouse::Definition::AnalogScroll) }
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

        if (_mouseMode == MouseMode::Look)
        {
            rot = glm::quat(glm::vec3(0, look.x, 0)) * rot * glm::quat(glm::vec3(look.y, 0, 0));
            trans->setRotation(rot);
        }
        if (_mouseMode == MouseMode::Drag)
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

    expected<void, std::string> EditorSceneView::update(float deltaTime) noexcept
    {
        updateCamera(deltaTime);
        return {};
    }

    expected<void, std::string> EditorSceneView::beforeRender() noexcept
    {
        return {};
    }

    expected<void, std::string> EditorSceneView::render() noexcept
    {
        if (ImGui::Begin(_windowName.c_str()))
        {
            auto size = ImguiUtils::getAvailableContentRegion();
            auto sizeResult = updateSize(size);
            if (!sizeResult)
            {
                return sizeResult;
            }

            if (_cam && _cam->getRenderOutput())
            {
                ImguiUtils::drawBuffer(*_cam->getRenderOutput());
            }

            _focused = ImGui::IsWindowFocused();
            if (ImGui::IsWindowHovered())
            {
                if (ImGui::IsMouseDown(ImGuiMouseButton_Left))
                {
                    _mouseMode = MouseMode::Drag;
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Right))
                {
                    _mouseMode = MouseMode::Look;
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle))
                {
                    _mouseMode = MouseMode::Drag;
                }
                else
                {
                    _mouseMode = MouseMode::None;
                }
            }
            else
            {
                _mouseMode = MouseMode::None;
            }
        }
        ImGui::End();
        return {};
    }
}