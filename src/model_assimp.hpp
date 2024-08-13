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
struct aiBone;

namespace Assimp
{
    class Importer;
}

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
    class VertexDataWriter;

    struct AssimpSceneLoadConfig final
    {
        bool leftHanded = true;
        bool populateArmature = false;
    };

    class AssimpSceneLoader
    {
    public:
        using Config = AssimpSceneLoadConfig;
        using result_type = std::shared_ptr<aiScene>;
        bool supports(std::string_view name) const noexcept;
        result_type loadFromFile(const std::filesystem::path& path, const Config& config = {}) const ;
        result_type loadFromMemory(const DataView& data, const std::string& name, const Config& config = {}) const;
    private:
        static unsigned int getImporterFlags(const Config& config = {}) noexcept;
        static result_type fixScene(Assimp::Importer& importer) noexcept;
    };

    class AssimpModelConverter final
    {
    public:
        using Config = AssimpModelLoadConfig;
        AssimpModelConverter(const aiScene& scene, const std::string& basePath, const Config& config,
            bx::AllocatorI& alloc, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        static std::vector<std::string> getTexturePaths(const aiScene& scene) noexcept;
        AssimpModelConverter& setBoneNames(const std::vector<std::string>& names) noexcept;
        AssimpModelConverter& setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept;
        AssimpModelConverter& setConfig(const nlohmann::json& config) noexcept;
        void update(Model& model) noexcept;
    private:
        bx::AllocatorI& _allocator;
        OptionalRef<IImageLoader> _imgLoader;
        const aiScene& _scene;
        std::string _basePath;
        Config _config;
        glm::mat4 _inverseRoot;
        std::unordered_map<std::string, std::string> _boneNames;

        static const std::unordered_map<aiTextureType, MaterialTextureType> _materialTextures;

        struct AssimpMaterialColor final
        {
            const char* name;
            unsigned int type;
            unsigned int idx;
            MaterialColorType darmokType;
        };

        static const std::vector<AssimpMaterialColor> _materialColors;

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

        Data createVertexData(const aiMesh& assimpMesh, const std::vector<aiBone*>& bones) const noexcept;
        std::vector<VertexIndex> createIndexData(const aiMesh& assimpMesh) const noexcept;
        bool updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept;
    };

    class AssimpModelLoaderImpl final
    {
    public:
        using Config = AssimpModelLoadConfig;
        AssimpModelLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<IImageLoader> imgLoader = nullptr) noexcept;
        void setConfig(const Config& config) noexcept;
        bool supports(std::string_view name) const noexcept;
        std::shared_ptr<Model> operator()(std::string_view name);

    private:
        Config _config;
        IDataLoader& _dataLoader;
        bx::AllocatorI& _allocator;
        OptionalRef<IImageLoader> _imgLoader;
        AssimpSceneLoader _sceneLoader;
    };

    struct AssimpModelImportConfig
    {
        using OutputFormat = ModelDataFormat;
        using LoadConfig = AssimpModelLoadConfig;
        ModelDataFormat outputFormat = OutputFormat::Binary;
        std::filesystem::path outputPath;
        LoadConfig loadConfig;
    };

    class AssimpModelImporterImpl final
    {
    public:
        using Input = AssetTypeImporterInput;
        using Config = AssimpModelImportConfig;
        using LoadConfig = AssimpModelLoadConfig;
        using OutputFormat = ModelDataFormat;
        using Dependencies = AssetImportDependencies;

        AssimpModelImporterImpl();
        
        bool startImport(const Input& input, bool dry = false);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        Dependencies getDependencies(const Input& input);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) const;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        void endImport(const Input& input);
        const std::string& getName() const noexcept;
    private:
        bx::DefaultAllocator _allocator;
        bx::FileReader _fileReader;
        FileDataLoader _dataLoader;
        DataImageLoader _imgLoader;
        AssimpSceneLoader _assimpLoader;
        std::optional<Config> _currentConfig;
        std::shared_ptr<aiScene> _currentScene;

        static const std::string _outputFormatJsonKey;
        static const std::string _outputPathJsonKey;
        static const std::string _vertexLayoutJsonKey;
        static const std::string _programPathJsonKey;
        static const std::string _programJsonKey;
        static const std::string _skipMeshesJsonKey;
        static const std::string _embedTexturesJsonKey;

        void loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, Config& config) const;
        void loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, LoadConfig& config) const;
        static VertexLayout loadVertexLayout(const nlohmann::ordered_json& json);
    };
}