#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/texture.hpp>
#include <darmok/optional_ref.hpp>

namespace darmok::editor
{
    class TextureDefinitionInspectorEditor final : public ITypeObjectEditor<TextureDefinition>
    {
    public:
        void init(EditorApp& app, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(TextureDefinition& def) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
    };
}