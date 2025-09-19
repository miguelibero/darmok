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
        using RenderResult = expected<bool, std::string>;

        EditorInspectorView();
        void setup();
        void init(EditorApp& app);
        void shutdown();
        RenderResult render();

        void selectObject(const SelectableObject& obj);

		bool isSceneSelected() const noexcept;
        EntityId getSelectedEntity() const noexcept;
		std::optional<std::filesystem::path> getSelectedAssetPath() const noexcept;

        static const std::string& getWindowName();
    private:
        ObjectEditorContainer _editors;
        static const std::string _windowName;
        SelectableObject _selected;
        OptionalRef<SceneDefinitionWrapper> _sceneDef;

        RenderResult renderEntity(EntityId entity);
        RenderResult renderAsset(std::filesystem::path path);
    };
}