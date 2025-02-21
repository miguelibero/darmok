#pragma once

#include <darmok/export.h>
#include <darmok/data.hpp>
#include <darmok/color.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/material.hpp>
#include <darmok/model_fwd.hpp>
#include <darmok/texture.hpp>
#include <darmok/mesh.hpp>
#include <darmok/glm.hpp>
#include <darmok/program_fwd.hpp>
#include <darmok/shape.hpp>
#include <darmok/loader.hpp>
#include <darmok/glm_serialize.hpp>

#include <memory>
#include <string_view>
#include <vector>
#include <variant>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include <cereal/cereal.hpp>

// to allow serialization
#include <cereal/types/optional.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

namespace darmok
{
    struct DARMOK_EXPORT ModelPointLight final
    {
        float intensity;
        Color3 color;
        float range;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(intensity),
                CEREAL_NVP(color),
                CEREAL_NVP(range)
            );
        }
    };

    struct DARMOK_EXPORT ModelSpotLight final
    {
        float intensity;
        Color3 color;
        float range;
        float innerConeAngle = 0.F;
        float coneAngle = 0.F;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(intensity),
                CEREAL_NVP(color),
                CEREAL_NVP(range),
                CEREAL_NVP(coneAngle),
                CEREAL_NVP(innerConeAngle)
            );
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
            archive(
                CEREAL_NVP(intensity),
                CEREAL_NVP(color),
                CEREAL_NVP(direction)
            );
        }
    };

    struct DARMOK_EXPORT ModelAmbientLight final
    {
        float intensity;
        Color3 color;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(intensity),
                CEREAL_NVP(color)
            );
        }
    };

    struct DARMOK_EXPORT ModelCamera final
    {
		glm::mat4 projection;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(projection));
        }
    };

    struct DARMOK_EXPORT ModelArmatureJoint final
    {
        std::string name;
        glm::mat4 inverseBindPose;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(name),
                CEREAL_NVP(inverseBindPose)
            );
        }
    };

    struct DARMOK_EXPORT ModelMesh final
    {
        std::string name;
        Data vertexData;
        Data indexData;
        bgfx::VertexLayout vertexLayout;
        MeshConfig config;
        BoundingBox boundingBox;
        std::vector<ModelArmatureJoint> joints;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(name),
                CEREAL_NVP(vertexData),
                CEREAL_NVP(indexData),
                CEREAL_NVP(vertexLayout),
                CEREAL_NVP(config),
                CEREAL_NVP(boundingBox),
                CEREAL_NVP(joints)
            );
        }

        std::string toString() const noexcept;
        bool empty() const noexcept;
    };

    struct DARMOK_EXPORT ModelRenderable final
    {
        std::shared_ptr<ModelMesh> mesh;
        std::shared_ptr<MaterialDefinition> material;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP(mesh),
                CEREAL_NVP(material)
            );
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
                CEREAL_NVP(name),
                CEREAL_NVP(transform),
                CEREAL_NVP(camera),
                CEREAL_NVP(pointLight),
                CEREAL_NVP(ambientLight),
                CEREAL_NVP(renderables),
                CEREAL_NVP(children),
                CEREAL_NVP(tags)
            );
        }

        std::string toString() const noexcept;
    };

    struct DARMOK_EXPORT Model final
    {
        std::string name;
        ModelNode rootNode;

        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(name), CEREAL_NVP(rootNode));
        }

        std::string toString() const noexcept;
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
        std::unordered_map<std::shared_ptr<ModelMesh>, std::shared_ptr<Mesh>> _meshes;
        std::unordered_map<std::shared_ptr<ModelMesh>, std::shared_ptr<Armature>> _armatures;

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

        std::shared_ptr<Material> loadMaterial(const std::shared_ptr<MaterialDefinition>& matDef) noexcept;
        std::shared_ptr<Mesh> loadMesh(const std::shared_ptr<ModelMesh>& modelMesh) noexcept;
        std::shared_ptr<Armature> loadArmature(const std::shared_ptr<ModelMesh>& modelMesh) noexcept;

        void configureEntity(const ModelNode& node, Entity entity, Transform& trans) noexcept;
        void configureEntity(const ModelRenderable& renderable, Entity entity) noexcept;
        void configureEntity(const ModelCamera& cam, Entity entity) noexcept;
        void configureEntity(const ModelPointLight& light, Entity entity) noexcept;
        void configureEntity(const ModelSpotLight& light, Entity entity) noexcept;
        void configureEntity(const ModelAmbientLight& light, Entity entity) noexcept;
    };

    class DARMOK_EXPORT BX_NO_VTABLE IModelLoader : public ILoader<Model>
	{
	};

    using CerealModelLoader = CerealLoader<IModelLoader>;
}

DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::MaterialDefinition& material);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelMesh& mesh);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelRenderable& renderable);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::ModelNode& node);
DARMOK_EXPORT std::ostream& operator<<(std::ostream& out, const darmok::Model& model);