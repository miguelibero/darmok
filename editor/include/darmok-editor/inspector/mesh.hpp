#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/mesh.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <filesystem>
#include <vector>
#include <string>

namespace darmok::editor
{
    class MeshSourceInspectorEditor final : public AssetObjectEditor<Mesh::Source>
    {
    public:
        std::string getTitle() const noexcept override;
        RenderResult renderType(Mesh::Source& src) noexcept override;
    private:
        static const std::string _externalFilter;
        std::filesystem::path _externalPath;
		Data _externalData;
        std::vector<std::string> _externalMeshes;
		size_t _selectedExternalMeshIndex = 0;

        RenderResult renderData(Mesh::Source& src) noexcept;
        RenderResult renderSphere(Mesh::Source& src) noexcept;
        RenderResult renderCube(Mesh::Source& srce) noexcept;
        RenderResult renderCapsule(Mesh::Source& src) noexcept;
        RenderResult renderRectangle(Mesh::Source& src) noexcept;
    };
}
