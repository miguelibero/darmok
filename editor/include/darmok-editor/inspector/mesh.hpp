#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/mesh.hpp>

#include <variant>

namespace darmok::editor
{
    class MeshSourceInspectorEditor final : public AssetObjectEditor<Mesh::Source>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Mesh::Source& src) noexcept override;
    };
}
