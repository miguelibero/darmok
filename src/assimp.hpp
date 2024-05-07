#pragma once

#include <string>
#include <memory>

#include <darmok/collection.hpp>
#include <darmok/data.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/vertex_fwd.hpp>

#include <assimp/light.h>
#include <assimp/material.h>
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
struct aiCamera;
struct aiLight;
template <typename TReal>
class aiColor4t;
typedef aiColor4t<float> aiColor4D;

namespace darmok
{
    class AssimpMaterialProperty final
    {
    public:
        AssimpMaterialProperty(aiMaterialProperty* ptr);
        [[nodiscard]] std::string_view getKey() const noexcept;
        [[nodiscard]] aiPropertyTypeInfo getType() const noexcept;
        [[nodiscard]] aiTextureType getTextureType() const noexcept;
        [[nodiscard]] size_t getTextureIndex() const noexcept;
        [[nodiscard]] const DataView& getData() const noexcept;
    private:
        aiMaterialProperty* _ptr;
        DataView _data;
    };

    class AssimpMaterialPropertyCollection final : public MemReadOnlyCollection<AssimpMaterialProperty>
    {
    public:
        AssimpMaterialPropertyCollection(aiMaterial* ptr);
        [[nodiscard]] size_t size() const override;
    private:
        aiMaterial* _ptr;
        AssimpMaterialProperty create(size_t pos) const override;
    };

    class AssimpMaterialTexture final
    {
    public:
        AssimpMaterialTexture(aiMaterial* ptr, aiTextureType type, unsigned int index);
        aiTextureType getType() const;
        size_t getIndex() const;
        const std::string& getPath() const;
        aiTextureMapping getMapping() const;
        aiTextureOp getOperation() const;
        aiTextureMapMode getMapMode() const;
        float getBlend() const;
        size_t getCoordIndex() const;

    private:
        aiMaterial* _ptr;
        aiTextureType _type;
        size_t _index;
        std::string _path;
        size_t _coordIndex;
        float _blend;
        aiTextureMapping _mapping;
        aiTextureOp _operation;
        aiTextureMapMode _mapMode;
    };

    class AssimpMaterialTextureCollection final : public MemReadOnlyCollection<AssimpMaterialTexture>
    {
    public:
        AssimpMaterialTextureCollection(aiMaterial* ptr, aiTextureType type);
        size_t size() const override;
    private:
        aiMaterial* _ptr;
        aiTextureType _type;
        AssimpMaterialTexture create(size_t pos) const override;
    };

    enum class AssimpMaterialColorType
    {
        Diffuse,
        Specular,
        Ambient,
        Emissive,
        Transparent,
        Reflective,
        Count
    };    

    class AssimpMaterial final
    {
    public:
        AssimpMaterial(aiMaterial* ptr);
        std::string_view getName() const;
        AssimpMaterialTextureCollection getTextures(aiTextureType type) const;
        const AssimpMaterialPropertyCollection& getProperties() const;
        std::optional<Color> getColor(AssimpMaterialColorType type) const;
        bool showWireframe() const;
        aiShadingMode getShadingMode() const;
        float getOpacity() const;
    private:
        aiMaterial* _ptr;
        AssimpMaterialPropertyCollection _properties;
    };

    class AssimpMeshFaceIndexCollection final : public ReadOnlyCollection<VertexIndex>
    {
    public:
        AssimpMeshFaceIndexCollection(aiFace* ptr);
        size_t size() const override;
        const VertexIndex& operator[](size_t pos) const override;
        VertexIndex& operator[](size_t pos) override;
    private:
        aiFace* _ptr;
    };

    class AssimpMeshFace final
    {
    public:
        AssimpMeshFace(aiFace* ptr);
        size_t size() const;
        bool empty() const;
        const AssimpMeshFaceIndexCollection& getIndices() const;
    private:
        aiFace* _ptr;
        AssimpMeshFaceIndexCollection _indices;
    };

    class AssimpVector3Collection final : public MemReadOnlyCollection<glm::vec3>
    {
    public:
        AssimpVector3Collection(aiVector3D* ptr, size_t size);
        size_t size() const override;

    private:
        aiVector3D* _ptr;
        size_t _size;
        glm::vec3 create(size_t pos) const override;
    };

    class AssimpTextureCoords final
    {
    public:
        AssimpTextureCoords(aiMesh* mesh, size_t pos);
        size_t getCompCount() const;
        const AssimpVector3Collection& getCoords() const;
    private:
        size_t _compCount;
        AssimpVector3Collection _coords;
    };

    class AssimpMeshFaceCollection final : public MemReadOnlyCollection<AssimpMeshFace>
    {
    public:
        AssimpMeshFaceCollection(aiMesh* ptr);
        size_t size() const override;
    private:
        aiMesh* _ptr;
        AssimpMeshFace create(size_t pos) const override;
    };

    class AssimpMeshTextureCoordsCollection final : public MemReadOnlyCollection<AssimpTextureCoords>
    {
    public:
        AssimpMeshTextureCoordsCollection(aiMesh* ptr);
        size_t size() const override;
    private:
        aiMesh* _ptr;
        AssimpTextureCoords create(size_t pos) const override;
    };

    class AssimpColorCollection final : public MemReadOnlyCollection<Color>
    {
    public:
        AssimpColorCollection(aiColor4D* ptr, size_t size);
        size_t size() const override;

    private:
        aiColor4D* _ptr;
        size_t _size;
        Color create(size_t pos) const override;
    };

    class AssimpMeshColorsCollection final : public MemReadOnlyCollection<AssimpColorCollection>
    {
    public:
        AssimpMeshColorsCollection(aiMesh* ptr);
        size_t size() const override;
    private:
        aiMesh* _ptr;
        AssimpColorCollection create(size_t pos) const override;
    };

    class AssimpScene;
    class Mesh;

    class AssimpMesh final
    {
    public:
        AssimpMesh(aiMesh* ptr, AssimpMaterial& material);

        AssimpMaterial& getMaterial();
        const AssimpMaterial& getMaterial() const;
        const AssimpVector3Collection& getPositions() const;
        const AssimpVector3Collection& getNormals() const;
        const AssimpVector3Collection& getTangents() const;
        const AssimpVector3Collection& getBitangents() const;
        const AssimpMeshTextureCoordsCollection& getTexCoords() const;
        const AssimpMeshFaceCollection& getFaces() const;
        const AssimpMeshColorsCollection& getColors() const;
        const size_t getVertexCount() const;

    private:
        aiMesh* _ptr;
        AssimpMaterial& _material;
        AssimpVector3Collection _positions;
        AssimpVector3Collection _normals;
        AssimpVector3Collection _tangents;
        AssimpVector3Collection _bitangents;
        AssimpMeshTextureCoordsCollection _texCoords;
        AssimpMeshFaceCollection _faces;
        AssimpMeshColorsCollection _colors;
    };

    class AssimpNodeMeshCollection final : public ReadOnlyCollection<AssimpMesh>
    {
    public:
        AssimpNodeMeshCollection(aiNode* ptr, AssimpScene& scene);
        size_t size() const override;
        const AssimpMesh& operator[](size_t pos) const override;
        AssimpMesh& operator[](size_t pos) override;
    private:
        aiNode* _ptr;
        AssimpScene& _scene;
    };

    class AssimpCamera final
    {
    public:
        AssimpCamera(aiCamera* ptr);
        std::string_view getName() const;
        const glm::mat4& getProjectionMatrix() const;
        const glm::mat4& getViewMatrix() const;
        float getAspect() const;
        float getClipFar() const;
        float getClipNear() const;
        float getHorizontalFieldOfView() const;
        float getOrthographicWidth() const;
        glm::vec3 getLookAt() const;
        glm::vec3 getPosition() const;
        glm::vec3 getUp() const;
    private:
        aiCamera* _ptr;
        glm::mat4 _view;
        glm::mat4 _proj;
    };

    class AssimpLight final
    {
    public:
        AssimpLight(aiLight* ptr) noexcept;

        [[nodiscard]] std::string_view getName() const noexcept;
        [[nodiscard]] aiLightSourceType getType() const noexcept;
        [[nodiscard]] float getInnerConeAngle() const noexcept;
        [[nodiscard]] float getOuterConeAngle() const noexcept;
        [[nodiscard]] glm::vec3 getAttenuation() const noexcept;
        [[nodiscard]] Color3 getAmbientColor() const noexcept;
        [[nodiscard]] Color3 getDiffuseColor() const noexcept;
        [[nodiscard]] Color3 getSpecularColor() const noexcept;
        [[nodiscard]] glm::vec3 getDirection() const noexcept;
        [[nodiscard]] glm::vec3 getPosition() const noexcept;
        [[nodiscard]] glm::vec2 getSize() const noexcept;
    private:
        aiLight* _ptr;
    };

    class AssimpNode;

    class AssimpNodeChildrenCollection final : public MemReadOnlyCollection<AssimpNode>
    {
    public:
        AssimpNodeChildrenCollection(aiNode* ptr, AssimpScene& scene);
        size_t size() const override;
    private:
        aiNode* _ptr;
        AssimpScene& _scene;
        AssimpNode create(size_t pos) const override;
    };

    class AssimpNode final
    {
    public:
        AssimpNode(aiNode* ptr, AssimpScene& scene);
        
        std::string_view getName() const;
        glm::mat4 getTransform() const;
        const AssimpNodeMeshCollection& getMeshes() const;
        const AssimpNodeChildrenCollection& getChildren() const;
        OptionalRef<const AssimpNode> getChild(const std::string_view& path) const;
        OptionalRef<const AssimpCamera> getCamera() const;
        OptionalRef<const AssimpLight> getLight() const;

        AssimpNodeMeshCollection& getMeshes();
        AssimpNodeChildrenCollection& getChildren();
        OptionalRef<AssimpNode> getChild(const std::string_view& path);
        OptionalRef<AssimpCamera> getCamera();
        OptionalRef<AssimpLight> getLight();

    private:
        aiNode* _ptr;
        AssimpNodeMeshCollection _meshes;
        AssimpNodeChildrenCollection _children;
        AssimpScene& _scene;
    };

    class AssimpCameraCollection final : public MemReadOnlyCollection<AssimpCamera>
    {
    public:
        AssimpCameraCollection(const aiScene* ptr);
        size_t size() const override;
        OptionalRef<AssimpCamera> get(const std::string_view& name);
        OptionalRef<const AssimpCamera> get(const std::string_view& name) const;
    private:
        const aiScene* _ptr;
        AssimpCamera create(size_t pos) const override;
    };

    class AssimpLightCollection final : public MemReadOnlyCollection<AssimpLight>
    {
    public:
        AssimpLightCollection(const aiScene* ptr);
        size_t size() const override;
        OptionalRef<AssimpLight> get(const std::string_view& name);
        OptionalRef<const AssimpLight> get(const std::string_view& name) const;
    private:
        const aiScene* _ptr;
        AssimpLight create(size_t pos) const override;
    };

    class AssimpMaterialCollection final : public MemReadOnlyCollection<AssimpMaterial>
    {
    public:
        AssimpMaterialCollection(const aiScene* ptr);
        size_t size() const override;
    private:
        const aiScene* _ptr;

        AssimpMaterial create(size_t pos) const override;
    };

    class AssimpMeshCollection final : public MemReadOnlyCollection<AssimpMesh>
    {
    public:
        AssimpMeshCollection(const aiScene* ptr, AssimpMaterialCollection& materials);
        size_t size() const override;
    private:
        const aiScene* _ptr;
        AssimpMaterialCollection& _materials;
        AssimpMesh create(size_t pos) const override;
    };

    class AssimpScene final
	{
    public:
        AssimpScene(const aiScene* ptr) noexcept;
        AssimpScene(const AssimpScene& other) = delete;
        AssimpScene& operator=(const AssimpScene& other) = delete;
        ~AssimpScene() noexcept;
        std::string_view getName() const noexcept;

        const AssimpNode& getRootNode() const noexcept;
        const AssimpMeshCollection& getMeshes() const noexcept;
        const AssimpMaterialCollection& getMaterials() const noexcept;
        const AssimpCameraCollection& getCameras() const noexcept;
        const AssimpLightCollection& getLights() const noexcept;

        AssimpNode& getRootNode() noexcept;
        AssimpMeshCollection& getMeshes() noexcept;
        AssimpMaterialCollection& getMaterials() noexcept;
        AssimpCameraCollection& getCameras() noexcept;
        AssimpLightCollection& getLights() noexcept;

    private:
        const aiScene* _ptr;
        AssimpNode _rootNode;
        AssimpMeshCollection _meshes;
        AssimpMaterialCollection _materials;
        AssimpCameraCollection _cameras;
        AssimpLightCollection _lights;
	};
}