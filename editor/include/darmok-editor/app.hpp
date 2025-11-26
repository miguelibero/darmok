#pragma once

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok-editor/project.hpp>
#include <darmok-editor/scene_view.hpp>
#include <darmok-editor/inspector.hpp>
#include <darmok-editor/assets_view.hpp>
#include <darmok-editor/asset_fwd.hpp>

#include <imgui.h>

namespace darmok
{
    class Scene;
    class ImguiAppComponent;
    class Camera;
    class Transform;
    struct Material;
    class ProgramDefinition;

    namespace protobuf
    {
        class Transform;
    }
}

namespace darmok::editor
{
    class EditorApp final : 
        public IAppDelegate, public IImguiRenderer,
        IEditorAssetsViewDelegate
    {
    public:
		EditorApp(App& app) noexcept;
        ~EditorApp() noexcept;

        static constexpr const char* entityDragType = "Entity";

        // IAppDelegate
        expected<int32_t, std::string> setup(const CmdArgs& args) noexcept override;
        expected<void, std::string> init() noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;

        // IImguiRenderer
        void imguiSetup() override;
        void imguiRender() override;

        EditorProject& getProject() noexcept;
        const EditorProject& getProject() const noexcept;
        AssetContext& getAssets() noexcept;
        const AssetContext& getAssets() const noexcept;
		const Window& getWindow() const noexcept;   
        Window& getWindow() noexcept;

        template<typename T>
        std::optional<std::string> getAssetDragType() const noexcept
        {
            return _assetsView.getAssetDragType(protobuf::getTypeId<T>());
        }

        bool drawFileInput(const char* label, std::filesystem::path& path, FileDialogOptions options) noexcept;

    private:
        App& _app;
        ProgramCompilerConfig _progCompConfig;
        std::unordered_map<void*, std::optional<FileDialogResult>> _fileInputResults;

        OptionalRef<ImguiAppComponent> _imgui;
        EditorProject _proj;
        EditorSceneView _sceneView;
        EditorInspectorView _inspectorView;
        EditorAssetsView _assetsView;
        bool _scenePlaying;

        ImGuiID _dockDownId;
        ImGuiID _dockRightId;
        ImGuiID _dockLeftId;
        ImGuiID _dockCenterId;

        ImFont* _symbolsFont;
        float _mainToolbarHeight;

        static const ImGuiWindowFlags _fixedFlags;
        static const char* _sceneTreeWindowName;

        // IEditorAssetsViewDelegate
        std::optional<std::filesystem::path> getSelectedAssetPath() const override;
        void onAssetPathSelected(const std::filesystem::path& assetPath) override;

        void renderMainMenu();
        void renderMainToolbar();
        void renderDockspace();
        bool renderSceneTree();
        void renderAboutDialog();

        using TransformDefinition = protobuf::Transform;

        bool renderEntityDragDropTarget(EntityId entity);
        bool renderSceneTreeBranch(EntityId entity);
        void onSceneTreeTransformClicked(EntityId entity);
        void onSceneTreeSceneClicked();

        void onObjectSelected(const SelectableObject& obj) noexcept;

        void playScene();
        void stopScene();
        void pauseScene();

        EntityId getSelectedEntity() const noexcept;

        void drawEntityComponentMenu(const char* name, const google::protobuf::Message& comp) noexcept;
        void drawAssetComponentMenu(const char* name, const google::protobuf::Message& asset) noexcept;
    };
}