#pragma once

#include <darmok/export.h>
#include <memory>
#include <string_view>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <bx/bx.h>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/model_fwd.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/glm.hpp>
#include <darmok/program_fwd.hpp>
#include <darmok/shape.hpp>

namespace darmok
{
    struct DARMOK_EXPORT ModelPointLight final
    {
        float intensity;
        Color3 color;
        float radius;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(intensity, color, radius);
        }
    };

    struct DARMOK_EXPORT ModelSpotLight final
    {
        float intensity;
        Color3 color;
        float innerAngle = 0.F;
        float outerAngle = 0.F;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(intensity, color, innerAngle, outerAngle);
        }
    };

    struct DARMOK_EXPORT ModelDirectionalLight final
    {
        float intensity;
        Color3 color;
        glm::vec3 direction;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(intensity, color, direction);
        }
    };

    struct DARMOK_EXPORT ModelAmbientLight final
    {
        float intensity;
        Color3 color;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(intensity, color);
        }
    };

    struct DARMOK_EXPORT ModelCamera final
    {
		glm::mat4 projection;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(projection);
        }
    };

    struct DARMOK_EXPORT ModelImage final
    {
        Data data;
        std::string name;
        TextureConfig config;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(data, name, config);
        }
    };

    struct DARMOK_EXPORT ModelTexture final
    {
        std::shared_ptr<ModelImage> image;
        TextureSamplingMode samplingMode = TextureSamplingMode::Anisotropic;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(image, samplingMode);
        }
    };

    using ProgramDefines = std::unordered_set<std::string>;

    struct DARMOK_EXPORT ModelMaterial final
    {
        std::string program;
        StandardProgramType standardProgram = StandardProgramType::Unlit;
        ProgramDefines programDefines;

        MaterialPrimitiveType primitiveType = MaterialPrimitiveType::Triangle;
        std::unordered_map<MaterialTextureType, ModelTexture> textures;

        Color baseColor = Colors::white();
        Color specularColor = Colors::white();
        float shininess = 0.F;
        float metallicFactor = 0.F;
        float roughnessFactor = 0.F;
        float normalScale = 0.F;
        float occlusionStrength = 0.F;
        Color3 emissiveColor = Colors::black();
        bool twoSided = false;

        OpacityType opacityType = OpacityType::Transparent;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                program,
                standardProgram,
                programDefines,
                primitiveType,
                textures,
                baseColor,
                specularColor,
                shininess,
                metallicFactor,
                roughnessFactor,
                normalScale,
                occlusionStrength,
                emissiveColor,
                twoSided,
                opacityType
            );
        }

        std::string toString() const noexcept;
    };

    struct DARMOK_EXPORT ModelArmatureJoint final
    {
        std::string name;
        glm::mat4 inverseBindPose;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(name, inverseBindPose);
        }
    };

    struct DARMOK_EXPORT ModelMesh final
    {
        Data vertexData;
        Data indexData;
        bgfx::VertexLayout vertexLayout;
        MeshConfig config;
        BoundingBox boundingBox;
        std::vector<ModelArmatureJoint> joints;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(vertexData, indexData, vertexLayout, config, boundingBox, joints);
        }

        std::string toString() const noexcept;
        bool empty() const noexcept;
    };

    struct DARMOK_EXPORT ModelRenderable final
    {
        std::shared_ptr<ModelMesh> mesh;
        std::shared_ptr<ModelMaterial> material;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(mesh, material);
        }

        std::string toString() const noexcept;
    };

    struct DARMOK_EXPORT ModelNode final
    {
        std::string name;
        glm::mat4 transform;
        std::optional<ModelCamera> camera;
        std::optional<ModelPointLight> pointLight;
        std::optional<ModelSpotLight> spotLight;
        std::optional<ModelDirectionalLight> dirLight;
        std::optional<ModelAmbientLight> ambientLight;
        std::vector<ModelRenderable> renderables;
        std::vector<ModelNode> children;
        std::unordered_set<std::string> tags;

        BoundingBox getBoundingBox() const noexcept;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                name,
                transform,
                camera,
                pointLight,
                ambientLight,
                renderables,
                children,
                tags);
        }

        std::string toString() const noexcept;
    };

    struct DARMOK_EXPORT Model final
    {
        ModelNode rootNode;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(rootNode);
        }

        std::string toString() const noexcept;

        using DataFormat = ModelDataFormat;

        static DataFormat getFormat(const std::string& name) noexcept;
        static DataFormat getExtensionFormat(const std::string& ext) noexcept;
        static std::string getFormatExtension(DataFormat format) noexcept;
        static std::string getFormatName(DataFormat format) noexcept;
        void read(std::istream& in, DataFormat format);
        void write(std::ostream& out, DataFormat format) const;
    };

    class AssetContext;
    class Scene;

    struct DARMOK_EXPORT ModelSceneConfig final
    {
        Scene& scene;
        AssetContext& assets;
    };

    class Mesh;
    class Material;
    class Armature;
    class Transform;

    class DARMOK_EXPORT ModelSceneConfigurer final
    {
    public:
        ModelSceneConfigurer(Scene& scene, AssetContext& assets);
        ModelSceneConfigurer& setParent(Entity parent) noexcept;
        ModelSceneConfigurer& setTextureFlags(uint64_t flags) noexcept;

        Entity operator()(const Model& model) noexcept;
        Entity operator()(const ModelNode& node) noexcept;

        template<typename C>
        Entity operator()(const ModelNode& node, C callback)
        {
            return operator()(node, _parent, callback);
        }

        template<typename C>
        Entity operator()(const Model& model, C callback)
        {
            return operator()(model.rootNode, callback);
        }

    private:
        ModelSceneConfig _config;
        Entity _parent;
        uint64_t _textureFlags;
        std::unordered_map<std::shared_ptr<ModelMaterial>, std::shared_ptr<Material>> _materials;
        std::unordered_map<std::shared_ptr<ModelMesh>, std::shared_ptr<Mesh>> _meshes;
        std::unordered_map<std::shared_ptr<ModelMesh>, std::shared_ptr<Armature>> _armatures;
        std::unordered_map<std::shared_ptr<ModelImage>, std::shared_ptr<Texture>> _textures;

        Entity add(const ModelNode& node, Entity parent) noexcept;
        Entity operator()(const ModelNode& node, Entity parent) noexcept;

        template<typename C>
        Entity operator()(const ModelNode& node, Entity parent, C callback)
        {
            auto entity = add(node, parent);
            callback(node, entity);
            for (auto& child : node.children)
            {
                operator()(child, entity, callback);
            }
            return entity;
        }

        std::shared_ptr<Material> loadMaterial(const std::shared_ptr<ModelMaterial>& modelMat) noexcept;
        std::shared_ptr<Mesh> loadMesh(const std::shared_ptr<ModelMesh>& modelMesh) noexcept;
        std::shared_ptr<Armature> loadArmature(const std::shared_ptr<ModelMesh>& modelMesh) noexcept;
        std::shared_ptr<Texture> loadTexture(const std::shared_ptr<ModelImage>& modelImg) noexcept;

        void configureEntity(const ModelNode& node, Entity entity, Transform& trans) noexcept;
        void configureEntity(const ModelRenderable& renderable, Entity entity) noexcept;
        void configureEntity(const ModelCamera& cam, Entity entity) noexcept;
        void configureEntity(const ModelPointLight& light, Entity entity) noexcept;
        void configureEntity(const ModelAmbientLight& light, Entity entity) noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IModelLoader
	{
	public:
        using result_type = std::shared_ptr<Model>;

        virtual ~IModelLoader() = default;
		virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
	};

    class IDataLoader;

    class DARMOK_EXPORT BinaryModelLoader : public IModelLoader
	{
	public:
        BinaryModelLoader(IDataLoader& dataLoader) noexcept;
        [[nodiscard]] result_type operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
	};
}

DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelMaterial& material);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelMesh& mesh);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelRenderable& renderable);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelNode& node);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::Model& model);