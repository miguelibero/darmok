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
        using Materials = std::unordered_set<std::shared_ptr<Material>>;
        using Programs = std::unordered_map<std::shared_ptr<ProgramSource>, std::shared_ptr<ProgramDefinition>>;
        using Meshes = std::unordered_map<std::shared_ptr<MeshSource>, std::shared_ptr<MeshData>>;
        using Scenes = std::unordered_set<std::shared_ptr<Scene>>;

        EditorProject(App& app);

        void init(const ProgramCompilerConfig& progCompilerConfig);
        void shutdown();

        void save(bool forceNewPath = false);
        void open();

        std::shared_ptr<Scene> getScene();
        OptionalRef<Camera> getCamera();

        const Materials& getMaterials() const;
        std::shared_ptr<Material> addMaterial();
        
        const Programs& getPrograms() const;
        bool removeProgram(ProgramSource& src) noexcept;
        std::string getProgramName(const std::shared_ptr<Program>& prog) const;
        bool isProgramCached(const ProgramAsset& asset) const noexcept;
        std::shared_ptr<ProgramSource> addProgram();
        std::shared_ptr<Program> loadProgram(const ProgramAsset& asset);

        const Meshes& getMeshes() const;
        std::string getMeshName(const std::shared_ptr<IMesh>& mesh) const;
        std::shared_ptr<IMesh> loadMesh(const MeshAsset& asset);
        std::shared_ptr<MeshSource> addMesh();

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