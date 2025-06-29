#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok::editor
{
    class RenderableInspectorEditor final : public ITypeObjectEditor<Renderable::Definition>
    {
    public:
        std::string getTitle() const noexcept override;
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(Renderable::Definition& renderable) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
    };
}