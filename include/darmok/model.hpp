#pragma once

#include <darmok/utils.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
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
    enum class ModelMaterialTextureType
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

    enum class ModelMaterialTextureMapping
    {
        UV = 0x0,
        Sphere = 0x1,
        Cylinder = 0x2,
        Box = 0x3,
        Plane = 0x4,
        Other = 0x5,
        Count
    };

    enum class ModelMaterialTextureOperation
    {
        Multiply = 0x0,
        Add = 0x1,
        Subtract = 0x2,
        Divide = 0x3,
        SmoothAdd = 0x4,
        SignedAdd = 0x5,
        Count
    };

    enum class ModelMaterialTextureMapMode
    {
        Wrap = 0x0,
        Clamp = 0x1,
        Decal = 0x3,
        Mirror = 0x2,
        Count
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
        ModelMaterialTextureType getTextureType() const;
        size_t getTextureIndex() const;
        const Data& getData() const;
    private:
        aiMaterialProperty* _ptr;
        Data _data;
    };

    class ModelMaterialPropertyCollection final : public MemReadOnlyCollection<ModelMaterialProperty>
    {
    public:
        ModelMaterialPropertyCollection(aiMaterial* ptr);
        size_t size() const override;
    private:
        aiMaterial* _ptr;
        ModelMaterialProperty create(size_t pos) const override;
    };

    class ModelMaterialTexture final
    {
    public:
        ModelMaterialTexture(aiMaterial* ptr, ModelMaterialTextureType type, unsigned int index);
        ModelMaterialTextureType getType() const;
        size_t getIndex() const;
        const std::string& getPath() const;
        ModelMaterialTextureMapping getMapping() const;
        ModelMaterialTextureOperation getOperation() const;
        ModelMaterialTextureMapMode getMapMode() const;
        float getBlend() const;
        size_t getCoordIndex() const;

    private:
        aiMaterial* _ptr;
        ModelMaterialTextureType _type;
        size_t _index;
        std::string _path;
        ModelMaterialTextureMapping _mapping;
        size_t _coordIndex;
        float _blend;
        ModelMaterialTextureOperation _operation;
        ModelMaterialTextureMapMode _mapMode;
    };

    class ModelMaterialTextureCollection final : public MemReadOnlyCollection<ModelMaterialTexture>
    {
    public:
        ModelMaterialTextureCollection(aiMaterial* ptr, ModelMaterialTextureType type);
        size_t size() const override;
    private:
        aiMaterial* _ptr;
        ModelMaterialTextureType _type;
        ModelMaterialTexture create(size_t pos) const override;
    };

    class ModelMaterial final
    {
    public:
        ModelMaterial(aiMaterial* ptr);
        std::string_view getName() const;
        ModelMaterialTextureCollection getTextures(ModelMaterialTextureType type) const;
        const ModelMaterialPropertyCollection& getProperties() const;
    private:
        aiMaterial* _ptr;
        std::string _path;
        ModelMaterialPropertyCollection _properties;
    };

    class ModelMeshFaceIndexCollection final : public BaseReadOnlyCollection<VertexIndex>
    {
    public:
        ModelMeshFaceIndexCollection(aiFace* ptr);
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
        const ModelMeshFaceIndexCollection& getIndices() const;
    private:
        aiFace* _ptr;
        ModelMeshFaceIndexCollection _indices;
    };

    class ModelVector3Collection final : public BaseReadOnlyCollection<glm::vec3>
    {
    public:
        ModelVector3Collection(aiVector3D* ptr, size_t size);
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
        const ModelVector3Collection& getCoords() const;
    private:
        size_t _compCount;
        ModelVector3Collection _coords;
    };

    class ModelMeshFaceCollection final : public MemReadOnlyCollection<ModelMeshFace>
    {
    public:
        ModelMeshFaceCollection(aiMesh* ptr);
        size_t size() const override;
    private:
        aiMesh* _ptr;
        ModelMeshFace create(size_t pos) const override;
    };

    class ModelMeshTextureCoordsCollection final : public MemReadOnlyCollection<ModelTextureCoords>
    {
    public:
        ModelMeshTextureCoordsCollection(aiMesh* ptr);
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
        const ModelVector3Collection& getVertices() const;
        const ModelVector3Collection& getNormals() const;
        const ModelVector3Collection& getTangents() const;
        const ModelVector3Collection& getBitangents() const;
        const ModelMeshTextureCoordsCollection& getTexCoords() const;
        const ModelMeshFaceCollection& getFaces() const;
        const size_t getVertexCount() const;
    private:
        aiMesh* _ptr;
        ModelMaterial _material;
        ModelVector3Collection _vertices;
        ModelVector3Collection _normals;
        ModelVector3Collection _tangents;
        ModelVector3Collection _bitangents;
        ModelMeshTextureCoordsCollection _texCoords;
        ModelMeshFaceCollection _faces;
    };

    class ModelNodeMeshCollection final : public MemReadOnlyCollection<ModelMesh>
    {
    public:
        ModelNodeMeshCollection(aiNode* ptr, const aiScene* scene);
        size_t size() const override;
    private:
        aiNode* _ptr;
        const aiScene* _scene;
        ModelMesh create(size_t pos) const override;
    };

    class ModelNode;

    class ModelNodeChildrenCollection final : public MemReadOnlyCollection<ModelNode>
    {
    public:
        ModelNodeChildrenCollection(aiNode* ptr, const aiScene* scene);
        size_t size() const override;
    private:
        aiNode* _ptr;
        const aiScene* _scene;
        ModelNode create(size_t pos) const override;
    };

    class ModelNode final
    {
    public:
        ModelNode(aiNode* ptr, const aiScene* scene);
        
        std::string_view getName() const;
        const glm::mat4x4& getTransform() const;
        const ModelNodeMeshCollection& getMeshes() const;
        const ModelNodeChildrenCollection& getChildren() const;

    private:
        aiNode* _ptr;
        ModelNodeMeshCollection _meshes;
        ModelNodeChildrenCollection _children;
    };

    class ModelMaterialCollection final : public MemReadOnlyCollection<ModelMaterial>
    {
    public:
        ModelMaterialCollection(const aiScene* ptr);
        size_t size() const override;
    private:
        const aiScene* _ptr;
        ModelMaterial create(size_t pos) const override;
    };

    class Model final
	{
    public:
        Model(const aiScene* ptr, const std::string& path = {});
        std::string_view getName() const;
        const std::string& getPath() const;
        const ModelNode& getRootNode() const;
        const ModelMaterialCollection& getMaterials() const;
    private:
        const aiScene* _ptr;
        ModelNode _rootNode;
        std::string _path;
        ModelMaterialCollection _materials;
	};
}