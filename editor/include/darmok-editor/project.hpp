#pragma once

#include <darmok/scene.hpp>
#include <cereal/cereal.hpp>
#include <darmok/scene_serialize.hpp>

#include <filesystem>

namespace darmok::editor
{
    class EditorProject final : darmok::ISceneDelegate
    {
    public:
        EditorProject(App& app);

        void init();
        void shutdown();

        void save(bool forceNewPath = false);
        void open();

        std::shared_ptr<Scene> getScene();
        OptionalRef<Camera> getCamera();
    
        // darmok::ISceneDelegate
        bool shouldCameraRender(const Camera& cam) const noexcept override;
        bool shouldEntityBeSerialized(Entity entity) const noexcept override;

        bool isEditorEntity(Entity entity) const noexcept;

        template<typename Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP_("scene", *_scene));
        }

    private:
        App& _app;
        OptionalRef<Camera> _cam;
        std::shared_ptr<Scene> _scene;
        std::optional<std::filesystem::path> _path;
        static const std::vector<std::string> _dialogFilters;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(Scene& scene);
    };
}