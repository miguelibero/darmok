#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok-editor/asset_fwd.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/optional_ref.hpp>

#include <memory>
#include <variant>

namespace darmok
{
    class Scene;
    class AssetContext;
}

namespace darmok::editor
{
    class EditorProject;
    class SceneInspectorEditor;
    class MaterialInspectorEditor;
    class ProgramInspectorEditor;

    class EditorInspectorView final
    {
    public:

        EditorInspectorView();
        void setup();
        void init(AssetContext& assets, EditorProject& proj);
        void shutdown();
        void render();

        void selectObject(const SelectableObject& obj, const std::shared_ptr<Scene>& scene = nullptr);

        std::shared_ptr<Scene> getSelectedScene() const noexcept;
        Entity getSelectedEntity() const noexcept;

        template<typename T>
        std::optional<T> getSelectedObject() const noexcept
        {
            auto ptr = std::get_if<T>(&_selected);
            if (ptr == nullptr)
            {
                return std::nullopt;
            }
            return *ptr;
        }

        static const std::string& getWindowName();
    private:
        OptionalRef<SceneInspectorEditor> _sceneEditor;
        OptionalRef<MaterialInspectorEditor> _materialEditor;
        OptionalRef<ProgramInspectorEditor> _programEditor;
        ObjectEditorContainer _editors;
        static const std::string _windowName;
        SelectableObject _selected;
        std::shared_ptr<Scene> _scene;
    };
}