#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/program.hpp>

namespace darmok::editor
{
    class ProgramSourceInspectorEditor final : public ITypeObjectEditor<ProgramSource>
    {
    public:
        void init(EditorApp& editor, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool renderType(ProgramSource& src) noexcept override;
    private:
        OptionalRef<EditorApp> _app;
        static const std::string _shaderFilter;
    };
}