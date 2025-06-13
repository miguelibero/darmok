#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/material.hpp>

namespace darmok::editor
{
    class MaterialInspectorEditor final : public ITypeObjectEditor<Material::Definition>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(Material::Definition& mat) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
    };
}