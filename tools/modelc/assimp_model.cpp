#include "assimp_model.hpp"
#include "assimp.hpp"
#include <darmok/model.hpp>
#include <darmok/image.hpp>
#include <filesystem>

namespace darmok
{
    AssimpModelUpdater::AssimpModelUpdater(const AssimpScene& assimpScene, bx::AllocatorI& allocator, const bgfx::VertexLayout& vertexLayout) noexcept
        : _assimpScene(assimpScene)
        , _allocator(allocator)
        , _vertexLayout(vertexLayout)
    {
    }

    void AssimpModelUpdater::setImageLoader(IImageLoader& imgLoader) noexcept
    {
        _imgLoader = imgLoader;
    }

    void AssimpModelUpdater::setPath(const std::string& path) noexcept
    {
        _basePath = std::filesystem::path(path).parent_path().string();
    }

    void AssimpModelUpdater::run(Model& model) noexcept
    {
        update(model.rootNode, *_assimpScene.getRootNode());
    }

    void AssimpModelUpdater::update(ModelTexture& modelTex, const Image& img) noexcept
    {
        modelTex.data = img.getData();
        modelTex.config = img.getTextureConfig();
    }

    void AssimpModelUpdater::update(ModelTexture& modelTex, const AssimpMaterialTexture& assimpTex) noexcept
    {
        modelTex.name = assimpTex.getPath();
        std::shared_ptr<Image> img;
        auto embedded = _assimpScene.getEmbeddedTexture(assimpTex.getPath());
        if (embedded)
        {
            img = embedded->loadImage(_allocator);
        }
        if (!img && _imgLoader)
        {
            std::filesystem::path fsPath(assimpTex.getPath());
            if (fsPath.is_relative())
            {
                fsPath = std::filesystem::path(_basePath) / fsPath;
            }
            img = (*_imgLoader)(fsPath.string());
        }
        if (img)
        {
            update(modelTex, *img);
        }
    }

    const std::unordered_map<AssimpMaterialColorType, MaterialColorType> AssimpModelUpdater::_materialColors =
    {
        { AssimpMaterialColorType::Diffuse, MaterialColorType::Diffuse },
        { AssimpMaterialColorType::Specular, MaterialColorType::Specular },
        { AssimpMaterialColorType::Ambient, MaterialColorType::Ambient },
        { AssimpMaterialColorType::Emissive, MaterialColorType::Emissive },
        { AssimpMaterialColorType::Transparent, MaterialColorType::Transparent },
        { AssimpMaterialColorType::Reflective, MaterialColorType::Reflective },
    };

    const std::unordered_map<aiTextureType, MaterialTextureType> AssimpModelUpdater::_materialTextures =
    {
        { aiTextureType_DIFFUSE, MaterialTextureType::Diffuse },
        { aiTextureType_SPECULAR, MaterialTextureType::Specular },
        { aiTextureType_NORMALS, MaterialTextureType::Normal },
    };

    void AssimpModelUpdater::update(ModelMaterial& modelMat, const AssimpMaterial& assimpMat) noexcept
    {
        std::filesystem::path basePath(_basePath);
        for (auto& elm : _materialTextures)
        {
            for (auto& assimpTex : assimpMat.getTextures(elm.first))
            {
                getTexture(assimpTex);
                /*
                ModelTexture modelTex;
                assimpTex.updateModel(modelTex, config);
                modelMat.textures.emplace(elm.second, std::move(modelTex));
                */
            }
        }
        for (auto& elm : _materialColors)
        {
            if (auto v = assimpMat.getColor(elm.first))
            {
                modelMat.colors.emplace(elm.second, v.value());
            }
        }
    }

    void AssimpModelUpdater::update(ModelMesh& modelMesh, const AssimpMesh& assimpMesh) noexcept
    {
        modelMesh.vertexData = assimpMesh.createVertexData(_vertexLayout, _allocator);
        modelMesh.indexData = assimpMesh.createIndexData();

        for (auto& bone : assimpMesh.getBones())
        {
            modelMesh.joints.push_back(ModelArmatureJoint{
                std::string(bone.getName()),
                bone.getInverseBindPoseMatrix()
            });
        }
    }

    void AssimpModelUpdater::update(ModelNode& modelNode, const AssimpNode& assimpNode) noexcept
    {
        modelNode.name = assimpNode.getName();
        modelNode.transform = assimpNode.getTransform();
        for (auto& assimpMesh : assimpNode.getMeshes())
        {
            auto& modelRenderable = modelNode.renderables.emplace_back();
            modelRenderable.mesh = getMesh(assimpMesh);
            modelRenderable.material = getMaterial(assimpMesh->getMaterial());
        }
        for (auto& assimpChild : assimpNode.getChildren())
        {
            auto& modelChild = modelNode.children.emplace_back();
            update(modelChild, *assimpChild);
        }
    }

    std::shared_ptr<ModelMesh> AssimpModelUpdater::getMesh(const std::shared_ptr<AssimpMesh> assimpMesh) noexcept
    {
        auto itr = _meshes.find(assimpMesh);
        if (itr != _meshes.end())
        {
            return itr->second;
        }
        auto modelMesh = std::make_shared<ModelMesh>();
        update(*modelMesh, *assimpMesh);
        _meshes.emplace(assimpMesh, modelMesh);
        return modelMesh;
    }

    std::shared_ptr<ModelMaterial> AssimpModelUpdater::getMaterial(const std::shared_ptr<AssimpMaterial> assimpMat) noexcept
    {
        auto itr = _materials.find(assimpMat);
        if (itr != _materials.end())
        {
            return itr->second;
        }
        auto modelMat = std::make_shared<ModelMaterial>();
        update(*modelMat, *assimpMat);
        _materials.emplace(assimpMat, modelMat);
        return modelMat;
    }

    std::shared_ptr<ModelTexture> AssimpModelUpdater::getTexture(const AssimpMaterialTexture& assimpTex) noexcept
    {
        auto itr = _textures.find(assimpTex.getPath());
        if (itr != _textures.end())
        {
            return itr->second;
        }
        auto modelTex = std::make_shared<ModelTexture>();
        update(*modelTex, assimpTex);
        _textures.emplace(assimpTex.getPath(), modelTex);
        return modelTex;
    }

}