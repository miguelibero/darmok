#pragma once

#include <darmok-editor/editor.hpp>
#include <darmok/mesh.hpp>
#include <darmok/protobuf/scene.pb.h>

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
        static const std::string _externalFilter;

		std::shared_ptr<aiScene> _externalScene;
        std::vector<std::string> _externalMeshes;
		size_t _externalMeshIndex = 0;

        expected<void, std::string> loadSceneMesh(protobuf::DataMeshSource& src) noexcept;
        RenderResult renderData(Mesh::Source& src) noexcept;
        RenderResult renderSphere(Mesh::Source& src) noexcept;
        RenderResult renderCube(Mesh::Source& srce) noexcept;
        RenderResult renderCapsule(Mesh::Source& src) noexcept;
        RenderResult renderRectangle(Mesh::Source& src) noexcept;
    };
}
