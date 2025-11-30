#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>

#include <memory>
#include <string>


namespace darmok
{
    class App;
    class Scene;
    class Camera;
    class FrameBuffer;
}

namespace darmok::editor
{
    enum class MouseSceneViewMode
    {
        None,
        Look,
        Drag
    };

    enum class TransformGizmoMode
    {
        Grab,
        Translate,
        Rotate,
        Scale
    };

    class EditorSceneView final
    {
    public:
        EditorSceneView(App& app) noexcept;
        expected<void, std::string> init(const std::shared_ptr<Scene>& scene, Camera& cam) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<void, std::string> beforeRender() noexcept;
        expected<void, std::string> render() noexcept;
        expected<void, std::string> update(float deltaTime) noexcept;

        TransformGizmoMode getTransformGizmoMode() const noexcept;
        EditorSceneView& setTransformGizmoMode(TransformGizmoMode mode) noexcept;
        EditorSceneView& selectEntity(Entity entity) noexcept;

        MouseSceneViewMode getMouseMode() const noexcept;
        static const std::string& getWindowName() noexcept;

    private:
        static const std::string _windowName;
        App& _app;
        std::shared_ptr<Scene> _scene;
        OptionalRef<Camera> _cam;
        MouseSceneViewMode _mouseMode;
        TransformGizmoMode _transGizmoMode;
        bool _focused;
        std::shared_ptr<FrameBuffer> _sceneBuffer;
        Entity _selectedEntity;

        expected<void, std::string> updateSize(const glm::uvec2& size) noexcept;
        void updateInputEvents(float deltaTime) noexcept;
        void updateCamera(float deltaTime) noexcept;
        void renderGizmos() noexcept;
    };
}