#pragma once

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok-editor/project.hpp>
#include <darmok-editor/scene_view.hpp>
#include <darmok-editor/inspector.hpp>

#include <imgui.h>

namespace darmok
{
    class Scene;
    class ImguiAppComponent;
    class Camera;
    class Transform;
}

namespace darmok::editor
{
    class EditorAppDelegate final : public darmok::IAppDelegate, public darmok::IImguiRenderer
    {
    public:
		EditorAppDelegate(App& app) noexcept;
        ~EditorAppDelegate() noexcept;

        // darmok::IAppDelegate
        std::optional<int32_t> setup(const std::vector<std::string>& args) noexcept;
        void init() override;
		void shutdown() override;
		void update(float deltaTime) override;

        // darmok::IImguiRenderer
        void imguiSetup() override;
        void imguiRender() override;

    private:
        App& _app;

        OptionalRef<ImguiAppComponent> _imgui;
        EditorProject _proj;
        EditorSceneView _sceneView;
        EditorInspectorView _inspectorView;
        bool _scenePlaying;

        ImGuiID _dockDownId;
        ImGuiID _dockRightId;
        ImGuiID _dockLeftId;
        ImGuiID _dockCenterId;

        ImFont* _symbolsFont;
        float _mainToolbarHeight;

        static const ImGuiWindowFlags _fixedFlags;
        static const char* _sceneTreeWindowName;
        static const char* _materialsWindowName;
        static const char* _programsWindowName;

        void renderMainMenu();
        void renderMainToolbar();
        void renderDockspace();
        void renderSceneTree();
        void renderMaterials();
        void renderPrograms();

        void onSceneTreeTransformClicked(Transform& trans);
        void onSceneTreeSceneClicked();

        using SelectedObject = EditorInspectorView::SelectedObject;
        void onObjectSelected(SelectedObject obj) noexcept;

        void playScene();
        void stopScene();
        void pauseScene();

        void addEntityComponent(const entt::meta_type& type);


    };
}