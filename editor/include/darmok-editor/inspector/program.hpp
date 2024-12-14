#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/program.hpp>

namespace darmok::editor
{
    class ProgramInspectorEditor final : public ITypeObjectEditor<ProgramSource>
    {
    public:
        void init(AssetContext& assets, EditorProject& proj, ObjectEditorContainer& container) override;
        void shutdown() override;
        bool render(ProgramSource& src) noexcept override;
    private:
        OptionalRef<AssetContext> _assets;
        OptionalRef<EditorProject> _proj;
    };
}