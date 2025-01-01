#pragma once

#include <darmok-editor/asset_fwd.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/program_core.hpp>
#include <darmok/material.hpp>

#include <filesystem>
#include <unordered_map>
#include <unordered_set>

#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/memory.hpp>

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

        std::shared_ptr<Scene> getScene();
        OptionalRef<Camera> getCamera();

        std::vector<MaterialAsset> getMaterials() const;
        std::shared_ptr<Material> addMaterial();
        
        std::vector<ProgramAsset> getPrograms() const;
        bool removeProgram(ProgramSource& src) noexcept;
        std::string getProgramName(const std::shared_ptr<Program>& prog) const;
        bool isProgramCached(const ProgramAsset& asset) const noexcept;
        std::shared_ptr<ProgramSource> addProgram();
        std::shared_ptr<Program> loadProgram(const ProgramAsset& asset);
        ProgramAsset findProgram(const std::shared_ptr<Program>& prog) const;

        std::vector<MeshAsset> getMeshes() const;
        std::string getMeshName(const std::shared_ptr<IMesh>& mesh) const;
        bool isMeshCached(const MeshAsset& asset, const bgfx::VertexLayout& layout) const noexcept;
        std::shared_ptr<IMesh> loadMesh(const MeshAsset& asset, const bgfx::VertexLayout& layout);
        std::shared_ptr<MeshSource> addMesh();
        bool removeMesh(MeshSource& src) noexcept;
        MeshAsset findMesh(const std::shared_ptr<IMesh>& mesh) const;

        // darmok::ISceneDelegate
        bool shouldCameraRender(const Camera& cam) const noexcept override;
        bool shouldEntityBeSerialized(Entity entity) const noexcept override;

        bool isEditorEntity(Entity entity) const noexcept;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(
                CEREAL_NVP_("scenes", _scenes),
                CEREAL_NVP_("materials", _materials),
                CEREAL_NVP_("programs", _programs),
                CEREAL_NVP_("meshes", _meshes)
            );
        }

    private:
        App& _app;
        OptionalRef<Camera> _cam;
        std::shared_ptr<Scene> _scene;
        std::optional<ProgramCompiler> _progCompiler;

        using Materials = std::unordered_set<std::shared_ptr<Material>>;
        using Programs = std::unordered_map<std::shared_ptr<ProgramSource>, std::shared_ptr<ProgramDefinition>>;
        using Meshes = std::unordered_map<std::shared_ptr<MeshSource>, std::vector<std::shared_ptr<MeshDefinition>>> ;
        using Scenes = std::unordered_set<std::shared_ptr<Scene>>;

        Materials _materials;
        Scenes _scenes;
        Programs _programs;
        Meshes _meshes;

        std::optional<std::filesystem::path> _path;
        static const std::vector<std::string> _dialogFilters;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(Scene& scene);
    };
}