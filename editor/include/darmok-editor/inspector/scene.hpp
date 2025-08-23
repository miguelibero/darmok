#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>

namespace darmok::editor
{
    class SceneInspectorEditor final : public AssetObjectEditor<Scene::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        void init(EditorApp& app, ObjectEditorContainer& editors) noexcept override;
        RenderResult renderType(Scene::Definition& scene) noexcept override;
    private:
        OptionalRef<ObjectEditorContainer> _editors;
    };
}