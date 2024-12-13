#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/optional_ref.hpp>

#include <memory>
#include <variant>

namespace darmok
{
    class Scene;
    class Material;
}

namespace darmok::editor
{
    class SceneInspectorEditor;
    class MaterialInspectorEditor;

    class EditorInspectorView final
    {
    public:
        using SelectedObject = std::variant<Entity, std::shared_ptr<Material>>;

        EditorInspectorView();
        void setup();
        void init();
        void shutdown();
        void render();

        EditorInspectorView& selectObject(SelectedObject obj, const std::shared_ptr<Scene>& scene);

        std::shared_ptr<Scene> getSelectedScene() const noexcept;
        std::shared_ptr<Material> getSelectedMaterial() const noexcept;
        Entity getSelectedEntity() const noexcept;

        static const std::string& getWindowName();
    private:
        OptionalRef<SceneInspectorEditor> _sceneEditor;
        OptionalRef<MaterialInspectorEditor> _materialEditor;
        ObjectEditorContainer _editors;
        static const std::string _windowName;
        SelectedObject _selected;
        std::shared_ptr<Scene> _scene;
    };
}