#include <darmok/mesh_assimp.hpp>
#include <darmok/varying.hpp>
#include <darmok/vertex.hpp>
#include <darmok/glm_serialize.hpp>

#include "detail/assimp.hpp"
#include "detail/mesh_assimp.hpp"

#include <assimp/mesh.h>

namespace darmok
{
    AssimpMeshDefinitionConverterImpl::AssimpMeshDefinitionConverterImpl(const aiMesh& assimpMesh, Definition& meshDef, bx::AllocatorI& allocator, const ImportConfig& config) noexcept
        : _assimpMesh{ assimpMesh }
		, _meshDef{ meshDef }
		, _allocator{ allocator }
		, _config{ config }
    {
    }

    expected<void, std::string> AssimpMeshDefinitionConverterImpl::operator()() noexcept
    {
        _meshDef.set_name(AssimpUtils::getString(_assimpMesh.mName));
        auto& bounds = *_meshDef.mutable_bounds();
        *bounds.mutable_min() = protobuf::convert(AssimpUtils::convert(_assimpMesh.mAABB.mMin));
        *bounds.mutable_max() = protobuf::convert(AssimpUtils::convert(_assimpMesh.mAABB.mMax));
        _meshDef.set_type(Definition::Static);
        *_meshDef.mutable_layout() = _config.vertex_layout();

        std::vector<aiBone*> bones;
        bones.reserve(_assimpMesh.mNumBones);
        for (size_t i = 0; i < _assimpMesh.mNumBones; ++i)
        {
            bones.push_back(_assimpMesh.mBones[i]);
        }

        _meshDef.set_vertices(createVertexData(bones));
        _meshDef.set_indices(createIndexData());

        return {};
    }

    bool AssimpMeshDefinitionConverterImpl::updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept
    {
        if (bones.empty())
        {
            return false;
        }
        if (!writer.getLayout().has(bgfx::Attrib::Weight) && !writer.getLayout().has(bgfx::Attrib::Indices))
        {
            return false;
        }

        std::map<size_t, std::vector<std::pair<size_t, float>>> data;
        size_t i = 0;
        for (auto bone : bones)
        {
            for (size_t j = 0; j < bone->mNumWeights; j++)
            {
                auto& weight = bone->mWeights[j];
                if (weight.mWeight > 0.F)
                {
                    data[weight.mVertexId].emplace_back(i, weight.mWeight);
                }
            }
            ++i;
        }
        for (auto& [i, vert] : data)
        {
            glm::vec4 weights{ 1, 0, 0, 0 };
            glm::vec4 indices{ -1 };
            size_t j = 0;
            std::sort(vert.begin(), vert.end(), [](auto& a, auto& b) { return a.second > b.second; });
            for (auto& [index, weight] : vert)
            {
                indices[j] = index;
                weights[j] = weight;
                if (++j > 3)
                {
                    break;
                }
            }
            writer.write(bgfx::Attrib::Indices, i, indices);
            writer.write(bgfx::Attrib::Weight, i, weights);
        }
        return true;
    }

    AssimpCalcTangentsOperation::AssimpCalcTangentsOperation() noexcept
    {
        _iface.m_getNumFaces = getNumFaces;
        _iface.m_getNumVerticesOfFace = getNumFaceVertices;
        _iface.m_getNormal = getNormal;
        _iface.m_getPosition = getPosition;
        _iface.m_getTexCoord = getTexCoords;
        _iface.m_setTSpaceBasic = setTangent;

        _context.m_pInterface = &_iface;
    }

    std::vector<glm::vec3> AssimpCalcTangentsOperation::operator()(const aiMesh& mesh) noexcept
    {
        _mesh = mesh;
        _tangents.clear();
        _tangents.resize(mesh.mNumVertices);
        _context.m_pUserData = this;
        genTangSpaceDefault(&_context);
        _mesh.reset();
        return _tangents;
    }

    const aiMesh& AssimpCalcTangentsOperation::getMeshFromContext(const SMikkTSpaceContext* context) noexcept
    {
        return *static_cast<AssimpCalcTangentsOperation*>(context->m_pUserData)->_mesh;
    }

    int AssimpCalcTangentsOperation::getVertexIndex(const SMikkTSpaceContext* context, int iFace, int iVert) noexcept
    {
        auto& mesh = getMeshFromContext(context);
        return mesh.mFaces[iFace].mIndices[iVert];
    }

    int AssimpCalcTangentsOperation::getNumFaces(const SMikkTSpaceContext* context) noexcept
    {
        auto& mesh = getMeshFromContext(context);
        return mesh.mNumFaces;
    }

    int AssimpCalcTangentsOperation::getNumFaceVertices(const SMikkTSpaceContext* context, int iFace) noexcept
    {
        auto& mesh = getMeshFromContext(context);
        return mesh.mFaces[iFace].mNumIndices;
    }

    void AssimpCalcTangentsOperation::getPosition(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert) noexcept
    {
        auto& mesh = getMeshFromContext(context);
        auto index = getVertexIndex(context, iFace, iVert);
        auto& pos = mesh.mVertices[index];
        outpos[0] = pos.x;
        outpos[1] = pos.y;
        outpos[2] = pos.z;
    }

    void AssimpCalcTangentsOperation::getNormal(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert) noexcept
    {
        auto& mesh = getMeshFromContext(context);
        auto index = getVertexIndex(context, iFace, iVert);
        auto& norm = mesh.mNormals[index];
        outnormal[0] = norm.x;
        outnormal[1] = norm.y;
        outnormal[2] = norm.z;
    }

    void AssimpCalcTangentsOperation::getTexCoords(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert) noexcept
    {
        auto& mesh = getMeshFromContext(context);
        auto index = getVertexIndex(context, iFace, iVert);
        auto& texCoord = mesh.mTextureCoords[0][index];
        outuv[0] = texCoord.x;
        outuv[1] = texCoord.y;
    }

    void AssimpCalcTangentsOperation::setTangent(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert) noexcept
    {
        auto& op = *static_cast<AssimpCalcTangentsOperation*>(context->m_pUserData);
        auto index = getVertexIndex(context, iFace, iVert);
        auto& tangent = op._tangents[index];
        tangent.x = tangentu[0];
        tangent.y = tangentu[1];
        tangent.z = tangentu[2];
    }

    std::string AssimpMeshDefinitionConverterImpl::createVertexData(const std::vector<aiBone*>& bones) const noexcept
    {
        auto vertexCount = _assimpMesh.mNumVertices;
        auto layout = ConstVertexLayoutWrapper{ _config.vertex_layout() }.getBgfx();
        VertexDataWriter writer(layout, vertexCount, _allocator);

        std::vector<glm::vec3> tangents;
        if (_assimpMesh.mTangents == nullptr && layout.has(bgfx::Attrib::Tangent))
        {
            AssimpCalcTangentsOperation op;
            tangents = op(_assimpMesh);
        }

        for (size_t i = 0; i < _assimpMesh.mNumVertices; ++i)
        {
            if (_assimpMesh.mVertices)
            {
                writer.write(bgfx::Attrib::Position, i, AssimpUtils::convert(_assimpMesh.mVertices[i]));
            }
            if (_assimpMesh.mNormals)
            {
                writer.write(bgfx::Attrib::Normal, i, AssimpUtils::convert(_assimpMesh.mNormals[i]));
            }
            if (_assimpMesh.mTangents)
            {
                writer.write(bgfx::Attrib::Tangent, i, AssimpUtils::convert(_assimpMesh.mTangents[i]));
            }
            else if (tangents.size() > i)
            {
                writer.write(bgfx::Attrib::Tangent, i, tangents[i]);
            }
            for (size_t j = 0; j < AI_MAX_NUMBER_OF_COLOR_SETS; j++)
            {
                if (_assimpMesh.mColors[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::Color0 + j);
                    writer.write(attrib, i, AssimpUtils::convert(_assimpMesh.mColors[j][i]));
                }
            }
            for (size_t j = 0; j < AI_MAX_NUMBER_OF_TEXTURECOORDS; j++)
            {
                if (_assimpMesh.mTextureCoords[j])
                {
                    auto attrib = (bgfx::Attrib::Enum)(bgfx::Attrib::TexCoord0 + j);
                    writer.write(attrib, i, AssimpUtils::convert(_assimpMesh.mTextureCoords[j][i]));
                }
            }
        }
        updateBoneData(bones, writer);
        return writer.finish().toString();
    }

    std::string AssimpMeshDefinitionConverterImpl::createIndexData() const noexcept
    {
        size_t size = 0;
        for (size_t i = 0; i < _assimpMesh.mNumFaces; ++i)
        {
            size += _assimpMesh.mFaces[i].mNumIndices;
        }
        std::vector<VertexIndex> indices;
        indices.reserve(size);
        for (size_t i = 0; i < _assimpMesh.mNumFaces; ++i)
        {
            auto& face = _assimpMesh.mFaces[i];
            for (size_t j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }
        return std::string(reinterpret_cast<std::string::value_type*>(indices.data()),
            size * sizeof(VertexIndex) / sizeof(std::string::value_type));
    }

    AssimpMeshDefinitionConverter::AssimpMeshDefinitionConverter(const aiMesh& assimpMesh, Definition& meshDef, bx::AllocatorI& alloc, const ImportConfig& config) noexcept
        : _impl{ std::make_unique<AssimpMeshDefinitionConverterImpl>(assimpMesh, meshDef, alloc, config) }
    {
    }

    AssimpMeshDefinitionConverter::~AssimpMeshDefinitionConverter() = default;

    expected<void, std::string> AssimpMeshDefinitionConverter::operator()() noexcept
    {
		return _impl->operator()();
    }
}