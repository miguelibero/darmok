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
    class EditorApp;
    class SceneInspectorEditor;
    class MaterialInspectorEditor;
    class ProgramSourceInspectorEditor;
    class MeshSourceInspectorEditor;
    class TextureDefinitionInspectorEditor;
	class ModelInspectorEditor;

    class EditorInspectorView final
    {
    public:

        EditorInspectorView();
        void setup();
        void init(EditorApp& app);
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
        OptionalRef<ProgramSourceInspectorEditor> _programEditor;
        OptionalRef<MeshSourceInspectorEditor> _meshEditor;
        OptionalRef<TextureDefinitionInspectorEditor> _textureEditor;
        OptionalRef<ModelInspectorEditor> _modelEditor;
        ObjectEditorContainer _editors;
        static const std::string _windowName;
        SelectableObject _selected;
        std::shared_ptr<Scene> _scene;


        template<typename T, typename E>
        bool renderSelectedSharedAsset(OptionalRef<E> editor) noexcept
        {
            if (!editor)
            {
                return false;
            }
            if (auto opt = getSelectedObject<T>())
            {
                if (auto asset = *opt)
                {
                    editor->renderType(*asset);
                    return true;
                }
            }
            return false;
        }
    };
}