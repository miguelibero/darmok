#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/mesh.hpp>

#include <filesystem>
#include <vector>
#include <string>

struct aiScene;

namespace darmok::editor
{
    class MeshSourceInspectorEditor final : public AssetObjectEditor<Mesh::Source>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Mesh::Source& src) noexcept override;
    private:
        MeshFileInput _meshInput;

        RenderResult renderData(Mesh::Source& src) noexcept;
        RenderResult renderSphere(Mesh::Source& src) noexcept;
        RenderResult renderCube(Mesh::Source& srce) noexcept;
        RenderResult renderCapsule(Mesh::Source& src) noexcept;
        RenderResult renderRectangle(Mesh::Source& src) noexcept;
        RenderResult renderCone(Mesh::Source& src) noexcept;
        RenderResult renderCylinder(Mesh::Source& src) noexcept;
    };
}
