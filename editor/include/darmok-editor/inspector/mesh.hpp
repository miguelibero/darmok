#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/mesh_source.hpp>

#include <variant>

namespace darmok::editor
{
    class MeshSourceInspectorEditor final : public ITypeObjectEditor<MeshSource>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(MeshSource& src) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
        OptionalRef<ObjectEditorContainer> _container;
        static const std::array<std::string, std::variant_size_v<MeshSource::Content>> _contentOptions;
    };
}
