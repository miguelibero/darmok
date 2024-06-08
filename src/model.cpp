#include "model.hpp"
#include <darmok/model.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include <darmok/light.hpp>
#include <darmok/render.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/program_standard.hpp>
#include <cereal/archives/json.hpp>

namespace darmok
{
    std::string ModelNode::to_string() const noexcept
    {
        std::stringstream ss;
        cereal::JSONOutputArchive archive(ss);
        archive(*this);
        return ss.str();
    }

    std::string Model::to_string() const noexcept
    {
        std::stringstream ss;
        cereal::JSONOutputArchive archive(ss);
        archive(*this);
        return ss.str();
    }

    ModelSceneConfigurer::ModelSceneConfigurer(Scene& scene, AssetContext& assets)
        : _config{ scene, assets }
        , _parent(entt::null)
    {
    }

    ModelSceneConfigurer& ModelSceneConfigurer::setParent(Entity parent) noexcept
    {
        _parent = parent;
        return *this;
    }

    Entity ModelSceneConfigurer::run(const Model& model) noexcept
    {
        return run(model.rootNode);
    }

    Entity ModelSceneConfigurer::run(const ModelNode& node) noexcept
    {
        return run(node, _parent);
    }

    Entity ModelSceneConfigurer::run(const ModelNode& node, Entity parent) noexcept
    {
        auto entity = add(node, parent);
        for (auto child : node.children)
        {
            run(child, entity);
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
		_config.scene.addComponent<Transform>(entity, node.transform, parentTrans);
        configureEntity(node, entity);
        return entity;
    }

    void ModelSceneConfigurer::configureEntity(const ModelNode& node, Entity entity) noexcept
    {
        if (node.camera)
        {
            configureEntity(*node.camera, entity);
        }
        if (node.pointLight)
        {
            configureEntity(*node.pointLight, entity);
        }
        if (node.ambientLight)
        {
            configureEntity(*node.ambientLight, entity);
        }
        for (auto& renderable : node.renderables)
        {
            configureEntity(renderable, entity);
        }
    }

    std::shared_ptr<Texture> ModelSceneConfigurer::loadTexture(const std::shared_ptr<ModelTexture>& modelTex) noexcept
    {
        if (!modelTex)
        {
            return nullptr;
        }
        auto itr = _textures.find(modelTex);
        if (itr != _textures.end())
        {
            return itr->second;
        }
        std::shared_ptr<Texture> tex;
        if (modelTex->data.empty())
        {
            tex = _config.assets.getTextureLoader()(modelTex->name);
        }
        else
        {
            tex = std::make_shared<Texture>(modelTex->data.view(), modelTex->config);
        }
        _textures.emplace(modelTex, tex);
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
        if (!modelMat->programName.empty())
        {
            prog = _config.assets.getProgramLoader()(modelMat->programName);
        }
        if (!prog)
        {
            prog = _config.assets.getStandardProgramLoader()(modelMat->standardProgram);
        }
        auto mat = std::make_shared<Material>(prog);
        for (auto& elm : modelMat->textures)
        {
            mat->setTexture(elm.first, loadTexture(elm.second));
        }
        for (auto& elm : modelMat->colors)
        {
            mat->setColor(elm.first, elm.second);
        }
        _materials.emplace(modelMat, mat);
        return mat;
    }

    std::shared_ptr<Mesh> ModelSceneConfigurer::loadMesh(const std::shared_ptr<ModelMesh>& modelMesh) noexcept
    {
        if (!modelMesh)
        {
            return nullptr;
        }
        auto itr = _meshes.find(modelMesh);
        if (itr != _meshes.end())
        {
            return itr->second;
        }
        auto mesh = std::make_shared<Mesh>(modelMesh->vertexLayout, modelMesh->vertexData.view(), modelMesh->indexData.view(), modelMesh->config);
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
            .setAttenuation(light.attenuation)
            .setDiffuseColor(light.diffuseColor)
            .setSpecularColor(light.specularColor);
    }

    void ModelSceneConfigurer::configureEntity(const ModelAmbientLight& light, Entity entity) noexcept
    {
        _config.scene.addComponent<AmbientLight>(entity)
            .setIntensity(light.intensity)
            .setColor(light.color);
    }

    BinaryModelLoader::BinaryModelLoader(IDataLoader& dataLoader) noexcept
        : _dataLoader(dataLoader)
    {
    }

    std::shared_ptr<Model> BinaryModelLoader::operator()(std::string_view name)
    {
        auto data = _dataLoader(name);
        auto model = std::shared_ptr<Model>();
        // TODO: how to pass bx::AllocatorI to the serialization process?
        DataInputStream::read(data.view(), *model);
        return model;
    }
}

std::ostream& operator<<(std::ostream& out, const darmok::ModelNode& node)
{
    out << node.to_string();
    return out;
}

std::ostream& operator<<(std::ostream& out, const darmok::Model& model)
{
    out << model.to_string();
    return out;
}