#pragma once

#include <string_view>
#include <memory>
#include <vector>
#include <bgfx/bgfx.h>
#include <assimp/material.h>
#include <darmok/material_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program_fwd.hpp>
#include <darmok/glm.hpp>

namespace bx
{
    struct AllocatorI;
}

struct aiNode;
struct aiMaterial;
struct aiMesh;
struct aiScene;
struct aiCamera;
struct aiLight;

namespace darmok
{
    struct Model;
    struct ModelNode;
    struct ModelMaterial;
    struct ModelMesh;
    struct ModelTexture;
    struct ModelImage;
    class IImageLoader;
    class IDataLoader;
    class Image;
    class Data;

    struct AssimpModelLoaderConfig final
    {
        IDataLoader& dataLoader;
        bx::AllocatorI& allocator;
        bgfx::VertexLayout vertexLayout;
        StandardProgramType standardProgram = StandardProgramType::ForwardPhong;
        OptionalRef<IImageLoader> imgLoader;
    };

    class AssimpModelLoaderContext final
    {
    public:
        AssimpModelLoaderContext(const aiScene& scene, const std::string& basePath, const AssimpModelLoaderConfig& config) noexcept;
        void update(ModelNode& modelNode, const aiNode& assimpNode) noexcept;

    private:

        const aiScene& _scene;
        std::string _basePath;
        const AssimpModelLoaderConfig& _config;
        glm::mat4 _inverseRoot;

        static const std::unordered_map<aiTextureType, MaterialTextureType> _materialTextures;

        std::unordered_map<const aiMesh*, std::shared_ptr<ModelMesh>> _meshes;
        std::unordered_map<const aiMaterial*, std::shared_ptr<ModelMaterial>> _materials;
        std::unordered_map<std::string, std::shared_ptr<ModelImage>> _images;

        std::shared_ptr<ModelMesh> getMesh(const aiMesh* assimpMesh) noexcept;
        std::shared_ptr<ModelMaterial> getMaterial(const aiMaterial* assimpMaterial) noexcept;
        std::shared_ptr<ModelImage> getImage(const std::string& path) noexcept;

        void update(ModelTexture& modelTex, const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept;
        void update(ModelMaterial& modelMat, const aiMaterial& assimpMat) noexcept;
        void update(ModelMesh& modelMesh, const aiMesh& assimpMesh) noexcept;
        void update(ModelNode& modelNode, const aiCamera& assimpCam) noexcept;
        void update(ModelNode& modelNode, const aiLight& assimpLight) noexcept;

        Data createVertexData(const aiMesh& assimpMesh) const noexcept;
        std::vector<VertexIndex> createIndexData(const aiMesh& assimpMesh) const noexcept;
    };

    class AssimpModelLoaderImpl final
    {
    public:
        AssimpModelLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        void setVertexLayout(const bgfx::VertexLayout& vertexLayout) noexcept;
        std::shared_ptr<Model> operator()(std::string_view path);
    private:
        AssimpModelLoaderConfig _config;
    };
}