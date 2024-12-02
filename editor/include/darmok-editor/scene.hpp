#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok
{
    class Scene;
}

namespace darmok::editor
{
    class SceneInspectorEditor final : public ITypeObjectEditor<Scene>
    {
    public:
        void init(EditorAppDelegate& app, ObjectEditorContainer& editors) noexcept override;
        bool render(Scene& scene) noexcept override;
    private:
        OptionalRef<ObjectEditorContainer> _editors;
    };
}