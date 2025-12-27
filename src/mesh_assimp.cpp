#include <darmok/mesh_assimp.hpp>
#include <darmok/varying.hpp>
#include <darmok/vertex.hpp>
#include <darmok/glm_serialize.hpp>
#include <darmok/assimp.hpp>

#include "detail/mesh_assimp.hpp"

#include <assimp/scene.h>
#include <assimp/mesh.h>

namespace darmok
{
    AssimpMeshSourceConverterImpl::AssimpMeshSourceConverterImpl(const aiMesh& assimpMesh, Definition& def) noexcept
        : _assimpMesh{ assimpMesh }
        , _def{ def }
    {
	}

    expected<void, std::string> AssimpMeshSourceConverterImpl::operator()() noexcept
    {
        auto& vertices = *_def.mutable_vertices();
        vertices.Clear();
		vertices.Reserve(_assimpMesh.mNumVertices);

        for (size_t i = 0; i < _assimpMesh.mNumVertices; ++i)
        {
			auto& v = *vertices.Add();
			*v.mutable_color() = convert<protobuf::Color>(Colors::white());

			*v.mutable_position() = convert<protobuf::Vec3>(convert<glm::vec3>(_assimpMesh.mVertices[i]));
            *v.mutable_normal() = convert<protobuf::Vec3>(convert<glm::vec3>(_assimpMesh.mNormals[i]));
            if (_assimpMesh.mTangents != nullptr)
            {
                *v.mutable_tangent() = convert<protobuf::Vec3>(convert<glm::vec3>(_assimpMesh.mTangents[i]));
            }
            for (size_t j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; ++j)
            {
                if (_assimpMesh.mColors[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::Color0 + j);
					*v.mutable_color() = convert<protobuf::Color>(convert<Color>(_assimpMesh.mColors[j][i]));
                }
            }
            for (size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; ++j)
            {
                if (_assimpMesh.mTextureCoords[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::TexCoord0 + j);
                    glm::vec2 texCoord = convert<glm::vec3>(_assimpMesh.mTextureCoords[j][i]);
                    *v.mutable_tex_coord() = convert<protobuf::Vec2>(texCoord);
                }
            }
        }

        auto& indices = *_def.mutable_indices();
        size_t size = 0;
        for (size_t i = 0; i < _assimpMesh.mNumFaces; ++i)
        {
            size += _assimpMesh.mFaces[i].mNumIndices;
        }
        indices.Clear();
        indices.Reserve(size);
        for (size_t i = 0; i < _assimpMesh.mNumFaces; ++i)
        {
            auto& face = _assimpMesh.mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; ++j)
            {
                indices.Add(face.mIndices[j]);
            }
        }

		auto& bones = *_def.mutable_bones();
        bones.Clear();
        bones.Reserve(_assimpMesh.mNumBones);
        for (size_t i = 0; i < _assimpMesh.mNumBones; ++i)
        {
            auto& assimpBone = _assimpMesh.mBones[i];
            auto& bone = *bones.Add();
            bone.set_name(convert<std::string>(assimpBone->mName));
            auto& weights = *bone.mutable_weights();
            for (size_t j = 0; j < assimpBone->mNumWeights; ++j)
            {
                auto& assimpWeight = assimpBone->mWeights[j];
				auto& weight = *weights.Add();
				weight.set_vertex_id(assimpWeight.mVertexId);
				weight.set_value(assimpWeight.mWeight);
            }
        }

        return {};
    }

    AssimpMeshSourceConverter::AssimpMeshSourceConverter(const aiMesh& assimpMesh, Definition& def) noexcept
        : _impl{ std::make_unique<AssimpMeshSourceConverterImpl>(assimpMesh, def) }
    {
    }

    AssimpMeshSourceConverter::~AssimpMeshSourceConverter() = default;

    expected<void, std::string> AssimpMeshSourceConverter::operator()() noexcept
    {
        return _impl->operator()();
    }
}