#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include <darmok/collection.hpp>
#include <darmok/data.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/material_fwd.hpp>

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

namespace Assimp
{
    class Importer;
}

namespace darmok
{
    using aiSceneRef = std::shared_ptr<const aiScene>;

    class AssimpMaterialProperty final
    {
    public:
        AssimpMaterialProperty(const aiMaterialProperty& prop, const aiSceneRef& scene) noexcept;

        [[nodiscard]] std::string_view getKey() const noexcept;
        [[nodiscard]] aiPropertyTypeInfo getType() const noexcept;
        [[nodiscard]] aiTextureType getTextureType() const noexcept;
        [[nodiscard]] size_t getTextureIndex() const noexcept;
        [[nodiscard]] const DataView& getData() const noexcept;
    private:
        const aiMaterialProperty& _prop;
        DataView _data;
        aiSceneRef _scene;
    };

    class AssimpMaterialPropertyCollection final : public VectorReadOnlyCollection<AssimpMaterialProperty>
    {
    public:
        AssimpMaterialPropertyCollection(const aiMaterial& material, const aiSceneRef& scene) noexcept;
    };

    class Texture;
    class ITextureLoader;

    class AssimpMaterialTexture final
    {
    public:
        AssimpMaterialTexture(const aiMaterial& material, aiTextureType type, unsigned int index) noexcept;

        aiTextureType getType() const noexcept;
        size_t getIndex() const noexcept;
        const std::string& getPath() const noexcept;
        aiTextureMapping getMapping() const noexcept;
        aiTextureOp getOperation() const noexcept;
        aiTextureMapMode getMapMode() const noexcept;
        float getBlend() const noexcept;
        size_t getCoordIndex() const noexcept;

    private:
        aiTextureType _type;
        size_t _index;
        aiSceneRef _sceneRef;
        std::string _path;
        size_t _coordIndex;
        float _blend;
        aiTextureMapping _mapping;
        aiTextureOp _operation;
        aiTextureMapMode _mapMode;
    };

    class AssimpMaterialTextureCollection final : public VectorReadOnlyCollection<AssimpMaterialTexture>
    {
    public:
        AssimpMaterialTextureCollection(const aiMaterial& material, aiTextureType type) noexcept;
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

    class Material;

    class AssimpMaterial final
    {
    public:
        AssimpMaterial(const aiMaterial& material, const aiSceneRef& scene, const std::string& basePath) noexcept;

        std::string_view getName() const noexcept;
        const AssimpMaterialTextureCollection& getTextures(aiTextureType type) const noexcept;
        const AssimpMaterialPropertyCollection& getProperties() const noexcept;
        std::optional<Color> getColor(AssimpMaterialColorType type) const noexcept;
        bool showWireframe() const noexcept;
        aiShadingMode getShadingMode() const noexcept;
        float getOpacity() const noexcept;

        std::shared_ptr<Material> load(ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept;

    private:
        aiSceneRef _scene;
        std::string _basePath;
        const aiMaterial& _material;
        AssimpMaterialPropertyCollection _properties;
        mutable std::unordered_map<aiTextureType, AssimpMaterialTextureCollection> _textures;
        static const std::unordered_map<AssimpMaterialColorType, MaterialColorType> _materialColors;
        static const std::unordered_map<aiTextureType, MaterialTextureType> _materialTextures;

        std::shared_ptr<Texture> loadEmbeddedTexture(const std::string& path, bx::AllocatorI& alloc) const noexcept;

    };

    class AssimpMeshFaceIndexCollection final : public ReadOnlyCollection<VertexIndex>
    {
    public:
        AssimpMeshFaceIndexCollection(const aiFace& face, const aiSceneRef& scene) noexcept;
        
        size_t size() const noexcept override;
        const VertexIndex& operator[](size_t pos) const noexcept override;
        VertexIndex& operator[](size_t pos) noexcept override;
    private:
        const aiFace& _face;
        aiSceneRef _scene;
    };

    class AssimpMeshFace final
    {
    public:
        AssimpMeshFace(const aiFace& face, const aiSceneRef& scene) noexcept;

        size_t size() const noexcept;
        bool empty() const noexcept;
        const AssimpMeshFaceIndexCollection& getIndices() const noexcept;
    private:
        const aiFace& _face;
        aiSceneRef _scene;
        AssimpMeshFaceIndexCollection _indices;

    };

    class AssimpVector3Collection final : public VectorReadOnlyCollection<glm::vec3>
    {
    public:
        AssimpVector3Collection(aiVector3D* vectors, size_t size) noexcept;
    private:
        aiSceneRef _sceneRef;
    };

    class AssimpColorCollection final : public VectorReadOnlyCollection<Color>
    {
    public:
        AssimpColorCollection(aiColor4D* colors, size_t size) noexcept;
    };

    class AssimpTextureCoords final
    {
    public:
        AssimpTextureCoords(const aiMesh& mesh, size_t pos) noexcept;
        size_t getCompCount() const noexcept;
        const AssimpVector3Collection& getCoords() const noexcept;
    private:
        size_t _compCount;
        AssimpVector3Collection _coords;
    };

    class AssimpMeshFaceCollection final : public VectorReadOnlyCollection<AssimpMeshFace>
    {
    public:
        AssimpMeshFaceCollection(const aiMesh& mesh, const aiSceneRef& scene) noexcept;
    };

    class AssimpMeshTextureCoordsCollection final : public VectorReadOnlyCollection<AssimpTextureCoords>
    {
    public:
        AssimpMeshTextureCoordsCollection(const aiMesh& mesh) noexcept;
    };

    class AssimpMeshColorsCollection final : public VectorReadOnlyCollection<AssimpColorCollection>
    {
    public:
        AssimpMeshColorsCollection(const aiMesh& mesh) noexcept;
    };

    class Mesh;

    class AssimpMesh final
    {
    public:
        AssimpMesh(const aiMesh& mesh, const AssimpMaterial& material, const aiSceneRef& scene) noexcept;

        const AssimpMaterial& getMaterial() const noexcept;
        const AssimpVector3Collection& getPositions() const noexcept;
        const AssimpVector3Collection& getNormals() const noexcept;
        const AssimpVector3Collection& getTangents() const noexcept;
        const AssimpVector3Collection& getBitangents() const noexcept;
        const AssimpMeshTextureCoordsCollection& getTexCoords() const noexcept;
        const AssimpMeshFaceCollection& getFaces() const noexcept;
        const AssimpMeshColorsCollection& getColors() const noexcept;
        const size_t getVertexCount() const noexcept;

        std::shared_ptr<Mesh> load(const bgfx::VertexLayout& layout, ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept;

    private:
        const aiMesh& _mesh;
        AssimpMaterial _material;
        aiSceneRef _scene;
        AssimpVector3Collection _positions;
        AssimpVector3Collection _normals;
        AssimpVector3Collection _tangents;
        AssimpVector3Collection _bitangents;
        AssimpMeshTextureCoordsCollection _texCoords;
        AssimpMeshFaceCollection _faces;
        AssimpMeshColorsCollection _colors;

        Data createVertexData(const bgfx::VertexLayout& layout) const noexcept;
        std::vector<VertexIndex> createIndexData() const noexcept;
    };

    class AssimpMeshCollection;

    class AssimpNodeMeshCollection final : public VectorReadOnlyCollection<AssimpMesh>
    {
    public:
        AssimpNodeMeshCollection(const aiNode& node, const AssimpMeshCollection& meshes) noexcept;

        std::vector<std::shared_ptr<Mesh>> load(const bgfx::VertexLayout& layout, ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept;
    private:
    };

    class AssimpCamera final
    {
    public:
        AssimpCamera(const aiCamera& cam, const aiSceneRef& scene) noexcept;

        std::string_view getName() const noexcept;
        const glm::mat4& getProjectionMatrix() const noexcept;
        const glm::mat4& getViewMatrix() const noexcept;
        float getAspect() const noexcept;
        float getClipFar() const noexcept;
        float getClipNear() const noexcept;
        float getHorizontalFieldOfView() const noexcept;
        float getOrthographicWidth() const noexcept;
        glm::vec3 getLookAt() const noexcept;
        glm::vec3 getPosition() const noexcept;
        glm::vec3 getUp() const noexcept;
    private:
        const aiCamera& _cam;
        aiSceneRef _scene;
        glm::mat4 _view;
        glm::mat4 _proj;
    };

    class AssimpLight final
    {
    public:
        AssimpLight(const aiLight& light, const aiSceneRef& scene) noexcept;

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
        const aiLight& _light;
        aiSceneRef _scene;
    };

    class AssimpScene;
    class AssimpNode;

    class AssimpNodeChildrenCollection final : public VectorReadOnlyCollection<AssimpNode>
    {
    public:
        AssimpNodeChildrenCollection(const aiNode& node, const AssimpScene& scene) noexcept;
    };

    class AssimpNode final
    {
    public:
        AssimpNode(const aiNode& node, const AssimpScene& scene) noexcept;
        
        std::string_view getName() const noexcept;
        glm::mat4 getTransform() const noexcept;
        const AssimpNodeMeshCollection& getMeshes() const noexcept;
        const AssimpNodeChildrenCollection& getChildren() const noexcept;
        OptionalRef<const AssimpNode> getChild(const std::string_view& path) const noexcept;
        OptionalRef<const AssimpCamera> getCamera() const noexcept;
        OptionalRef<const AssimpLight> getLight() const noexcept;

    private:
        const aiNode& _node;
        std::optional<AssimpCamera> _camera;
        std::optional<AssimpLight> _light;
        AssimpNodeMeshCollection _meshes;
        AssimpNodeChildrenCollection _children;
    };

    class AssimpCameraCollection final : public VectorReadOnlyCollection<AssimpCamera>
    {
    public:
        AssimpCameraCollection(const aiSceneRef& scene) noexcept;

        OptionalRef<const AssimpCamera> get(const std::string_view& name) const noexcept;
    };

    class AssimpLightCollection final : public VectorReadOnlyCollection<AssimpLight>
    {
    public:
        AssimpLightCollection(const aiSceneRef& scene) noexcept;

        OptionalRef<const AssimpLight> get(const std::string_view& name) const noexcept;
    };

    class AssimpMaterialCollection final : public VectorReadOnlyCollection<AssimpMaterial>
    {
    public:
        AssimpMaterialCollection(const aiSceneRef& scene, const std::string& basePath) noexcept;
    };

    class AssimpMeshCollection final : public VectorReadOnlyCollection<AssimpMesh>
    {
    public:
        AssimpMeshCollection(const aiSceneRef& scene, const AssimpMaterialCollection& materials) noexcept;
    };

    class AssimpScene final
	{
    public:
        AssimpScene(const aiSceneRef& scene, const std::string& path = {}) noexcept;
        AssimpScene(Assimp::Importer& importer, const DataView& data, const std::string& path = {});

        std::string_view getName() const noexcept;
        const std::string& getPath() const noexcept;

        const AssimpNode& getRootNode() const noexcept;
        const AssimpMeshCollection& getMeshes() const noexcept;
        const AssimpMaterialCollection& getMaterials() const noexcept;
        const AssimpCameraCollection& getCameras() const noexcept;
        const AssimpLightCollection& getLights() const noexcept;

    private:
        std::string _path;
        aiSceneRef _scene;
        AssimpMaterialCollection _materials;
        AssimpMeshCollection _meshes;
        AssimpCameraCollection _cameras;
        AssimpLightCollection _lights;
        AssimpNode _rootNode;


        static aiSceneRef importScene(Assimp::Importer& importer, const DataView& data, const std::string & path);
	};
}