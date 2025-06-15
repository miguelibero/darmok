#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok-editor/asset_fwd.hpp>
#include <darmok/optional_ref.hpp>

#include <memory>
#include <variant>

namespace darmok
{
    class SceneDefinitionWrapper;
}

namespace darmok::editor
{
    class EditorApp;

    class EditorInspectorView final
    {
    public:

        EditorInspectorView();
        void setup();
        void init(EditorApp& app);
        void shutdown();
        void render();

        void selectObject(const SelectableObject& obj);

		bool isSceneSelected() const noexcept;
        Entity getSelectedEntity() const noexcept;
		OptionalRef<const SelectedAsset> getSelectedAsset() const noexcept;

        static const std::string& getWindowName();
    private:
        ObjectEditorContainer _editors;
        static const std::string _windowName;
        SelectableObject _selected;
        OptionalRef<SceneDefinitionWrapper> _sceneDef;
    };
}