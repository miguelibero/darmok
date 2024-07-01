#pragma once

#include <darmok/export.h>
#include <memory>
#include <string_view>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>
#include <bx/bx.h>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/material_fwd.hpp>
#include <darmok/program_fwd.hpp>
#include <darmok/model_fwd.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/glm.hpp>

namespace darmok
{
    struct DARMOK_EXPORT ModelPointLight final
    {
        glm::vec3 attenuation;
        Color3 diffuseColor;
        Color3 specularColor;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(attenuation, diffuseColor, specularColor);
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

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(image);
        }
    };

    struct DARMOK_EXPORT ModelMaterial final
    {
        StandardProgramType standardProgram;
        std::string programName;
        MaterialPrimitiveType primitiveType;
        std::unordered_map<MaterialTextureType, std::vector<ModelTexture>> textures;
        std::unordered_map<MaterialColorType, Color> colors;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                standardProgram,
                programName,
                primitiveType,
                textures,
                colors
            );
        }
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
        std::vector<ModelArmatureJoint> joints;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(vertexData, indexData, vertexLayout, config, joints);
        }
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
    };

    struct DARMOK_EXPORT ModelNode final
    {
        std::string name;
        glm::mat4 transform;
        std::optional<ModelCamera> camera;
        std::optional<ModelPointLight> pointLight;
        std::optional<ModelAmbientLight> ambientLight;
        std::vector<ModelRenderable> renderables;
        std::vector<ModelNode> children;

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
                children);
        }

        std::string to_string() const noexcept;
    };

    struct DARMOK_EXPORT Model final
    {
        ModelNode rootNode;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(rootNode);
        }

        std::string to_string() const noexcept;

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

    struct DARMOK_EXPORT ArmatureJoint final
    {
        std::string name;
        glm::mat4 inverseBindPose;
    };

    class DARMOK_EXPORT Armature final
    {
    public:
        Armature(const std::vector<ArmatureJoint>& joints) noexcept;
        Armature(std::vector<ArmatureJoint>&& joints) noexcept;
        const std::vector<ArmatureJoint>& getJoints() const noexcept;

    private:
        std::vector<ArmatureJoint> _joints;
    };

    class DARMOK_EXPORT ModelSceneConfigurer final
    {
    public:
        ModelSceneConfigurer(Scene& scene, AssetContext& assets);
        ModelSceneConfigurer& setParent(Entity parent) noexcept;

        Entity run(const Model& model) noexcept;
        Entity run(const ModelNode& node) noexcept;

        template<typename C>
        Entity run(const ModelNode& node, C callback)
        {
            return run(node, _parent, callback);
        }

        template<typename C>
        Entity run(const Model& model, C callback)
        {
            return run(model.rootNode, callback);
        }

    private:
        ModelSceneConfig _config;
        Entity _parent;
        std::unordered_map<std::shared_ptr<ModelMaterial>, std::shared_ptr<Material>> _materials;
        std::unordered_map<std::shared_ptr<ModelMesh>, std::shared_ptr<Mesh>> _meshes;
        std::unordered_map<std::shared_ptr<ModelMesh>, std::shared_ptr<Armature>> _armatures;
        std::unordered_map<std::shared_ptr<ModelImage>, std::shared_ptr<Texture>> _textures;

        Entity add(const ModelNode& node, Entity parent) noexcept;
        Entity run(const ModelNode& node, Entity parent) noexcept;

        template<typename C>
        Entity run(const ModelNode& node, Entity parent, C callback)
        {
            auto entity = add(node, parent);
            callback(node, entity);
            for (auto& child : node.children)
            {
                run(child, entity, callback);
            }
            return entity;
        }

        std::shared_ptr<Material> loadMaterial(const std::shared_ptr<ModelMaterial>& modelMat) noexcept;
        std::shared_ptr<Mesh> loadMesh(const std::shared_ptr<ModelMesh>& modelMesh) noexcept;
        std::shared_ptr<Armature> loadArmature(const std::shared_ptr<ModelMesh>& modelMesh) noexcept;
        std::shared_ptr<Texture> loadTexture(const std::shared_ptr<ModelImage>& modelImg) noexcept;

        void configureEntity(const ModelNode& node, Entity entity) noexcept;
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

    class DARMOK_EXPORT Skinnable
    {
    public:
        Skinnable(const std::shared_ptr<Armature>& armature = nullptr) noexcept;
        std::shared_ptr<Armature> getArmature() const noexcept;
        void setArmature(const std::shared_ptr<Armature>& armature) noexcept;
    private:
        std::shared_ptr<Armature> _armature;
    };
}

DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelNode& node);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::Model& model);