#pragma once

#include <darmok-editor/asset_fwd.hpp>
#include <darmok/scene.hpp>
#include <darmok/program_core.hpp>
#include <darmok/material.hpp>

#include <filesystem>
#include <unordered_map>
#include <unordered_set>

namespace darmok::editor
{
    class EditorProject final : darmok::ISceneDelegate
    {
    public:
        EditorProject(App& app);

        void init(const ProgramCompilerConfig& progCompilerConfig);
        void shutdown();

        void save(bool forceNewPath = false);
        void open();
        void reset();
        void exportAssetPack(bool forceNewPath = false);

        void render();

        std::shared_ptr<Scene> getScene();
        OptionalRef<Camera> getCamera();

        std::vector<SceneAsset> getScenes() const;
        std::shared_ptr<Scene> addScene();
        std::string getSceneName(const std::shared_ptr<Scene>& scene) const;
        bool removeScene(Scene& scene) noexcept;

        std::vector<TextureAsset> getTextures() const;
        std::shared_ptr<Texture::Definition> addTexture();
        std::string getTextureName(const std::shared_ptr<Texture::Definition>& tex) const;
        TextureAsset findTexture(const std::shared_ptr<Texture>& tex) const;
        TextureAsset findTexture(const Texture::Definition& def) const;
        std::shared_ptr<Texture> loadTexture(const TextureAsset& asset);
        bool removeTexture(Texture::Definition& def) noexcept;
        bool reloadTexture(Texture::Definition& def);

        std::vector<MaterialAsset> getMaterials() const;
        std::shared_ptr<Material> addMaterial();
        std::string getMaterialName(const std::shared_ptr<Material>& mat) const;
        bool removeMaterial(Material& mat) noexcept;

        std::vector<ProgramAsset> getPrograms() const;
        std::string getProgramName(const std::shared_ptr<Program>& prog) const;
        std::string getProgramName(const ProgramAsset& asset) const;
        bool isProgramCached(const ProgramAsset& asset) const noexcept;
        std::shared_ptr<ProgramSource> addProgram();
        std::shared_ptr<Program> loadProgram(const ProgramAsset& asset);
        ProgramAsset findProgram(const std::shared_ptr<Program>& prog) const;
        bool removeProgram(ProgramSource& src) noexcept;
        bool reloadProgram(ProgramSource& src);

        std::vector<MeshAsset> getMeshes() const;
        std::string getMeshName(const std::shared_ptr<IMesh>& mesh) const;
        std::string getMeshName(const MeshAsset& asset) const;
        bool isMeshCached(const MeshAsset& asset, const bgfx::VertexLayout& layout) const noexcept;
        std::shared_ptr<IMesh> loadMesh(const MeshAsset& asset, const bgfx::VertexLayout& layout);
        std::shared_ptr<MeshSource> addMesh();
        MeshAsset findMesh(const std::shared_ptr<IMesh>& mesh) const;
        bool removeMesh(MeshSource& src) noexcept;
        bool reloadMesh(MeshSource& src);

        std::vector<ModelAsset> getModels() const;
        std::string getModelName(const ModelAsset& asset) const;
        std::shared_ptr<Model> addModel();
        bool removeModel(Model& model) noexcept;
        bool reloadModel(Model& model);

        // darmok::ISceneDelegate
        bool shouldCameraRender(const Camera& cam) const noexcept override;
        bool shouldEntityBeSerialized(Entity entity) const noexcept override;

        bool isEditorEntity(Entity entity) const noexcept;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("scenes", _scenes),
                CEREAL_NVP_("textures", _textures),
                CEREAL_NVP_("materials", _materials),
                CEREAL_NVP_("programs", _programs),
                CEREAL_NVP_("meshes", _meshes),
                CEREAL_NVP_("models", _models)
            );
        }

    private:
        App& _app;
        OptionalRef<Camera> _cam;
        std::shared_ptr<Scene> _scene;
        std::optional<ProgramCompiler> _progCompiler;
        bool _tryReset;

        using Programs = std::unordered_map<std::shared_ptr<ProgramSource>, std::shared_ptr<ProgramDefinition>>;
        using Textures = std::unordered_set<std::shared_ptr<TextureDefinition>>;
        using Materials = std::unordered_set<std::shared_ptr<Material>>;
        using Meshes = std::unordered_map<std::shared_ptr<MeshSource>, std::vector<std::shared_ptr<MeshDefinition>>> ;
        using Models = std::unordered_set<std::shared_ptr<Model>>;
        using Scenes = std::unordered_set<std::shared_ptr<Scene>>;

        Textures _textures;
        Materials _materials;
        Scenes _scenes;
        Programs _programs;
        Meshes _meshes;
        Models _models;

        std::filesystem::path _path;
        std::filesystem::path _exportPath;
        static const std::vector<std::string> _dialogFilters;
        static const char* _confirmNewPopup;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(Scene& scene);

        Programs::const_iterator findProgramSource(const ProgramSource& src) const;
        Programs::iterator findProgramSource(const ProgramSource& src);
        Meshes::const_iterator findMeshSource(const MeshSource& src) const;
        Meshes::iterator findMeshSource(const MeshSource& src);
        Textures::const_iterator findTextureDefinition(const TextureDefinition& def) const;
        Models::iterator findModel(const Model& model);
        Scenes::iterator findScene(const Scene& scene);

        std::vector<std::reference_wrapper<Renderable>> getRenderables();

        void doReset();
    };
}