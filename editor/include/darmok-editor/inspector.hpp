#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/optional_ref.hpp>

#include <memory>

namespace darmok
{
    class Scene;
}

namespace darmok::editor
{
    class SceneInspectorEditor;

    class EditorInspectorView final
    {
    public:
        EditorInspectorView();
        void setup();
        void init(const std::shared_ptr<Scene>& scene);
        void shutdown();
        void render();

        EditorInspectorView& selectEntity(Entity entity);

        static const std::string& getWindowName();
    private:
        OptionalRef<SceneInspectorEditor> _sceneEditor;
        std::shared_ptr<Scene> _scene;
        ObjectEditorContainer _editors;
        static const std::string _windowName;
        Entity _selectedEntity;
    };
}