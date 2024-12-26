#pragma once

#include <darmok-editor/asset_fwd.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/program_core.hpp>
#include <darmok/material.hpp>

#include <filesystem>
#include <vector>

#include <cereal/cereal.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/memory.hpp>

namespace darmok::editor
{
    class EditorProject final : darmok::ISceneDelegate
    {
    public:
        using Materials = std::vector<std::shared_ptr<Material>>;
        using Programs = std::vector<std::shared_ptr<ProgramSource>>;
        using Scenes = std::vector<std::shared_ptr<Scene>>;

        EditorProject(App& app);

        void init(const ProgramCompilerConfig& progCompilerConfig);
        void shutdown();

        void save(bool forceNewPath = false);
        void open();

        std::shared_ptr<Scene> getScene();
        OptionalRef<Camera> getCamera();

        Materials& getMaterials();
        const Materials& getMaterials() const;

        Programs& getPrograms();
        const Programs& getPrograms() const;

        std::string getProgramName(const std::shared_ptr<Program>& prog) const;
        std::shared_ptr<Program> loadProgram(const ProgramAsset& asset);
    
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
                CEREAL_NVP_("programs", _programs)
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

        std::optional<std::filesystem::path> _path;
        static const std::vector<std::string> _dialogFilters;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(Scene& scene);
    };
}