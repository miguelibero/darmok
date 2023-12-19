#pragma once

#include <darmok/utils.hpp>
#include <darmok/data.hpp>
#include <vector>
#include <string>
#include <glm/glm.hpp>

template <typename TReal>
class aiVector3t;
typedef aiVector3t<float> aiVector3D;
struct aiMesh;
struct aiFace;
struct aiNode;
struct aiScene;
struct aiMaterial;
struct aiMaterialProperty;

namespace darmok
{
    enum class ModelTextureType
    {
        None = 0,
        Diffuse = 1,
        Specular = 2,
        Ambient = 3,
        Emissive = 4,
        Height = 5,
        Normals = 6,
        Shininess = 7,
        Opacity = 8,
        Displacement = 9,
        Lightmap = 10,
        Reflection = 11,
        BaseColor = 12,
        NormalCamera = 13,
        EmissionColor = 14,
        Metalness = 15,
        DiffuseRoughness = 16,
        AmbientOcclusion = 17,
        Sheen = 19,
        ClearCoat = 20,
        Transmission = 21,
        Unknown = 18,
        Count,
    };

    enum class ModelTextureMapping {
        UV = 0x0,
        Sphere = 0x1,
        Cylinder = 0x2,
        Box = 0x3,
        Plane = 0x4,
        Other = 0x5,
        Count
    };

    enum class ModelTextureOperation {
        Multiply = 0x0,
        Add = 0x1,
        Subtract = 0x2,
        Divide = 0x3,
        SmoothAdd = 0x4,
        SignedAdd = 0x5,
        Count
    };

    enum class ModelTextureMapMode {
        Wrap = 0x0,
        Clamp = 0x1,
        Decal = 0x3,
        Mirror = 0x2,
        Count
    };

    class ModelTexture final
    {
    public:
        ModelTexture(aiMaterial* ptr, ModelTextureType type, unsigned int index);
        ModelTextureType getType() const;
        const std::string& getPath() const;
        ModelTextureMapping getMapping() const;
        ModelTextureOperation getOperation() const;
        ModelTextureMapMode getMapMode() const;
        float getBlend() const;
        size_t getCoordIndex() const;
    private:
        ModelTextureType _type;
        std::string _path;
        ModelTextureMapping _mapping;
        size_t _coordIndex;
        float _blend;
        ModelTextureOperation _operation;
        ModelTextureMapMode _mapMode;
    };

    class ModelTextureContainer final : public MemContainer<ModelTexture>
    {
    public:
        ModelTextureContainer(aiMaterial* ptr);
        size_t size() const override;
    private:
        aiMaterial* _ptr;
        ModelTexture create(size_t pos) const override;
    };

    enum class ModelMaterialPropertyType
    {
        Float = 0x1,
        Double = 0x2,
        String = 0x3,
        Integer = 0x4,
        Buffer = 0x5,
    };

    class ModelMaterialProperty final
    {
    public:
        ModelMaterialProperty(aiMaterialProperty* ptr);
        std::string_view getKey() const;
        ModelMaterialPropertyType getType() const;
    private:
        aiMaterialProperty* _ptr;
    };

    class ModelMaterialPropertyContainer final : public MemContainer<ModelMaterialProperty>
    {
    public:
        ModelMaterialPropertyContainer(aiMaterial* ptr);
        size_t size() const override;
    private:
        aiMaterial* _ptr;
        ModelMaterialProperty create(size_t pos) const override;
    };

    class ModelMaterial final
    {
    public:
        ModelMaterial(aiMaterial* ptr);
        std::string_view getName() const;
        const ModelTextureContainer& getTextures() const;
        const ModelMaterialPropertyContainer& getProperties() const;
    private:
        aiMaterial* _ptr;
        ModelTextureContainer _textures;
        ModelMaterialPropertyContainer _properties;
    };

    class ModelMeshFaceIndexContainer final : public BaseContainer<VertexIndex>
    {
    public:
        ModelMeshFaceIndexContainer(aiFace* ptr);
        size_t size() const override;
        const VertexIndex& operator[](size_t pos) const  override;
        VertexIndex& operator[](size_t pos) override;
    private:
        aiFace* _ptr;
    };

    class ModelMeshFace final
    {
    public:
        ModelMeshFace(aiFace* ptr);
        size_t size() const;
        bool empty() const;
        const ModelMeshFaceIndexContainer& getIndices() const;
    private:
        aiFace* _ptr;
        ModelMeshFaceIndexContainer _indices;
    };

    class ModelVector3Container final : public BaseContainer<glm::vec3>
    {
    public:
        ModelVector3Container(aiVector3D* ptr, size_t size);
        size_t size() const override;
        const glm::vec3& operator[](size_t pos) const  override;
        glm::vec3& operator[](size_t pos) override;
    private:
        aiVector3D* _ptr;
        size_t _size;
    };

    class ModelTextureCoords final
    {
    public:
        ModelTextureCoords(aiMesh* mesh, size_t pos);
        size_t getCompCount() const;
        const ModelVector3Container& getCoords() const;
    private:
        size_t _compCount;
        ModelVector3Container _coords;
    };

    class ModelMeshFaceContainer final : public MemContainer<ModelMeshFace>
    {
    public:
        ModelMeshFaceContainer(aiMesh* ptr);
        size_t size() const override;
    private:
        aiMesh* _ptr;
        ModelMeshFace create(size_t pos) const override;
    };

    class ModelMeshTextureCoordsContainer final : public MemContainer<ModelTextureCoords>
    {
    public:
        ModelMeshTextureCoordsContainer(aiMesh* ptr);
        size_t size() const override;
    private:
        aiMesh* _ptr;
        ModelTextureCoords create(size_t pos) const override;
    };

    class ModelMesh final
    {
    public:
        ModelMesh(aiMesh* ptr, const aiScene* scene);

        const ModelMaterial& getMaterial() const;
        const ModelVector3Container& getVertices() const;
        const ModelVector3Container& getNormals() const;
        const ModelVector3Container& getTangents() const;
        const ModelVector3Container& getBitangents() const;
        const ModelMeshTextureCoordsContainer& getTexCoords() const;
        const ModelMeshFaceContainer& getFaces() const;
        const size_t getVertexCount() const;
    private:
        aiMesh* _ptr;
        ModelMaterial _material;
        ModelVector3Container _vertices;
        ModelVector3Container _normals;
        ModelVector3Container _tangents;
        ModelVector3Container _bitangents;
        ModelMeshTextureCoordsContainer _texCoords;
        ModelMeshFaceContainer _faces;
    };

    class ModelNode final
    {
    public:
        ModelNode(aiNode* ptr, const aiScene* scene);
        
        std::string_view getName() const;
        const glm::mat4x4& getTransform() const;
        const std::vector<ModelMesh>& getMeshes() const;
        const std::vector<ModelNode>& getChildren() const;

    private:
        aiNode* _ptr;
        std::vector<ModelMesh> _meshes;
        std::vector<ModelNode> _children;
    };

    class Model final
	{
    public:
        Model(const aiScene* ptr);
        
        std::string_view getName() const;
        const ModelNode& getRootNode() const;
    private:
        const aiScene* _ptr;
        ModelNode _rootNode;
	};
}