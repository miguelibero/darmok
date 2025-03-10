#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/mesh.hpp>

#include <variant>

namespace darmok::editor
{
    class MeshSourceInspectorEditor final : public ITypeObjectEditor<protobuf::MeshSource>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(protobuf::MeshSource& src) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
        OptionalRef<ObjectEditorContainer> _container;
        static const std::array<std::string, std::variant_size_v<protobuf::MeshSource::Content>> _contentOptions;
    };
}
