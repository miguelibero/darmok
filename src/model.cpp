#include <darmok/model.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include <darmok/light.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/data_stream.hpp>

#include <cereal/archives/portable_binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>

namespace darmok
{
    template<typename T>
    static std::string getModelString(const T& obj, std::string_view name) noexcept
    {
        std::stringstream out;
        out << name << ":\n";
        cereal::JSONOutputArchive archive(out);
        archive(obj);
        return out.str();
    }

    std::string ModelMaterial::toString() const noexcept
    {
        return getModelString(*this, "ModelMaterial");
    }

    std::string ModelMesh::toString() const noexcept
    {
        return getModelString(*this, "ModelMesh");
    }

    std::string ModelRenderable::toString() const noexcept
    {
        return getModelString(*this, "ModelRenderable");
    }

    std::string ModelNode::toString() const noexcept
    {
        return getModelString(*this, "ModelNode");
    }

    std::string Model::toString() const noexcept
    {
        return getModelString(*this, "Model");
    }

    bool ModelMesh::empty() const noexcept
    {
        if (vertexLayout.getStride() == 0)
        {
            return true;
        }
        if (vertexData.empty())
        {
            return true;
        }
        return false;
    }

    BoundingBox ModelNode::getBoundingBox() const noexcept
    {
        BoundingBox bounds;
        for (const auto& renderable : renderables)
        {
            if (renderable.mesh)
            {
                bounds += renderable.mesh->boundingBox;
            }
        }
        return bounds;
    }

    ModelSceneConfigurer::ModelSceneConfigurer(Scene& scene, AssetContext& assets)
        : _config{ scene, assets }
        , _parent(entt::null)
        , _textureFlags(defaultTextureLoadFlags)
    {
    }

    ModelSceneConfigurer& ModelSceneConfigurer::setParent(Entity parent) noexcept
    {
        _parent = parent;
        return *this;
    }

    ModelSceneConfigurer& ModelSceneConfigurer::setTextureFlags(uint64_t flags) noexcept
    {
        _textureFlags = flags;
        return *this;
    }

    Entity ModelSceneConfigurer::operator()(const Model& model) noexcept
    {
        return operator()(model.rootNode);
    }

    Entity ModelSceneConfigurer::operator()(const ModelNode& node) noexcept
    {
        return operator()(node, _parent);
    }

    Entity ModelSceneConfigurer::operator()(const ModelNode& node, Entity parent) noexcept
    {
        auto entity = add(node, parent);
        for (const auto& child : node.children)
        {
            operator()(child, entity);
        }
        return entity;
    }

    Entity ModelSceneConfigurer::add(const ModelNode& node, Entity parent) noexcept
    {
        auto entity = _config.scene.createEntity();
        OptionalRef<Transform> parentTrans;
		if (parent != entt::null)
		{
			parentTrans = _config.scene.getOrAddComponent<Transform>(parent);
		}
		auto& trans = _config.scene.addComponent<Transform>(entity, node.transform, parentTrans);
        configureEntity(node, entity, trans);
        return entity;
    }

    void ModelSceneConfigurer::configureEntity(const ModelNode& node, Entity entity, Transform& trans) noexcept
    {
        trans.setName(node.name);

        if (node.camera)
        {
            configureEntity(*node.camera, entity);
        }
        if (node.pointLight)
        {
            configureEntity(*node.pointLight, entity);
        }
        if (node.spotLight)
        {
            configureEntity(*node.spotLight, entity);
        }
        if (node.ambientLight)
        {
            configureEntity(*node.ambientLight, entity);
        }

        if (node.renderables.size() == 1)
        {
            configureEntity(node.renderables[0], entity);
        }
        else
        {
            for (const auto& renderable : node.renderables)
            {
                auto childEntity = _config.scene.createEntity();
                _config.scene.addComponent<Transform>(childEntity).setParent(trans);
                configureEntity(renderable, childEntity);
            }
        }
    }

    std::shared_ptr<Texture> ModelSceneConfigurer::loadTexture(const std::shared_ptr<ModelImage>& modelImg) noexcept
    {
        if (!modelImg)
        {
            return nullptr;
        }
        auto itr = _textures.find(modelImg);
        if (itr != _textures.end())
        {
            return itr->second;
        }
        std::shared_ptr<Texture> tex;
        if (modelImg->data.empty())
        {
            tex = _config.assets.getTextureLoader()(modelImg->name);
        }
        else
        {
            tex = std::make_shared<Texture>(modelImg->data, modelImg->config, _textureFlags);
        }
        _textures.emplace(modelImg, tex);
        return tex;
    }

    std::shared_ptr<Material> ModelSceneConfigurer::loadMaterial(const std::shared_ptr<ModelMaterial>& modelMat) noexcept
    {
        if (!modelMat)
        {
            return nullptr;
        }
        auto itr = _materials.find(modelMat);
        if (itr != _materials.end())
        {
            return itr->second;
        }
        std::shared_ptr<Program> prog;
        if (!modelMat->program.empty())
        {
            prog = _config.assets.getProgramLoader()(modelMat->program);
        }
        else
        {
            prog = StandardProgramLoader::load(modelMat->standardProgram);
        }
        auto mat = std::make_shared<Material>(prog);
        for (auto& [type, modelTex] : modelMat->textures)
        {
            if (modelTex.image)
            {
                mat->setTexture(type, loadTexture(modelTex.image));
            }
        }
        mat->setBaseColor(modelMat->baseColor)
            .setMetallicFactor(modelMat->metallicFactor)
            .setRoughnessFactor(modelMat->roughnessFactor)
            .setNormalScale(modelMat->normalScale)
            .setOcclusionStrength(modelMat->occlusionStrength)
            .setEmissiveColor(modelMat->emissiveColor)
            .setTwoSided(modelMat->twoSided)
            .setOpacityType(modelMat->opacityType)
            .setProgramDefines(modelMat->programDefines);

        _materials.emplace(modelMat, mat);
        return mat;
    }

    std::shared_ptr<Mesh> ModelSceneConfigurer::loadMesh(const std::shared_ptr<ModelMesh>& modelMesh) noexcept
    {
        if (!modelMesh)
        {
            return nullptr;
        }
        if (modelMesh->empty())
        {
            return nullptr;
        }
        auto itr = _meshes.find(modelMesh);
        if (itr != _meshes.end())
        {
            return itr->second;
        }
        auto mesh = std::make_shared<Mesh>(modelMesh->vertexLayout, modelMesh->vertexData, modelMesh->indexData, modelMesh->config);
        _meshes.emplace(modelMesh, mesh);
        return mesh;
    }

    std::shared_ptr<Armature> ModelSceneConfigurer::loadArmature(const std::shared_ptr<ModelMesh>& modelMesh) noexcept
    {
        if (!modelMesh || modelMesh->joints.empty())
        {
            return nullptr;
        }
        auto itr = _armatures.find(modelMesh);
        if (itr != _armatures.end())
        {
            return itr->second;
        }
        std::vector<ArmatureJoint> joints;
        for (auto& joint : modelMesh->joints)
        {
            joints.emplace_back(joint.name, joint.inverseBindPose);
        }
        auto armature = std::make_shared<Armature>(joints);
        _armatures.emplace(modelMesh, armature);
        return armature;
    }

    void ModelSceneConfigurer::configureEntity(const ModelRenderable& modelRenderable, Entity entity) noexcept
    {
        auto mat = loadMaterial(modelRenderable.material);
        auto mesh = loadMesh(modelRenderable.mesh);

        _config.scene.addComponent<BoundingBox>(entity, modelRenderable.mesh->boundingBox);

        _config.scene.addComponent<Renderable>(entity, mesh, mat);
        auto armature = loadArmature(modelRenderable.mesh);
        if (armature)
        {
            _config.scene.addComponent<Skinnable>(entity, armature);
        }
    }

    void ModelSceneConfigurer::configureEntity(const ModelCamera& cam, Entity entity) noexcept
    {
        _config.scene.addComponent<Camera>(entity, cam.projection);
    }

    void ModelSceneConfigurer::configureEntity(const ModelPointLight& light, Entity entity) noexcept
    {
        _config.scene.addComponent<PointLight>(entity)
            .setIntensity(light.intensity)
            .setRange(light.range)
            .setColor(light.color);
    }

    void ModelSceneConfigurer::configureEntity(const ModelSpotLight& light, Entity entity) noexcept
    {
        _config.scene.addComponent<SpotLight>(entity)
            .setIntensity(light.intensity)
            .setRange(light.range)
            .setColor(light.color)
            .setConeAngle(light.coneAngle)
            .setInnerConeAngle(light.innerConeAngle);
    }

    void ModelSceneConfigurer::configureEntity(const ModelAmbientLight& light, Entity entity) noexcept
    {
        _config.scene.addComponent<AmbientLight>(entity)
            .setIntensity(light.intensity)
            .setColor(light.color);
    }
}

std::ostream& operator<<(std::ostream& out, const darmok::ModelMaterial& material)
{
    out << material.toString();
    return out;
}

std::ostream& operator<<(std::ostream& out, const darmok::ModelMesh& mesh)
{
    out << mesh.toString();
    return out;
}

std::ostream& operator<<(std::ostream& out, const darmok::ModelRenderable& renderable)
{
    out << renderable.toString();
    return out;
}

std::ostream& operator<<(std::ostream& out, const darmok::ModelNode& node)
{
    out << node.toString();
    return out;
}

std::ostream& operator<<(std::ostream& out, const darmok::Model& model)
{
    out << model.toString();
    return out;
}