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

        EditorInspectorView() noexcept;
        expected<void, std::string> setup() noexcept;
        expected<void, std::string> init(EditorApp& app) noexcept;
        expected<void, std::string> shutdown() noexcept;
        RenderResult render() noexcept;

        void selectObject(const SelectableObject& obj) noexcept;

		bool isSceneSelected() const noexcept;
        EntityId getSelectedEntity() const noexcept;
		std::optional<std::filesystem::path> getSelectedAssetPath() const noexcept;

        static const std::string& getWindowName() noexcept;
    private:
        ObjectEditorContainer _editors;
        static const std::string _windowName;
        SelectableObject _selected;
        OptionalRef<SceneDefinitionWrapper> _sceneDef;

        RenderResult renderEntity(EntityId entity) noexcept;
        RenderResult renderAsset(std::filesystem::path path) noexcept;
    };
}