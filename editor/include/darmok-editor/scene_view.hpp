#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/scene.hpp>

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

    class BX_NO_VTABLE ISceneGizmo
    {
    public:
        virtual ~ISceneGizmo() = default;
        virtual expected<void, std::string> init(Camera& cam, Scene& scene, EditorApp& app) noexcept { return {}; }
        virtual expected<void, std::string> shutdown() noexcept { return {}; }
        virtual expected<void, std::string> update(float deltaTime) noexcept { return {}; }
        virtual expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept { return {}; }
    };

    class TransformGizmo final : public ISceneGizmo
    {
    public:
        using Mode = SceneViewTransformMode;

        expected<void, std::string> init(Camera& cam, Scene& scene, EditorApp& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;

        Mode getMode() const noexcept;
        void setMode(Mode mode) noexcept;

    private:
        Mode _mode;
        OptionalRef<EditorApp> _app;
        OptionalRef<Scene> _scene;
        OptionalRef<Camera> _cam;
        std::unique_ptr<Mesh> _arrowMesh;
        std::unique_ptr<Mesh> _rotMesh;
        std::unique_ptr<Mesh> _scaleMesh;
        std::unique_ptr<Material> _redMat;
        std::unique_ptr<Material> _greenMat;
        std::unique_ptr<Material> _blueMat;

        expected<void, std::string> render(Entity entity, bgfx::ViewId viewId, bgfx::Encoder& encoder, const std::unique_ptr<Mesh>& mesh, const std::unique_ptr<Material>& material, std::optional<glm::mat4> transform = std::nullopt) noexcept;
    };

    class EditorSceneGizmosRenderer final : public ITypeCameraComponent<EditorSceneGizmosRenderer>
    {
    public:
        EditorSceneGizmosRenderer(EditorApp& app) noexcept;
        expected<void, std::string> init(Camera& cam, Scene& scene, App& app) noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<bgfx::ViewId, std::string> renderReset(bgfx::ViewId viewId) noexcept override;
        expected<void, std::string> render() noexcept override;

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
    private:
        EditorApp& _app;
        OptionalRef<Camera> _cam;
        OptionalRef<Scene> _scene;
        std::optional<bgfx::ViewId> _viewId;
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