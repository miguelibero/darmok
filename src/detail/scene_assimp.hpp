#pragma once

#include <darmok/vertex_fwd.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/program.hpp>
#include <darmok/scene_assimp.hpp>
#include <darmok/image.hpp>
#include <darmok/texture.hpp>
#include <darmok/glm.hpp>
#include <darmok/assimp.hpp>

#include <darmok/protobuf.hpp>
#include <darmok/protobuf/scene.pb.h>
#include <darmok/protobuf/asset.pb.h>
#include <darmok/protobuf/mesh.pb.h>
#include <darmok/protobuf/material.pb.h>
#include <darmok/protobuf/texture.pb.h>
#include <darmok/protobuf/light.pb.h>
#include <darmok/protobuf/assimp.pb.h>
#include <darmok/protobuf/skeleton.pb.h>
#include <darmok/protobuf/varying.pb.h>

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

    class AssimpSceneDefinitionConverter final
    {
    public:
        using ImportConfig = protobuf::AssimpSceneImportConfig;
        using Definition = protobuf::Scene;
        using MeshSource = protobuf::MeshSource;
        using ArmatureDefinition = protobuf::Armature;
        using TransformDefinition = protobuf::Transform;
        using CameraDefinition = protobuf::Camera;
        using RenderableDefinition = protobuf::Renderable;
        using SkinnableDefinition = protobuf::Skinnable;
        using MaterialDefinition = protobuf::Material;
        using TextureSource = protobuf::TextureSource;
        using TextureType = protobuf::MaterialTexture::Type;
        using Message = google::protobuf::Message;

        AssimpSceneDefinitionConverter(const aiScene& assimpScene, Definition& sceneDef, const std::filesystem::path& basePath, const ImportConfig& config,
            bx::AllocatorI& alloc, OptionalRef<ITextureSourceLoader> texLoader = nullptr, OptionalRef<IProgramSourceLoader> progLoader = nullptr) noexcept;
        static std::vector<std::string> getTexturePaths(const aiScene& scene) noexcept;
        AssimpSceneDefinitionConverter& setBoneNames(const std::vector<std::string>& names) noexcept;
        AssimpSceneDefinitionConverter& setBoneNames(const std::unordered_map<std::string, std::string>& names) noexcept;
        AssimpSceneDefinitionConverter& setConfig(const nlohmann::json& config) noexcept;
        expected<void, std::string> operator()() noexcept;
    private:
        bx::AllocatorI& _allocator;
        OptionalRef<ITextureSourceLoader> _texLoader;
        OptionalRef<IProgramSourceLoader> _progLoader;
        const aiScene& _assimpScene;
		SceneDefinitionWrapper _scene;
        std::filesystem::path _basePath;
        ImportConfig _config;
        std::unordered_map<std::string, std::string> _boneNames;
        std::vector<std::regex> _skipMeshes;

        struct AssimpMaterialTexture final
        {
            aiTextureType assimpType;
            unsigned int assimpIndex;
            protobuf::MaterialTexture::Type darmokType;
        };

        static const std::vector<AssimpMaterialTexture> _materialTextures;

        std::unordered_map<const aiMesh*, std::string> _meshPaths;
        std::unordered_map<const aiMesh*, std::string> _armaturePaths;
        std::unordered_map<const aiMaterial*, std::string> _materialPaths;
        std::unordered_set<std::string> _texturePaths;

        static float getLightRange(const glm::vec3& attenuation) noexcept;

        expected<std::string, std::string> getMesh(int index) noexcept;
        std::string getArmature(int index) noexcept;
        std::string getMaterial(int index) noexcept;
        std::string getTexture(const aiMaterial& assimpMat, aiTextureType type, unsigned int index) noexcept;
        bool loadTexture(const std::string& path) noexcept;

        expected<EntityId, std::string> updateNode(const aiNode& assimpNode, EntityId parentEntity = 0) noexcept;
        void updateMaterial(MaterialDefinition& matDef, const aiMaterial& assimpMat) noexcept;
        expected<void, std::string> updateMesh(MeshSource& meshSrc, const aiMesh& assimpMesh) noexcept;
        void updateArmature(ArmatureDefinition& armDef, const aiMesh& assimpMesh) noexcept;
        void updateCamera(EntityId entity, const aiCamera& assimpCam) noexcept;
        void updateLight(EntityId entity, const aiLight& assimpLight) noexcept;
        expected<bool, std::string> updateMeshes(EntityId entity, const std::regex& regex) noexcept;

        bool updateBoneData(const std::vector<aiBone*>& bones, VertexDataWriter& writer) const noexcept;
        expected<void, std::string> addMeshComponents(EntityId entity, int index, bool addChild) noexcept;
    };

    class AssimpSceneDefinitionLoaderImpl final
    {
    public:
        using Config = protobuf::AssimpSceneImportConfig;
        using Model = protobuf::Scene;
        using Result = expected<std::shared_ptr<Model>, std::string>;
        AssimpSceneDefinitionLoaderImpl(IDataLoader& dataLoader, bx::AllocatorI& allocator, OptionalRef<ITextureSourceLoader> texLoader = nullptr) noexcept;
        void setConfig(const Config& config) noexcept;
        bool supports(const std::filesystem::path& path) const noexcept;
        Result operator()(const std::filesystem::path& path);

    private:
        Config _config;
        IDataLoader& _dataLoader;
        bx::AllocatorI& _allocator;
        OptionalRef<ITextureSourceLoader> _texLoader;
        AssimpLoader _assimpLoader;
    };

    class AssimpSceneFileImporterImpl final
    {
    public:
        using Input = FileTypeImporterInput;
        using Config = protobuf::AssimpSceneImportConfig;
        using CompilerConfig = SceneDefinitionCompilerConfig;
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
        void setLogOutput(OptionalRef<std::ostream> log) noexcept;
        void setShadercPath(const std::filesystem::path& path) noexcept;
        void addIncludePath(const std::filesystem::path& path) noexcept;
    private:
        bx::AllocatorI& _alloc;
        bx::FileReader _fileReader;
        FileDataLoader _dataLoader;
        ImageTextureSourceLoader _texLoader;
        ProgramSourceLoader _progLoader;
        AssimpLoader _assimpLoader;
        std::optional<Config> _currentConfig;
        OutputFormat _outputFormat = OutputFormat::Binary;
        std::filesystem::path _outputPath;
        std::shared_ptr<aiScene> _currentScene;
        CompilerConfig _defaultCompilerConfig;
        std::optional<CompilerConfig> _compilerConfig;

        void loadConfig(const nlohmann::ordered_json& json, const std::filesystem::path& basePath, Config& config);
        static expected<protobuf::VertexLayout, std::string> loadVertexLayout(const nlohmann::ordered_json& json);
    };
}