#pragma once

#include <darmok/vertex_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program.hpp>
#include <darmok/scene_assimp.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/glm.hpp>

#include <darmok/protobuf.hpp>
#include <darmok/protobuf/scene.pb.h>
#include <darmok/protobuf/mesh.pb.h>
#include <darmok/protobuf/material.pb.h>
#include <darmok/protobuf/texture.pb.h>
#include <darmok/protobuf/assimp.pb.h>
#include <darmok/protobuf/light.pb.h>

#include <string_view>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>
#include <filesystem>

#include <bgfx/bgfx.h>
#include <assimp/material.h>
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
    class IImageLoader;
    class IDataLoader;
    class IProgramLoader;
    class Image;
    class Data;
    class VertexDataWriter;

    class AssimpLoader final
    {
    public:
        struct Config final
        {
            bool leftHanded = true;
            bool populateArmature = false;
            std::string basePath;
            std::string format;

            void setPath(const std::filesystem::path& path) noexcept;
        };

        using Result = expected<std::shared_ptr<aiScene>, std::string>;
        bool supports(const std::filesystem::path& path) const noexcept;
        Result loadFromFile(const std::filesystem::path& path, const Config& config = {}) const ;
        Result loadFromMemory(const DataView& data, const Config& config = {}) const;
    private:
        static unsigned int getImporterFlags(const Config& config = {}) noexcept;
        static std::shared_ptr<aiScene> fixScene(Assimp::Importer& importer) noexcept;
    };

    class AssimpSceneDefinitionConverter final
    {
    public:
        using Config = protobuf::AssimpSceneImportConfig;
        using Definition = protobuf::Scene;
        using MeshDefinition = protobuf::Mesh;
        using TransformDefinition = protobuf::Transform;
        using CameraDefinition = protobuf::Camera;
        using MaterialDefinition = protobuf::Material;
        using TextureDefinition = protobuf::Texture;
        using TextureType = protobuf::MaterialTextureType;
        using Message = google::protobuf::Message;

        AssimpSceneDefinitionConverter(const aiScene& scene, const std::filesystem::path& basePath, const Config& config,
            bx::AllocatorI& alloc, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        static std::vector<std::string> getTexturePaths(const aiScene& scene) noexcept;
        AssimpSceneDefinitionConverter& setBoneNames(const std::vector<std::string>& names) noexcept;
        AssimpSceneDefinitionConverter& setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept;
        AssimpSceneDefinitionConverter& setConfig(const nlohmann::json& config) noexcept;
        bool update(Definition& def) noexcept;
    private:
        bx::AllocatorI& _allocator;
        OptionalRef<ITextureDefinitionLoader> _texLoader;
        const aiScene& _scene;
        std::filesystem::path _basePath;
        Config _config;
        std::unordered_map<std::string, std::string> _boneNames;
        std::vector<std::regex> _skipMeshes;

        struct AssimpMaterialTexture final
        {
            aiTextureType assimpType;
            unsigned int assimpIndex;
            TextureType::Enum darmokType;
        };

        static const std::vector<AssimpMaterialTexture> _materialTextures;

        std::unordered_map<const aiMesh*, std::shared_ptr<MeshDefinition>> _meshes;
        std::unordered_map<const aiMaterial*, std::shared_ptr<MaterialDefinition>> _materials;
        std::unordered_map<std::string, std::shared_ptr<TextureDefinition>> _textures;

        static float getLightRange(const glm::vec3& attenuation) noexcept;

        std::shared_ptr<MeshDefinition> getMesh(Definition& def, const aiMesh* assimpMesh) noexcept;
        std::shared_ptr<MaterialDefinition> getMaterial(Definition& def, const aiMaterial* assimpMaterial) noexcept;
        std::shared_ptr<TextureDefinition> getTexture(Definition& def, const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept;
        std::shared_ptr<TextureDefinition> getTexture(Definition& def, const std::string& path) noexcept;

        bool updateNode(Definition& def, uint32_t entityId, const aiNode& assimpNode, uint32_t parentEntityId = 0) noexcept;
        void updateMaterial(Definition& def, MaterialDefinition& matDef, const aiMaterial& assimpMat) noexcept;
        void updateMesh(Definition& def, MeshDefinition& meshDef, const aiMesh& assimpMesh) noexcept;
        void updateCamera(Definition& def, uint32_t entityId, const aiCamera& assimpCam) noexcept;
        void updateLight(Definition& def, uint32_t entityId, const aiLight& assimpLight) noexcept;
        bool updateMeshes(Definition& def, uint32_t entityId, const std::regex& regex) noexcept;

        std::string createVertexData(const aiMesh& assimpMesh, const std::vector<aiBone*>& bones) const noexcept;
        std::string createIndexData(const aiMesh& assimpMesh) const noexcept;
        bool updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept;

		uint32_t getNextEntityId(const Definition& def) noexcept;
		bool addAsset(Definition& def, std::string_view path, Message& asset) noexcept;
        bool addComponent(Definition& def, uint32_t entityId, Message& comp) noexcept;
    };

    class AssimpSceneDefinitionLoaderImpl final
    {
    public:
        using Config = protobuf::AssimpSceneImportConfig;
        using Model = protobuf::Scene;
        using Result = expected<std::shared_ptr<Model>, std::string>;
        AssimpSceneDefinitionLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        void setConfig(const Config& config) noexcept;
        bool supports(const std::filesystem::path& path) const noexcept;
        Result operator()(const std::filesystem::path& path);

    private:
        Config _config;
        IDataLoader& _dataLoader;
        bx::AllocatorI& _allocator;
        OptionalRef<ITextureDefinitionLoader> _texLoader;
        AssimpLoader _assimpLoader;
    };

    class AssimpFileImporterImpl final
    {
    public:
        using Input = FileTypeImporterInput;
        using Config = protobuf::AssimpSceneImportConfig;
        using OutputFormat = protobuf::Format;
        using Dependencies = FileTypeImportDependencies;
        using Definition = protobuf::Scene;

        AssimpFileImporterImpl(bx::AllocatorI& alloc);
        
        bool startImport(const Input& input, bool dry = false);
        std::vector<std::filesystem::path> getOutputs(const Input& input);
        Dependencies getDependencies(const Input& input);
        std::ofstream createOutputStream(const Input& input, size_t outputIndex, const std::filesystem::path& path) const;
        void writeOutput(const Input& input, size_t outputIndex, std::ostream& out);
        void endImport(const Input& input);
        const std::string& getName() const noexcept;
    private:
        bx::AllocatorI& _alloc;
        bx::FileReader _fileReader;
        DataLoader _dataLoader;
        ImageLoader _imgLoader;
        ImageTextureDefinitionLoader _texLoader;
        DataProgramDefinitionLoader _progLoader;
        AssimpLoader _assimpLoader;
        std::optional<Config> _currentConfig;
        OutputFormat _outputFormat = OutputFormat::Binary;
        std::filesystem::path _outputPath;
        std::shared_ptr<aiScene> _currentScene;

        static const std::string _outputFormatJsonKey;
        static const std::string _outputPathJsonKey;
        static const std::string _vertexLayoutJsonKey;
        static const std::string _programPathJsonKey;
        static const std::string _programJsonKey;
        static const std::string _programDefinesJsonKey;
        static const std::string _skipMeshesJsonKey;
        static const std::string _skipNodesJsonKey;
        static const std::string _embedTexturesJsonKey;
        static const std::string _defaultTextureJsonKey;
        static const std::string _rootMeshJsonKey;
        static const std::string _opacityJsonKey;
        static const std::string _formatJsonKey;
        static const std::string _loadPathJsonKey;

        void loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, Config& config);
        static expected<VertexLayout, std::string> loadVertexLayout(const nlohmann::ordered_json& json);
    };
}