#pragma once

#include <darmok-editor/app.hpp>
#include <darmok-editor/scene_view.hpp>

namespace darmok::editor
{
    class Physics3dEditorAppComponent final : public IEditorAppComponent
    {
        expected<void, std::string> renderMainMenu(MainMenuSection section) noexcept override;
        expected<void, std::string> init(EditorApp& app) noexcept override;
        expected<void, std::string> shutdown() noexcept override;

        OptionalRef<EditorApp> _app;
    };

    class Physics3dShapeGizmo final : public ISceneGizmo
    {
    public:
        expected<void, std::string> init(Camera& cam, Scene& scene, SceneGizmosRenderer& renderer) noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> render(bgfx::ViewId viewId, bgfx::Encoder& encoder) noexcept override;
    private:
        OptionalRef<SceneGizmosRenderer> _renderer;
        OptionalRef<Scene> _scene;
    };
}