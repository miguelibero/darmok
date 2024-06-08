#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <darmok/optional_ref.hpp>
#include <darmok/material_fwd.hpp>
#include <bgfx/bgfx.h>
#include <assimp/material.h>
#include "assimp_fwd.hpp"


namespace bx
{
    struct AllocatorI;
}

namespace darmok
{
    class Image;
    class IImageLoader;

    class AssimpScene;
    class AssimpNode;
    class AssimpMesh;
    class AssimpMaterial;
    class AssimpMaterialTexture;

    struct Model;
    struct ModelNode;
    struct ModelMesh;
    struct ModelMaterial;
    struct ModelTexture;

    class AssimpModelUpdater final
    {
    public:
        AssimpModelUpdater(const AssimpScene& assimpScene, bx::AllocatorI& allocator, const bgfx::VertexLayout& vertexLayout) noexcept;
        void setImageLoader(IImageLoader& imgLoader) noexcept;
        void setPath(const std::string& path) noexcept;
        void run(Model& model) noexcept;
    private:
        bgfx::VertexLayout _vertexLayout;
        bx::AllocatorI& _allocator;
        OptionalRef<IImageLoader> _imgLoader;
        const AssimpScene& _assimpScene;
        std::string _basePath;

        static const std::unordered_map<AssimpMaterialColorType, MaterialColorType> _materialColors;
        static const std::unordered_map<aiTextureType, MaterialTextureType> _materialTextures;

        std::unordered_map<std::shared_ptr<AssimpMesh>, std::shared_ptr<ModelMesh>> _meshes;
        std::unordered_map<std::shared_ptr<AssimpMaterial>, std::shared_ptr<ModelMaterial>> _materials;
        std::unordered_map<std::string, std::shared_ptr<ModelTexture>> _textures;

        std::shared_ptr<ModelMesh> getMesh(const std::shared_ptr<AssimpMesh> assimpMesh) noexcept;
        std::shared_ptr<ModelMaterial> getMaterial(const std::shared_ptr<AssimpMaterial> assimpMaterial) noexcept;
        std::shared_ptr<ModelTexture> getTexture(const AssimpMaterialTexture& assimpTex) noexcept;
    
        void update(ModelTexture& modelTex, const Image& image) noexcept;
        void update(ModelTexture& modelTex, const AssimpMaterialTexture& assimpTex) noexcept;
        void update(ModelMaterial& modelMat, const AssimpMaterial& assimpMat) noexcept;
        void update(ModelMesh& modelMesh, const AssimpMesh& assimpMesh) noexcept;
        void update(ModelNode& modelNode, const AssimpNode& assimpNode) noexcept;
    };
}