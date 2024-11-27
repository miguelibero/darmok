#pragma once

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>

#include <memory>
#include <optional>

#include <imgui.h>

namespace darmok
{
    class Scene;
    class ImguiAppComponent;
    class FrameBuffer;
    class Camera;
    class Transform;
}

namespace darmok::editor
{
    class EditorAppDelegate final : public darmok::IAppDelegate, public darmok::IImguiRenderer
    {
    public:
		EditorAppDelegate(App& app);
		void init() override;
		void shutdown() override;
        void imguiSetup() override;
		void imguiRender() override;
		void update(float deltaTime) override;
    private:
        App& _app;
        std::shared_ptr<Scene> _scene;
        OptionalRef<ImguiAppComponent> _imgui;

        ImGuiID _dockDownId;
        ImGuiID _dockRightId;
        ImGuiID _dockLeftId;
        ImGuiID _dockCenterId;
        std::shared_ptr<FrameBuffer> _sceneBuffer;
        OptionalRef<Camera> _editorCam;
        std::optional<Entity> _selectedEntity;

        static const float _mainToolbarHeight;
        static const ImGuiWindowFlags _fixedFlags;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(Scene& scene);

        void renderMainMenu();
        void renderMainToolbar();
        void renderDockspace();
        void renderSceneTree();
        void renderInspector();
        void renderSceneView();
        void renderProject();

        void updateSceneSize(const glm::uvec2& size) noexcept;
        void onSceneTreeTransformClicked(Transform& trans);
    };
}