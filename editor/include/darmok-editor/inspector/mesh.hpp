#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/mesh.hpp>

#include <variant>

namespace darmok::editor
{
    class MeshSourceInspectorEditor final : public ITypeObjectEditor<Mesh::Source>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(Mesh::Source& src) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
        OptionalRef<ObjectEditorContainer> _container;
    };
}
