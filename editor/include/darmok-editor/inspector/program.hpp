#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/program.hpp>

namespace darmok::editor
{
    class ProgramSourceInspectorEditor final : public AssetObjectEditor<protobuf::ProgramSource>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(protobuf::ProgramSource& src) noexcept override;
    private:
        static const std::string _shaderFilter;
    };
}