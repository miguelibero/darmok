#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/material.hpp>

namespace darmok::editor
{
    class MaterialInspectorEditor final : public ITypeObjectEditor<Material>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool render(Material& mat) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
    };
}