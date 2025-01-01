#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/mesh_source.hpp>

#include <variant>

namespace darmok::editor
{
    class MeshSourceInspectorEditor final : public ITypeObjectEditor<MeshSource>
    {
    public:
        void init(EditorApp& editor, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool render(MeshSource& src) noexcept override;
    private:
        OptionalRef<EditorApp> _editor;
        OptionalRef<ObjectEditorContainer> _container;
        static const std::array<std::string, std::variant_size_v<MeshSource::Content>> _typeLabels;
    };
}
