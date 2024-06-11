#pragma once

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
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <glm/glm.hpp>


// to allow serialization
#include <cereal/types/optional.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/vertex_layout.hpp>
#include <darmok/math.hpp>


namespace darmok
{
    struct ModelPointLight final
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

    struct ModelAmbientLight final
    {
        float intensity;
        Color3 color;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(intensity, color);
        }
    };

    struct ModelCamera final
    {
		glm::mat4 projection;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(projection);
        }
    };

    struct ModelImage final
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

    struct ModelTexture final
    {
        std::shared_ptr<ModelImage> image;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(image);
        }
    };

    struct ModelMaterial final
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

    struct ModelArmatureJoint final
    {
        std::string name;
        glm::mat4 inverseBindPose;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(name, inverseBindPose);
        }
    };

    struct ModelMesh final
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

    struct ModelRenderable final
    {
        std::shared_ptr<ModelMesh> mesh;
        std::shared_ptr<ModelMaterial> material;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(mesh, material);
        }
    };

    struct ModelNode final
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

    struct Model final
    {
        ModelNode rootNode;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(rootNode);
        }

        std::string to_string() const noexcept;
    };

    class AssetContext;
    class Scene;

    struct ModelSceneConfig final
    {
        Scene& scene;
        AssetContext& assets;
    };

    class Mesh;
    class Material;
    class Armature;

    class ModelSceneConfigurer final
    {
    public:
        DLLEXPORT ModelSceneConfigurer(Scene& scene, AssetContext& assets);
        DLLEXPORT ModelSceneConfigurer& setParent(Entity parent) noexcept;

        DLLEXPORT Entity run(const Model& model) noexcept;
        DLLEXPORT Entity run(const ModelNode& node) noexcept;

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

        DLLEXPORT Entity add(const ModelNode& node, Entity parent) noexcept;
        DLLEXPORT Entity run(const ModelNode& node, Entity parent) noexcept;

        template<typename C>
        Entity run(const ModelNode& node, Entity parent, C callback)
        {
            auto entity = add(node, parent);
            callback(node, entity);
            for (auto child : node.children)
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

    class BX_NO_VTABLE IModelLoader
	{
	public:
        using result_type = std::shared_ptr<Model>;

        DLLEXPORT virtual ~IModelLoader() = default;
		DLLEXPORT virtual [[nodiscard]] result_type operator()(std::string_view name) = 0;
	};

    class IDataLoader;

    class BinaryModelLoader : public IModelLoader
	{
	public:
        BinaryModelLoader(IDataLoader& dataLoader) noexcept;
        DLLEXPORT [[nodiscard]] std::shared_ptr<Model> operator()(std::string_view name) override;
    private:
        IDataLoader& _dataLoader;
	};
}

DLLEXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelNode& node);
DLLEXPORT std::ostream& operator<<(std::ostream& out, const darmok::Model& model);