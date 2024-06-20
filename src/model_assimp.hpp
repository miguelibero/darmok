#pragma once

#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <bgfx/bgfx.h>
#include <assimp/material.h>
#include <darmok/material_fwd.hpp>
#include <darmok/vertex_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program.hpp>
#include <darmok/model_assimp.hpp>
#include <darmok/vertex_layout.hpp>
#include <darmok/glm.hpp>
#include <nlohmann/json.hpp>
#include <bx/allocator.h>
#include <bx/file.h>


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
    class IProgramLoader;
    class Image;
    class Data;

    class AssimpSceneLoader
    {
    public:
        using result_type = std::shared_ptr<aiScene>;
        AssimpSceneLoader(IDataLoader& dataLoader) noexcept;
        result_type operator()(std::string_view name);
    private:
        IDataLoader& _dataLoader;
    };

    class AssimpModelConverter final
    {
    public:
        using Config = AssimpModelLoadConfig;
        AssimpModelConverter(const aiScene& scene, const std::string& basePath, const Config& config,
            bx::AllocatorI& alloc, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        void update(Model& model) noexcept;
    private:
        bx::AllocatorI& _allocator;
        OptionalRef<IImageLoader> _imgLoader;
        const aiScene& _scene;
        std::string _basePath;
        Config _config;
        glm::mat4 _inverseRoot;

        static const std::unordered_map<aiTextureType, MaterialTextureType> _materialTextures;

        std::unordered_map<const aiMesh*, std::shared_ptr<ModelMesh>> _meshes;
        std::unordered_map<const aiMaterial*, std::shared_ptr<ModelMaterial>> _materials;
        std::unordered_map<std::string, std::shared_ptr<ModelImage>> _images;

        std::shared_ptr<ModelMesh> getMesh(const aiMesh* assimpMesh) noexcept;
        std::shared_ptr<ModelMaterial> getMaterial(const aiMaterial* assimpMaterial) noexcept;
        std::shared_ptr<ModelImage> getImage(const std::string& path) noexcept;

        void update(ModelNode& modelNode, const aiNode& assimpNode) noexcept;
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
        using Config = AssimpModelLoadConfig;
        AssimpModelLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        void setConfig(const Config& config) noexcept;
        std::shared_ptr<Model> operator()(std::string_view path);

    private:
        Config _config;
        IDataLoader& _dataLoader;
        bx::AllocatorI& _allocator;
        OptionalRef<IImageLoader> _imgLoader;
        AssimpSceneLoader _sceneLoader;
    };

    enum class AssimpModelImporterOutputFormat
    {
        Binary,
        Json,
        Xml
    };

    struct AssimpModelImportConfig
    {
        using OutputFormat = AssimpModelImporterOutputFormat;
        using LoadConfig = AssimpModelLoadConfig;
        OutputFormat outputFormat = OutputFormat::Binary;
        std::string outputFile;
        LoadConfig loadConfig;
    };

    class AssimpModelImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        using Config = AssimpModelImportConfig;
        using LoadConfig = AssimpModelLoadConfig;
        using OutputFormat = AssimpModelImporterOutputFormat;

        AssimpModelImporterImpl();
        std::shared_ptr<Model> read(const std::filesystem::path& path, const LoadConfig& config);

        bool getOutputs(const Input& input, std::vector<std::filesystem::path>& outputs);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path);
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        std::string getName() const noexcept;
    private:
        bx::DefaultAllocator _allocator;
        bx::FileReader _fileReader;
        FileDataLoader _dataLoader;
        DataImageLoader _imgLoader;
        AssimpModelLoader _assimpLoader;
        DataVertexLayoutLoader _layoutLoader;
        DataProgramLoader _progLoader;

        static std::filesystem::path getOutputFile(const std::filesystem::path& path, OutputFormat format) noexcept;

        static const std::string _outputFormatJsonKey;
        static const std::string _outputFileJsonKey;
        static const std::string _vertexLayoutJsonKey;
        static const std::string _programJsonKey;
        static const std::string _embedTexturesJsonKey;

        void loadConfig(const nlohmann::ordered_json& json, Config& config);
        void loadConfig(const nlohmann::ordered_json& json, LoadConfig& config);
        static bgfx::VertexLayout loadVertexLayout(const nlohmann::ordered_json& json);
    };
}