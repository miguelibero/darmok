#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene.hpp>

namespace darmok::editor
{
    class SceneInspectorEditor final : public ITypeObjectEditor<Scene>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& editors) noexcept override;
        bool renderType(Scene& scene) noexcept override;
    private:
        OptionalRef<ObjectEditorContainer> _editors;
    };
}