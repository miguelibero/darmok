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
#include <darmok/protobuf/asset.pb.h>
#include <darmok/protobuf/mesh.pb.h>
#include <darmok/protobuf/material.pb.h>
#include <darmok/protobuf/texture.pb.h>
#include <darmok/protobuf/light.pb.h>
#include <darmok/protobuf/assimp.pb.h>

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
        using ImportConfig = protobuf::AssimpSceneImportConfig;
        using Definition = protobuf::Scene;
        using MeshDefinition = protobuf::Mesh;
        using ArmatureDefinition = protobuf::Armature;
        using TransformDefinition = protobuf::Transform;
        using CameraDefinition = protobuf::Camera;
        using RenderableDefinition = protobuf::Renderable;
        using SkinnableDefinition = protobuf::Skinnable;
        using MaterialDefinition = protobuf::Material;
        using TextureDefinition = protobuf::Texture;
        using TextureType = protobuf::MaterialTexture::Type;
        using Message = google::protobuf::Message;

        AssimpSceneDefinitionConverter(const aiScene& scene, const std::filesystem::path& basePath, const ImportConfig& config,
            bx::AllocatorI& alloc, OptionalRef<ITextureDefinitionLoader> texLoader = nullptr) noexcept;
        static std::vector<std::string> getTexturePaths(const aiScene& scene) noexcept;
        AssimpSceneDefinitionConverter& setBoneNames(const std::vector<std::string>& names) noexcept;
        AssimpSceneDefinitionConverter& setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept;
        AssimpSceneDefinitionConverter& setConfig(const nlohmann::json& config) noexcept;
        bool operator()(Definition& def) noexcept;
    private:
        bx::AllocatorI& _allocator;
        OptionalRef<ITextureDefinitionLoader> _texLoader;
        const aiScene& _scene;
        std::filesystem::path _basePath;
        ImportConfig _config;
        std::unordered_map<std::string, std::string> _boneNames;
        std::vector<std::regex> _skipMeshes;

        struct AssimpMaterialTexture final
        {
            aiTextureType assimpType;
            unsigned int assimpIndex;
            Material::TextureType darmokType;
        };

        static const std::vector<AssimpMaterialTexture> _materialTextures;

        std::unordered_map<const aiMesh*, std::string> _meshPaths;
        std::unordered_map<const aiMesh*, std::string> _armaturePaths;
        std::unordered_map<const aiMaterial*, std::string> _materialPaths;
        std::unordered_set<std::string> _texturePaths;

        static float getLightRange(const glm::vec3& attenuation) noexcept;

        std::string getMesh(Definition& def, int index) noexcept;
        std::string getArmature(Definition& def, int index) noexcept;
        std::string getMaterial(Definition& def, int index) noexcept;
        std::string getTexture(Definition& def, const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept;
        bool loadTexture(Definition& def, const std::string& path) noexcept;

        std::optional<uint32_t> updateNode(Definition& def, const aiNode& assimpNode, uint32_t parentEntityId = 0) noexcept;
        void updateMaterial(Definition& def, MaterialDefinition& matDef, const aiMaterial& assimpMat) noexcept;
        void updateMesh(Definition& def, MeshDefinition& meshDef, const aiMesh& assimpMesh) noexcept;
        void updateArmature(Definition& def, ArmatureDefinition& armDef, const aiMesh& assimpMesh) noexcept;
        void updateCamera(Definition& def, uint32_t entityId, const aiCamera& assimpCam) noexcept;
        void updateLight(Definition& def, uint32_t entityId, const aiLight& assimpLight) noexcept;
        bool updateMeshes(Definition& def, uint32_t entityId, const std::regex& regex) noexcept;

        std::string createVertexData(const aiMesh& assimpMesh, const std::vector<aiBone*>& bones) const noexcept;
        std::string createIndexData(const aiMesh& assimpMesh) const noexcept;
        bool updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept;

		uint32_t createEntity(Definition& def) noexcept;
		bool addAsset(Definition& def, std::string_view path, Message& asset) noexcept;
        bool addComponent(Definition& def, uint32_t entityId, Message& comp) noexcept;
        bool addMeshComponents(Definition& def, uint32_t entityId, int index) noexcept;
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

    class AssimpSceneFileImporterImpl final
    {
    public:
        using Input = FileTypeImporterInput;
        using Config = protobuf::AssimpSceneImportConfig;
        using OutputFormat = protobuf::Format;
        using Dependencies = FileTypeImportDependencies;
        using Definition = protobuf::Scene;

        AssimpSceneFileImporterImpl(bx::AllocatorI& alloc);
        
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
        FileDataLoader _dataLoader;
        ImageLoader _imgLoader;
        ImageTextureDefinitionLoader _texLoader;
        DataProgramDefinitionLoader _progLoader;
        AssimpLoader _assimpLoader;
        std::optional<Config> _currentConfig;
        OutputFormat _outputFormat = OutputFormat::Binary;
        std::filesystem::path _outputPath;
        std::shared_ptr<aiScene> _currentScene;

        void loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, Config& config);
        static expected<VertexLayout, std::string> loadVertexLayout(const nlohmann::ordered_json& json);
    };
}