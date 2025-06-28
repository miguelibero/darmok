#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/transform.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok::editor
{
    class TransformInspectorEditor final : public ITypeObjectEditor<Transform::Definition>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(Transform::Definition& trans) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
    };
}