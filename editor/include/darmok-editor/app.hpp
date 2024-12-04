#pragma once

#include <darmok/app.hpp>
#include <darmok/scene.hpp>
#include <darmok/imgui.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok-editor/editor.hpp>

#include <memory>
#include <optional>
#include <filesystem>

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
    enum class MouseSceneViewMode
    {
        None,
        Look,
        Drag
    };

    enum class TransformGizmoMode
    {
        Grab,
        Translate,
        Rotate,
        Scale
    };

    class IObjectEditor;

    class EditorAppDelegate final : public darmok::IAppDelegate, public darmok::IImguiRenderer, darmok::ISceneDelegate
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

        // darmok::ISceneDelegate
        bool shouldCameraRender(const Camera& cam) const noexcept override;
        bool shouldEntityBeSerialized(Entity entity) const noexcept override;

    private:
        App& _app;
        std::shared_ptr<Scene> _scene;
        AssetPack _proj;
        OptionalRef<ImguiAppComponent> _imgui;
        MouseSceneViewMode _mouseSceneViewMode;
        TransformGizmoMode _transGizmoMode;
        bool _sceneViewFocused;
        bool _scenePlaying;

        ImGuiID _dockDownId;
        ImGuiID _dockRightId;
        ImGuiID _dockLeftId;
        ImGuiID _dockCenterId;
        std::shared_ptr<FrameBuffer> _sceneBuffer;
        OptionalRef<Camera> _editorCam;
        Entity _selectedEntity;
        bool _selectedScene;
        std::optional<std::filesystem::path> _projPath;
        ImFont* _symbolsFont;
        float _mainToolbarHeight;
        ObjectEditorContainer _inspectorEditors;

        static const ImGuiWindowFlags _fixedFlags;
        static const char* _sceneTreeWindowName;
        static const char* _sceneViewWindowName;
        static const char* _projectWindowName;
        static const char* _inspectorWindowName;
        static const std::vector<std::string> _sceneDialogFilters;

        void configureEditorScene(Scene& scene);
        void configureDefaultScene(Scene& scene);

        void renderMainMenu();
        void renderMainToolbar();
        void renderDockspace();
        void renderSceneTree();
        void renderInspector();
        void renderSceneView();
        void renderProject();
        void renderGizmos();

        void updateSceneSize(const glm::uvec2& size) noexcept;
        void onSceneTreeTransformClicked(Transform& trans);
        void onSceneTreeSceneClicked();

        bool isEditorEntity(Entity entity) const noexcept;

        void onEntitySelected(Entity entity) noexcept;

        void saveProject(bool forceNewPath = false);
        void openProject();
        void playScene();
        void stopScene();
        void pauseScene();

        void updateCamera(float dt);
        void updateInputEvents(float dt);
        void addEntityComponent(const entt::meta_type& type);
    };
}