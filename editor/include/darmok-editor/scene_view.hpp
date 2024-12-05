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
        EditorSceneView(App& app);
        void init(const std::shared_ptr<Scene>& scene, Camera& cam);
        void shutdown();
        void beforeRender();
        void render();
        void update(float deltaTime);

        TransformGizmoMode getTransformGizmoMode() const;
        EditorSceneView& setTransformGizmoMode(TransformGizmoMode mode);
        EditorSceneView& selectEntity(Entity entity);

        MouseSceneViewMode getMouseMode() const;
        static const std::string& getWindowName();

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

        void updateSize(const glm::uvec2& size) noexcept;
        void updateInputEvents(float deltaTime);
        void updateCamera(float deltaTime);
        void renderGizmos();
        bool isEditorEntity(Entity entity) const noexcept;

    };
}