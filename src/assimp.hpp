#pragma once

#include <string>
#include <unordered_map>
#include <optional>
#include <vector>
#include <memory>

#include <darmok/data.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/color_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/collection.hpp>

#include <assimp/light.h>
#include <assimp/material.h>
#include <glm/glm.hpp>

#if DARMOK_OZZ
    #include <ozz/animation/offline/raw_skeleton.h>
#endif

template <typename TReal>
class aiVector3t;
typedef aiVector3t<float> aiVector3D;
struct aiMesh;
struct aiVertexWeight;
struct aiBone;
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
        AssimpMaterialProperty(const aiMaterialProperty& prop, aiSceneRef scene) noexcept;

        [[nodiscard]] std::string_view getKey() const noexcept;
        [[nodiscard]] aiPropertyTypeInfo getType() const noexcept;
        [[nodiscard]] aiTextureType getTextureType() const noexcept;
        [[nodiscard]] size_t getTextureIndex() const noexcept;
        [[nodiscard]] const DataView& getData() const noexcept;
    private:
        OptionalRef<const aiMaterialProperty> _prop;
        DataView _data;
        aiSceneRef _scene;
    };

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
        std::string _path;
        size_t _coordIndex;
        float _blend;
        aiTextureMapping _mapping;
        aiTextureOp _operation;
        aiTextureMapMode _mapMode;
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
    class Texture;
    class ITextureLoader;

    class AssimpMaterial final
    {
    public:
        AssimpMaterial(const aiMaterial& material, aiSceneRef scene, const std::string& basePath) noexcept;

        std::string_view getName() const noexcept;
        std::vector<AssimpMaterialTexture> getTextures(aiTextureType type) const noexcept;
        const std::vector<AssimpMaterialProperty>& getProperties() const noexcept;
        std::optional<Color> getColor(AssimpMaterialColorType type) const noexcept;
        bool showWireframe() const noexcept;
        aiShadingMode getShadingMode() const noexcept;
        float getOpacity() const noexcept;

        std::shared_ptr<Material> load(ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept;

    private:
        aiSceneRef _scene;
        std::string _basePath;
        OptionalRef<const aiMaterial> _material;
        std::vector<AssimpMaterialProperty> _properties;

        static const std::unordered_map<AssimpMaterialColorType, MaterialColorType> _materialColors;
        static const std::unordered_map<aiTextureType, MaterialTextureType> _materialTextures;

        std::shared_ptr<Texture> loadEmbeddedTexture(const std::string& path, bx::AllocatorI& alloc) const noexcept;

        void fillTextures(std::vector<AssimpMaterialTexture>& textures, aiTextureType type) const noexcept;
    };


    class AssimpVector3Collection final : public ValCollection<glm::vec3>
    {
    public:
        AssimpVector3Collection(const aiVector3D* ptr, size_t size) noexcept;

        [[nodiscard]] size_t size() const noexcept override;
        [[nodiscard]] glm::vec3 operator[](size_t pos) const override;
    private:
        const aiVector3D* _ptr;
        size_t _size;
    };

    class AssimpColorCollection final : public ValCollection<Color>
    {
    public:
        AssimpColorCollection(const aiColor4D* ptr, size_t size) noexcept;

        [[nodiscard]] size_t size() const noexcept override;
        [[nodiscard]] Color operator[](size_t pos) const override;
    private:
        const aiColor4D* _ptr;
        size_t _size;
    };

    class AssimpMeshFace final : public ValCollection<VertexIndex>
    {
    public:
        AssimpMeshFace(const aiFace& face, aiSceneRef scene) noexcept;
        size_t size() const noexcept override;
        VertexIndex operator[](size_t pos) const override;
    private:
        OptionalRef<const aiFace> _face;
        aiSceneRef _scene;
    };

    class AssimpTextureCoords final
    {
    public:
        AssimpTextureCoords(const aiMesh& mesh, size_t pos, aiSceneRef scene) noexcept;
        size_t getCompCount() const noexcept;
        AssimpVector3Collection getCoords() const noexcept;
    private:
        size_t _compCount;
        aiSceneRef _scene;
        AssimpVector3Collection _coords;
    };

    struct AssimpVertexWeight final
    {
        unsigned int vertexIndex;
        float value;
    };

    class AssimpVertexWeightCollection final : public ValCollection<AssimpVertexWeight>
    {
    public:
        AssimpVertexWeightCollection(const aiVertexWeight* ptr, size_t size) noexcept;

        [[nodiscard]] size_t size() const noexcept override;
        [[nodiscard]] AssimpVertexWeight operator[](size_t pos) const override;
    private:
        const aiVertexWeight* _ptr;
        size_t _size;
    };

    class AssimpBone final
    {
    public:
        AssimpBone(const aiBone& bone, aiSceneRef scene) noexcept;
        std::string_view getName() const noexcept;
        glm::mat4 getInverseBindPoseMatrix() const noexcept;
        const AssimpVertexWeightCollection& getWeights() const noexcept;
    private:
        OptionalRef<const aiBone> _bone;
        aiSceneRef _scene;
        AssimpVertexWeightCollection _weights;
    };

    class IMesh;
    class Armature;
    struct ArmatureBone;

    class AssimpMesh final
    {
    public:
        AssimpMesh(const aiMesh& mesh, const std::shared_ptr<AssimpMaterial>& material, aiSceneRef scene) noexcept;

        std::shared_ptr<AssimpMaterial> getMaterial() const noexcept;
        const AssimpVector3Collection& getPositions() const noexcept;
        const AssimpVector3Collection& getNormals() const noexcept;
        const AssimpVector3Collection& getTangents() const noexcept;
        const AssimpVector3Collection& getBitangents() const noexcept;
        const std::vector<AssimpTextureCoords>& getTexCoords() const noexcept;
        const std::vector<AssimpMeshFace>& getFaces() const noexcept;
        const std::vector<AssimpColorCollection>& getColors() const noexcept;
        const std::vector<AssimpBone>& getBones() const noexcept;
        const size_t getVertexCount() const noexcept;

        bool hasBones() const noexcept;

        std::shared_ptr<IMesh> load(const bgfx::VertexLayout& layout, ITextureLoader& textureLoader, bx::AllocatorI& alloc) const noexcept;
        std::shared_ptr<Armature> loadArmature() const noexcept;

    private:
        OptionalRef<const aiMesh> _mesh;
        std::shared_ptr<AssimpMaterial> _material;
        aiSceneRef _scene;
        AssimpVector3Collection _positions;
        AssimpVector3Collection _normals;
        AssimpVector3Collection _tangents;
        AssimpVector3Collection _bitangents;
        std::vector<AssimpTextureCoords> _texCoords;
        std::vector<AssimpMeshFace> _faces;
        std::vector<AssimpColorCollection> _colors;
        std::vector<AssimpBone> _bones;

        Data createVertexData(const bgfx::VertexLayout& layout, bx::AllocatorI& alloc) const noexcept;
        std::vector<VertexIndex> createIndexData() const noexcept;
        std::vector<ArmatureBone> createArmatureBones() const noexcept;
    };

    class AssimpCamera final
    {
    public:
        AssimpCamera(const aiCamera& cam, aiSceneRef scene) noexcept;

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
        OptionalRef<const aiCamera> _cam;
        aiSceneRef _scene;
        glm::mat4 _view;
        glm::mat4 _proj;
    };

    class AssimpLight final
    {
    public:
        AssimpLight(const aiLight& light, aiSceneRef scene) noexcept;

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
        OptionalRef<const aiLight> _light;
        aiSceneRef _scene;
    };

    class AssimpScene;

    class AssimpNode final
    {
    public:
        using ParentRef = std::optional<std::weak_ptr<AssimpNode>>;

        AssimpNode(const aiNode& node, const AssimpScene& scene, const ParentRef& parent = std::nullopt) noexcept;
        void loadChildren(const std::weak_ptr<AssimpNode> selfPtr, const AssimpScene& scene) noexcept;

        const aiNode& getAssimp() const noexcept;
        std::string_view getName() const noexcept;
        glm::mat4 getTransform() const noexcept;
        glm::mat4 getWorldTransform() const noexcept;
        static glm::mat4 getWorldTransform(const aiNode* node) noexcept;
        const std::vector<std::shared_ptr<AssimpMesh>>& getMeshes() const noexcept;
        const std::vector<std::shared_ptr<AssimpNode>>& getChildren() const noexcept;
        ParentRef getParent() const noexcept;
        std::shared_ptr<AssimpNode> findChildByPath(std::string_view path) const noexcept;
        std::shared_ptr<AssimpNode> findChildByName(std::string_view name) const noexcept;
        std::shared_ptr<AssimpCamera> getCamera() const noexcept;
        std::shared_ptr<AssimpLight> getLight() const noexcept;

    private:
        OptionalRef<const aiNode> _node;
        aiSceneRef _scene;
        ParentRef _parent;
        std::shared_ptr<AssimpCamera> _camera;
        std::shared_ptr<AssimpLight> _light;
        std::vector<std::shared_ptr<AssimpMesh>> _meshes;
        std::vector<std::shared_ptr<AssimpNode>> _children;
        mutable std::optional<glm::mat4> _worldTransform;
    };

    class AssimpScene final
	{
    public:
        AssimpScene(aiSceneRef scene, const std::string& path = {}) noexcept;
        AssimpScene(Assimp::Importer& importer, const DataView& data, const std::string& path = {});

        std::string_view getName() const noexcept;
        const std::string& getPath() const noexcept;
        aiSceneRef getInternal() const noexcept;

#if DARMOK_OZZ
        ozz::animation::offline::RawSkeleton createSkeleton() const;
#endif

        std::shared_ptr<AssimpNode> getRootNode() const noexcept;
        const std::vector<std::shared_ptr<AssimpMaterial>>& getMaterials() const noexcept;
        const std::vector<std::shared_ptr<AssimpMesh>>& getMeshes() const noexcept;
        const std::vector<std::shared_ptr<AssimpCamera>>& getCameras() const noexcept;
        const std::vector<std::shared_ptr<AssimpLight>>& getLights() const noexcept;

        std::shared_ptr<AssimpCamera> getCamera(std::string_view name) const noexcept;
        std::shared_ptr<AssimpLight> getLight(std::string_view name) const noexcept;

    private:
        std::string _path;
        aiSceneRef _scene;
        std::vector<std::shared_ptr<AssimpMaterial>> _materials;
        std::vector<std::shared_ptr<AssimpMesh>> _meshes;
        std::vector<std::shared_ptr<AssimpCamera>> _cameras;
        std::vector<std::shared_ptr<AssimpLight>> _lights;
        std::shared_ptr<AssimpNode> _rootNode;

        static std::vector<std::shared_ptr<AssimpMaterial>> loadMaterials(aiSceneRef scene, const std::string& path) noexcept;
        static std::vector<std::shared_ptr<AssimpMesh>> loadMeshes(aiSceneRef scene, const std::vector<std::shared_ptr<AssimpMaterial>>& materials) noexcept;
        static std::vector<std::shared_ptr<AssimpCamera>> loadCameras(aiSceneRef scene) noexcept;
        static std::vector<std::shared_ptr<AssimpLight>> loadLights(aiSceneRef scene) noexcept;

        static aiSceneRef importScene(Assimp::Importer& importer, const DataView& data, const std::string & path);
	};
}