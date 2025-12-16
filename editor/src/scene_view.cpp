#include <darmok-editor/scene_view.hpp>
#include <darmok-editor/app.hpp>
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

#include <imgui.h>

namespace darmok::editor
{
    expected<void, std::string> TransformGizmo::init(Camera& cam, Scene& scene, EditorApp& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        _app = app;

        auto progResult = StandardProgramLoader::load(Program::Standard::Unlit);
        if (!progResult)
        {
            return unexpected{ std::move(progResult).error() };
        }
        auto prog = progResult.value();
        auto meshResult = MeshData{ Line{}, Mesh::Definition::Arrow }.createMesh(prog->getVertexLayout());
        if (!meshResult)
        {
            return unexpected{ std::move(meshResult).error() };
        }

        auto createMaterial = [prog](const Color& c)
        {
            auto mat = std::make_unique<Material>(prog, c);
            mat->depthTest = Material::Definition::DepthNoTest;
            mat->writeDepth = false;
            // mat->twoSided = true;
            return mat;
        };

        _arrowMesh = std::make_unique<Mesh>(std::move(meshResult).value());
        _redMat = createMaterial(Colors::red());
        _greenMat = createMaterial(Colors::green());
        _blueMat = createMaterial(Colors::blue());

        return {};
    }

    expected<void, std::string> TransformGizmo::shutdown() noexcept
    {
        _arrowMesh.reset();
        _cam.reset();
        _scene.reset();
        _app.reset();

        return {};
    }

    expected<void, std::string> TransformGizmo::update(float deltaTime) noexcept
    {
        if (!_app)
        {
            return unexpected<std::string>{"uninitialized app"};
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

        auto& input = _app->getInput();
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

        return {};
    }

    expected<void, std::string> TransformGizmo::render(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder, const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Material>& material, std::optional<glm::mat4> transform) noexcept
    {
        if (!_cam)
        {
            return unexpected<std::string>{"missing camera"};
        }
        if (!mesh)
        {
            return unexpected<std::string>{"missing mesh"};
        }
        if (!material)
        {
            return unexpected<std::string>{"missing material"};
        }

        _cam->setEntityTransform(entity, encoder, transform);
        auto result = mesh->render(encoder);
        if (!result)
        {
            return result;
        }
        material->renderSubmit(viewId, encoder);
        return {};
    }

    expected<void, std::string> TransformGizmo::render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept
    {
        if (!_scene || !_app || !_cam)
        {
            return {};
        }
        auto entityId = _app->getSelectedEntity();
        auto entity = _app->getProject().getComponentLoadContext().getEntity(entityId);
        if (entity == entt::null)
        {
            return {};
        }

        _cam->setViewTransform(viewId);

        auto result = render(entity, viewId, encoder, _arrowMesh, _greenMat);
        if (!result)
        {
            return result;
        }
        glm::quat rot{ glm::radians(glm::vec3{-90, 0, -90}) };
        result = render(entity, viewId, encoder, _arrowMesh, _redMat, glm::mat4_cast(rot));
        if (!result)
        {
            return result;
        }
        rot = glm::quat{ glm::radians(glm::vec3{90, 90, 0}) };
        result = render(entity, viewId, encoder, _arrowMesh, _blueMat, glm::mat4_cast(rot));
        if (!result)
        {
            return result;
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

    EditorSceneGizmosRenderer::EditorSceneGizmosRenderer(EditorApp& app) noexcept
        : _app{ app }
    {
    }

    expected<void, std::string> EditorSceneGizmosRenderer::init(Camera& cam, Scene& scene, App& app) noexcept
    {
        _cam = cam;
        _scene = scene;
        std::vector<std::string> errors;
        for (auto& gizmo : _gizmos)
        {
            auto result = gizmo->init(cam, scene, _app);
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        return StringUtils::joinExpectedErrors(errors);
    }

    expected<void, std::string> EditorSceneGizmosRenderer::update(float deltaTime) noexcept
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
    
    expected<void, std::string> EditorSceneGizmosRenderer::shutdown() noexcept
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

    expected<bgfx::ViewId, std::string> EditorSceneGizmosRenderer::renderReset(bgfx::ViewId viewId) noexcept
    {
        _viewId = viewId;
        _cam->configureView(viewId, "Editor Gizmos");
        // bgfx::setViewMode(viewId, bgfx::ViewMode::Sequential);
        return ++viewId;
    }

    expected<void, std::string> EditorSceneGizmosRenderer::render() noexcept
    {
        if (!_viewId)
        {
            return unexpected<std::string>{"view not configured"};
        }
        std::vector<std::string> errors;
        auto encoder = bgfx::begin();
        for (auto& gizmo : _gizmos)
        {
            auto result = gizmo->render(*_viewId, *encoder);
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
        }
        bgfx::end(encoder);
        return StringUtils::joinExpectedErrors(errors);
    }


    expected<void, std::string> EditorSceneGizmosRenderer::add(std::unique_ptr<ISceneGizmo> gizmo) noexcept
    {
        if (_cam && _scene)
        {
            auto result = gizmo->init(*_cam, *_scene, _app);
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
        {
            auto gizmosResult = cam.addComponent<EditorSceneGizmosRenderer>(_app);
            if (!gizmosResult)
            {
                return unexpected{ std::move(gizmosResult).error() };
            }
            auto result = gizmosResult.value().get().add<TransformGizmo>();
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            _transformGizmo = result.value().get();
        }

        _scene = scene;
        _cam = cam;

        glm::uvec2 size{ 0 };
        if (_sceneBuffer)
        {
            size = _sceneBuffer->getSize();
        }
        auto result = updateSize(size);
        if (!result)
        {
            return result;
        }
        
        return {};
    }
    
    expected<void, std::string> EditorSceneView::shutdown() noexcept
    {
        _scene.reset();
        _sceneBuffer.reset();
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
        if (_sceneBuffer && size == _sceneBuffer->getSize())
        {
            return {};
        }
        if (size.x <= 0.F || size.y <= 0.F)
        {
            _sceneBuffer.reset();
            _cam->setEnabled(false);
        }
        else
        {
			auto fbResult = FrameBuffer::load(size);
			if (!fbResult)
            {
                return unexpected{ std::move(fbResult).error() };
            }
            _sceneBuffer = std::make_shared<FrameBuffer>(std::move(fbResult).value());
            auto result = _cam->getRenderChain().setOutput(_sceneBuffer);
            if (!result)
            {
                return result;
            }
            _cam->setEnabled(true);
            _cam->setBaseViewport(Viewport{ size });
        }
        // TODO: maybe a bit harsh
        _app.requestRenderReset();
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
                { Mouse::createInputDir(Input::Definition::LeftDir) },
                { Mouse::createInputDir(Input::Definition::RightDir) },
            },
            InputAxis{
                { Mouse::createInputDir(Input::Definition::DownDir) },
                { Mouse::createInputDir(Input::Definition::UpDir) },
            }
        };
        static const std::array<InputAxis, 2> moveAxis = {
            InputAxis{
                { Keyboard::createInputDir(Keyboard::Definition::KeyLeft) },
                { Keyboard::createInputDir(Keyboard::Definition::KeyRight) },
            },
            InputAxis{
                { Keyboard::createInputDir(Keyboard::Definition::KeyDown), Mouse::createInputDir(Input::Definition::UpDir, Mouse::Definition::ScrollAnalog) },
                { Keyboard::createInputDir(Keyboard::Definition::KeyUp), Mouse::createInputDir(Input::Definition::DownDir, Mouse::Definition::ScrollAnalog) }
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
            auto size = ImGui::GetContentRegionAvail();
            auto sizeResult = updateSize(glm::uvec2{ size.x, size.y });
            if (!sizeResult)
            {
                return sizeResult;
            }

            if (_sceneBuffer)
            {
                ImguiTextureData texData{ _sceneBuffer->getTexture()->getHandle() };
                ImGui::Image(texData, size);
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