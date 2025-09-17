#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/program.hpp>

namespace darmok::editor
{
    class ProgramSourceInspectorEditor final : public AssetObjectEditor<Program::Source>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Program::Source& src) noexcept override;
    private:
        static const std::string _shaderFilter;
    };

    class ProgramRefInspectorEditor final : public ObjectEditor<Program::Ref>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Program::Ref& ref) noexcept override;
    };
}