#pragma once

#include <darmok/utils.hpp>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <string>
#include <string_view>
#include <memory>
#include <glm/glm.hpp>
#include <bx/bx.h>

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
    class Texture;

    enum class ModelMaterialTextureType
    {
        None = 0,
        Diffuse = 1,
        Specular = 2,
        Ambient = 3,
        Emissive = 4,
        Height = 5,
        Normal = 6,
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
        Count
    };

    enum class ModelMaterialColorType
    {
        Diffuse,
        Specular,
        Ambient,
        Emissive,
        Transparent,
        Reflective,
        Count
    };

    enum class ModelMaterialShadingModel
    {
        Unknown,
        Flat = 0x1,
        Gourand = 0x2,
        Phong = 0x3,
        Blinn = 0x4,
        Toon = 0x5,
        OrenNayar = 0x6,
        Minnaert = 0x7,
        CookTorrance = 0x8,
        Unlit = 0x09,
        Fresnel = 0xa,
        Physicallybased = 0xb,
        Count
    };

    enum class ModelLightType
    {
        Undefined,
        Directional,
        Point,
        Spot,
        Ambient,
        Area,
        Count
    };

    enum class ModelLightColorType
    {
        Diffuse,
        Specular,
        Ambient,
        Count
    };

    class ModelMaterialProperty final
    {
    public:
        ModelMaterialProperty(aiMaterialProperty* ptr);
        [[nodiscard]] std::string_view getKey() const noexcept;
        [[nodiscard]] ModelMaterialPropertyType getType() const noexcept;
        [[nodiscard]] ModelMaterialTextureType getTextureType() const noexcept;
        [[nodiscard]] size_t getTextureIndex() const noexcept;
        [[nodiscard]] const DataView& getData() const noexcept;
    private:
        aiMaterialProperty* _ptr;
        DataView _data;
    };

    class ModelMaterialPropertyCollection final : public MemReadOnlyCollection<ModelMaterialProperty>
    {
    public:
        ModelMaterialPropertyCollection(aiMaterial* ptr);
        [[nodiscard]] size_t size() const override;
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
        size_t _coordIndex;
        float _blend;
        ModelMaterialTextureMapping _mapping;
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
        ModelMaterial(aiMaterial* ptr, const aiScene* scene = nullptr, const std::string& basePath = {},
            const OptionalRef<ITextureLoader>& textureLoader = nullptr, bx::AllocatorI* alloc = nullptr);
        std::string_view getName() const;
        ModelMaterialTextureCollection getTextures(ModelMaterialTextureType type) const;
        const ModelMaterialPropertyCollection& getProperties() const;
        std::shared_ptr<Material> load() noexcept;
        std::optional<Color> getColor(ModelMaterialColorType type) const;
        bool showWireframe() const;
        ModelMaterialShadingModel getShadingModel() const;
        float getOpacity() const;
    private:
        aiMaterial* _ptr;
        const aiScene* _scene;
        std::string _basePath;
        ModelMaterialPropertyCollection _properties;
        OptionalRef<ITextureLoader> _textureLoader;
        bx::AllocatorI* _alloc;

        std::pair<MaterialTextureType, std::shared_ptr<Texture>> createMaterialTexture(const ModelMaterialTexture& modelTexture) const;
    };

    class ModelMeshFaceIndexCollection final : public BaseReadOnlyCollection<VertexIndex>
    {
    public:
        ModelMeshFaceIndexCollection(aiFace* ptr);
        size_t size() const override;
        const VertexIndex& operator[](size_t pos) const override;
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

    class ModelVector3Collection final : public MemReadOnlyCollection<glm::vec3>
    {
    public:
        ModelVector3Collection(aiVector3D* ptr, size_t size);
        size_t size() const override;

    private:
        aiVector3D* _ptr;
        size_t _size;
        glm::vec3 create(size_t pos) const override;
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

    class ModelColorCollection final : public MemReadOnlyCollection<Color>
    {
    public:
        ModelColorCollection(aiColor4D* ptr, size_t size);
        size_t size() const override;

    private:
        aiColor4D* _ptr;
        size_t _size;
        Color create(size_t pos) const override;
    };

    class ModelMeshColorsCollection final : public MemReadOnlyCollection<ModelColorCollection>
    {
    public:
        ModelMeshColorsCollection(aiMesh* ptr);
        size_t size() const override;
    private:
        aiMesh* _ptr;
        ModelColorCollection create(size_t pos) const override;
    };

    class Model;

    class ModelMesh final
    {
    public:
        ModelMesh(aiMesh* ptr, ModelMaterial& material);

        ModelMaterial& getMaterial();
        const ModelMaterial& getMaterial() const;
        const ModelVector3Collection& getPositions() const;
        const ModelVector3Collection& getNormals() const;
        const ModelVector3Collection& getTangents() const;
        const ModelVector3Collection& getBitangents() const;
        const ModelMeshTextureCoordsCollection& getTexCoords() const;
        const ModelMeshFaceCollection& getFaces() const;
        const ModelMeshColorsCollection& getColors() const;
        const size_t getVertexCount() const;

        std::shared_ptr<Mesh> load(const bgfx::VertexLayout& layout);

    private:
        aiMesh* _ptr;
        ModelMaterial& _material;
        ModelVector3Collection _positions;
        ModelVector3Collection _normals;
        ModelVector3Collection _tangents;
        ModelVector3Collection _bitangents;
        ModelMeshTextureCoordsCollection _texCoords;
        ModelMeshFaceCollection _faces;
        ModelMeshColorsCollection _colors;
    };

    class ModelNodeMeshCollection final : public BaseReadOnlyCollection<ModelMesh>
    {
    public:
        ModelNodeMeshCollection(aiNode* ptr, Model& model);
        size_t size() const override;
        std::vector<std::shared_ptr<Mesh>> load(const bgfx::VertexLayout& layout);
        const ModelMesh& operator[](size_t pos) const override;
        ModelMesh& operator[](size_t pos) override;
    private:
        aiNode* _ptr;
        Model& _model;
    };

    class ModelCamera final
    {
    public:
        ModelCamera(aiCamera* ptr);
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

    class ModelLight final
    {
    public:
        ModelLight(aiLight* ptr) noexcept;

        void addToScene(Scene& scene, Entity entity) noexcept;
        
        [[nodiscard]] std::string_view getName() const noexcept;
        [[nodiscard]] ModelLightType getType() const noexcept;
        [[nodiscard]] float getInnerConeAngle() const noexcept;
        [[nodiscard]] float getOuterConeAngle() const noexcept;
        [[nodiscard]] glm::vec3 getAttenuation() const noexcept;
        [[nodiscard]] Color3 getColor(ModelLightColorType type) const noexcept;
        [[nodiscard]] glm::vec3 getDirection() const noexcept;
        [[nodiscard]] glm::vec3 getPosition() const noexcept;
        [[nodiscard]] glm::vec2 getSize() const noexcept;
    private:
        aiLight* _ptr;
    };

    class ModelNode;

    class ModelNodeChildrenCollection final : public MemReadOnlyCollection<ModelNode>
    {
    public:
        ModelNodeChildrenCollection(aiNode* ptr, Model& model, const std::string& basePath);
        size_t size() const override;
    private:
        aiNode* _ptr;
        Model& _model;
        std::string _basePath;
        ModelNode create(size_t pos) const override;
    };

    class ModelNode final
    {
    public:
        ModelNode(aiNode* ptr, Model& model, const std::string& basePath);
        
        std::string_view getName() const;
        glm::mat4 getTransform() const;
        const ModelNodeMeshCollection& getMeshes() const;
        const ModelNodeChildrenCollection& getChildren() const;
        OptionalRef<const ModelNode> getChild(const std::string_view& path) const;
        OptionalRef<const ModelCamera> getCamera() const;
        OptionalRef<const ModelLight> getLight() const;

        ModelNodeMeshCollection& getMeshes();
        ModelNodeChildrenCollection& getChildren();
        OptionalRef<ModelNode> getChild(const std::string_view& path);
        OptionalRef<ModelCamera> getCamera();
        OptionalRef<ModelLight> getLight();
        Entity addToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent = entt::null);

        template<typename C>
        Entity addToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent, C callback)
        {
            auto entity = doAddToScene(scene, layout, parent);
            callback(*this, entity);
            for (auto& child : getChildren())
            {
                child.addToScene(scene, layout, entity, callback);
            }
            return entity;
        }

        template<typename C>
        Entity addToScene(Scene& scene, const bgfx::VertexLayout& layout, C callback)
        {
            auto entity = doAddToScene(scene, layout);
            callback(*this, entity);
            for (auto& child : getChildren())
            {
                child.addToScene(scene, layout, entity, callback);
            }
            return entity;
        }

    private:
        aiNode* _ptr;
        ModelNodeMeshCollection _meshes;
        ModelNodeChildrenCollection _children;
        std::string _basePath;
        Model& _model;

        Entity doAddToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent = entt::null);
    };

    class ModelCameraCollection final : public MemReadOnlyCollection<ModelCamera>
    {
    public:
        ModelCameraCollection(const aiScene* ptr);
        size_t size() const override;
        OptionalRef<ModelCamera> get(const std::string_view& name);
        OptionalRef<const ModelCamera> get(const std::string_view& name) const;
    private:
        const aiScene* _ptr;
        ModelCamera create(size_t pos) const override;
    };

    class ModelLightCollection final : public MemReadOnlyCollection<ModelLight>
    {
    public:
        ModelLightCollection(const aiScene* ptr);
        size_t size() const override;
        OptionalRef<ModelLight> get(const std::string_view& name);
        OptionalRef<const ModelLight> get(const std::string_view& name) const;
    private:
        const aiScene* _ptr;
        ModelLight create(size_t pos) const override;
    };

    class ModelMaterialCollection final : public MemReadOnlyCollection<ModelMaterial>
    {
    public:
        ModelMaterialCollection(const aiScene* ptr, const std::string& basePath = {},
            const OptionalRef<ITextureLoader>& textureLoader = nullptr,
            bx::AllocatorI* alloc = nullptr);
        size_t size() const override;
    private:
        const aiScene* _ptr;
        std::string _basePath;
        OptionalRef<ITextureLoader> _textureLoader;
        bx::AllocatorI* _alloc;

        ModelMaterial create(size_t pos) const override;
    };

    class ModelMeshCollection final : public MemReadOnlyCollection<ModelMesh>
    {
    public:
        ModelMeshCollection(const aiScene* ptr, ModelMaterialCollection& materials);
        size_t size() const override;
    private:
        const aiScene* _ptr;
        ModelMaterialCollection& _materials;
        ModelMesh create(size_t pos) const override;
    };

    class Model final
	{
    public:
        Model(const aiScene* ptr, const std::string& path = {}, const OptionalRef<ITextureLoader>& textureLoader = nullptr, bx::AllocatorI* alloc = nullptr);
        std::string_view getName() const;
        const std::string& getPath() const;
        const ModelNode& getRootNode() const;
        const ModelMeshCollection& getMeshes() const;
        const ModelMaterialCollection& getMaterials() const;
        const ModelCameraCollection& getCameras() const;
        const ModelLightCollection& getLights() const;

        ModelNode& getRootNode();
        ModelMeshCollection& getMeshes();
        ModelMaterialCollection& getMaterials();
        ModelCameraCollection& getCameras();
        ModelLightCollection& getLights();
        Entity addToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent = entt::null);

        template<typename C>
        Entity addToScene(Scene& scene, const bgfx::VertexLayout& layout, Entity parent, C callback)
        {
            return getRootNode().addToScene(scene, layout, parent, callback);
        }

        template<typename C>
        Entity addToScene(Scene& scene, const bgfx::VertexLayout& layout, C callback)
        {
            return getRootNode().addToScene(scene, layout, callback);
        }

    private:
        const aiScene* _ptr;
        std::string _basePath;
        std::string _path;
        ModelNode _rootNode;
        ModelMeshCollection _meshes;
        ModelMaterialCollection _materials;
        ModelCameraCollection _cameras;
        ModelLightCollection _lights;
	};

    class BX_NO_VTABLE IModelLoader
	{
	public:
		virtual ~IModelLoader() = default;
		virtual std::shared_ptr<Model> operator()(std::string_view name) = 0;
	};
}