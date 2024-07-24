#include <darmok/model.hpp>
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/scene.hpp>
#include <darmok/light.hpp>
#include <darmok/render.hpp>
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/program.hpp>
#include <darmok/skeleton.hpp>

// to allow serialization
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/math.hpp>

namespace darmok
{
    std::string ModelNode::to_string() const noexcept
    {
        std::stringstream ss;
        ss << "ModelNode:" << std::endl;
        cereal::JSONOutputArchive archive(ss);
        archive(*this);
        return ss.str();
    }

    std::string Model::to_string() const noexcept
    {
        std::stringstream ss;
        ss << "Model:" << std::endl;
        cereal::JSONOutputArchive archive(ss);
        archive(*this);
        return ss.str();
    }

    Model::DataFormat Model::getFormat(const std::string& name) noexcept
    {
        if (name == "xml")
        {
            return DataFormat::Xml;
        }
        if (name == "json")
        {
            return DataFormat::Json;
        }
        return DataFormat::Binary;
    }

    Model::DataFormat Model::getExtensionFormat(const std::string& ext) noexcept
    {
        if (ext == ".xml")
        {
            return DataFormat::Xml;
        }
        if (ext == ".json" || ext == ".js")
        {
            return DataFormat::Json;
        }
        return DataFormat::Binary;
    }

    std::string Model::getFormatExtension(DataFormat format) noexcept
    {
        switch (format)
        {
            case DataFormat::Binary:
            {
                return ".bin";
            }
            case DataFormat::Json:
            {
                return ".json";
            }
            case DataFormat::Xml:
            {
                return ".xml";
            }
        }
        return "";
    }

    std::string Model::getFormatName(DataFormat format) noexcept
    {
        switch (format)
        {
            case DataFormat::Binary:
            {
                return "binary";
            }
            case DataFormat::Json:
            {
                return "json";
            }
            case DataFormat::Xml:
            {
                return "xml";
            }
        }
        return "";
    }

    void Model::read(std::istream& in, DataFormat format)
    {
        switch (format)
        {
            case DataFormat::Binary:
            {
                cereal::BinaryInputArchive archive(in);
                archive(*this);
                break;
            }
            case DataFormat::Json:
            {
                cereal::JSONInputArchive archive(in);
                archive(*this);
                break;
            }
            case DataFormat::Xml:
            {
                cereal::XMLInputArchive archive(in);
                archive(*this);
                break;
            }
        }
    }

    void Model::write(std::ostream& out, DataFormat format) const
    {
        switch (format)
        {
            case DataFormat::Binary:
            {
                cereal::BinaryOutputArchive archive(out);
                archive(*this);
                break;
            }
            case DataFormat::Json:
            {
                cereal::JSONOutputArchive archive(out);
                archive(*this);
                break;
            }
            case DataFormat::Xml:
            {
                cereal::XMLOutputArchive archive(out);
                archive(*this);
                break;
            }
        }
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
        for (auto& child : node.children)
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
            tex = _config.assets.getTextureLoader()(modelImg->name, _textureFlags);
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
            prog = std::make_shared<Program>(modelMat->standardProgram);
        }
        auto mat = std::make_shared<Material>(prog);
        for (auto& elm : modelMat->textures)
        {
            // TODO: support for multiple textures of the same type
            if (!elm.second.empty())
            {
                mat->setTexture(elm.first, loadTexture(elm.second.at(0).image));
            }
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

    BinaryModelLoader::result_type BinaryModelLoader::operator()(std::string_view name)
    {
        auto data = _dataLoader(name);
        auto model = std::make_shared<Model>();
        // TODO: how to pass bx::AllocatorI to the serialization process?
        DataInputStream::read(data, *model);
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