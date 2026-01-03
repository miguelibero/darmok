#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene.hpp>
#include <darmok/material.hpp>

#include <memory>
#include <string>


namespace darmok
{
    class Mesh;
    class FrameBuffer;
}

namespace darmok::editor
{
    class EditorApp;

    enum class SceneViewMouseMode
    {
        None,
        Look,
        Drag
    };

    enum class SceneViewTransformMode
    {
        Grab,
        Translate,
        Rotate,
        Scale
    };

    class SceneGizmosRenderer;

    class BX_NO_VTABLE ISceneGizmo
    {
    public:
        virtual ~ISceneGizmo() = default;
        virtual expected<void, std::string> init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept { return {}; }
        virtual expected<void, std::string> shutdown() noexcept { return {}; }
        virtual expected<void, std::string> update(float deltaTime) noexcept { return {}; }
        virtual expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept { return {}; }
    };

    class TransformGizmo final : public ISceneGizmo
    {
    public:
        using Mode = SceneViewTransformMode;

        expected<void, std::string> init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;

        Mode getMode() const noexcept;
        void setMode(Mode mode) noexcept;

    private:
        Mode _mode;
        OptionalRef<SceneGizmosRenderer> _renderer;
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        std::unique_ptr<Mesh> _transMesh;
        std::unique_ptr<Mesh> _rotMesh;
        std::unique_ptr<Mesh> _scaleMesh;
        std::optional<glm::vec2> _lastMousePosition;
    };

    class CameraGizmo final : public ISceneGizmo
    {
    public:
        expected<void, std::string> init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
    private:
        OptionalRef<SceneGizmosRenderer> _renderer;
        OptionalRef<Scene> _scene;
    };

    class Physics3dShapeGizmo final : public ISceneGizmo
    {
    public:
        expected<void, std::string> init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
    private:
        OptionalRef<SceneGizmosRenderer> _renderer;
        OptionalRef<Scene> _scene;
    };

    class SceneGizmosRenderer final : public ITypeCameraComponent<SceneGizmosRenderer>
    {
    public:
        SceneGizmosRenderer(EditorApp& app) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> render() noexcept override;

        EditorApp& getEditorApp() noexcept;
        const EditorApp& getEditorApp() const noexcept;

        expected<void, std::string> add(std::unique_ptr<ISceneGizmo> gizmo) noexcept;

        template<typename T, typename... A>
        expected<std::reference_wrapper<T>, std::string> add(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            auto result = add(std::move(ptr));
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            return std::ref(ref);
        }

        const bgfx::VertexLayout& getVertexLayout() const noexcept;
        expected<void, std::string> renderMesh(Entity entity, const Mesh& mesh, const Material& material, std::optional<glm::mat4> transform = std::nullopt) noexcept;
        expected<void, std::string> renderMesh(Entity entity, const Mesh& mesh, const Color& color, std::optional<glm::mat4> transform = std::nullopt) noexcept;
        expected<void, std::string> renderMesh(Entity entity, const Mesh& mesh, const Color& color, Material::PrimitiveType prim, std::optional<glm::mat4> transform = std::nullopt) noexcept;
        Entity getSelectedEntity() const noexcept;

    private:
        EditorApp& _app;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        std::shared_ptr<Program> _program;
        std::optional<bgfx::ViewId> _viewId;
        OptionalRef<bgfx::Encoder> _encoder;
        std::vector<std::unique_ptr<ISceneGizmo>> _gizmos;
    };
        
    class EditorSceneView final
    {
    public:
        using TransformMode = SceneViewTransformMode;
        using MouseMode = SceneViewMouseMode;

        EditorSceneView(EditorApp& app) noexcept;
        expected<void, std::string> init(std::shared_ptr<Scene> scene, Camera& cam) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<void, std::string> beforeRender() noexcept;
        expected<void, std::string> render() noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;

        EditorSceneView& selectEntity(Entity entity) noexcept;
        bool focusEntity(Entity entity = entt::null) noexcept;

        MouseMode getMouseMode() const noexcept;
        static const std::string& getWindowName() noexcept;

        TransformMode getTransformMode() const noexcept;
        EditorSceneView& setTransformMode(TransformMode mode) noexcept;

    private:
        static const std::string _windowName;
        EditorApp& _app;
        std::shared_ptr<Scene> _scene;
        OptionalRef<Camera> _cam;
        MouseMode _mouseMode;
        bool _focused;
        std::shared_ptr<FrameBuffer> _sceneBuffer;
        Entity _selectedEntity;
        OptionalRef<TransformGizmo> _transformGizmo;

        expected<void, std::string> updateSize(const glm::uvec2& size) noexcept;
        void updateCamera(float deltaTime) noexcept;
       
    };
}