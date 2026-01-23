#pragma once

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
#include <darmok/window.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok-editor/project.hpp>
#include <darmok-editor/scene_view.hpp>
#include <darmok-editor/player_view.hpp>
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
    enum class EditorAppMainMenuSection
    {
        File,
        Edit,
        AddAsset,
        AddSceneComponent,
        AddEntityComponent,
        AddCameraComponent,
        Help,
        Main,
    };

    class EditorApp;

    class IEditorAppComponent
    {
    public:
        virtual ~IEditorAppComponent() = default;
        using MainMenuSection = EditorAppMainMenuSection;
        virtual expected<void, std::string> renderMainMenu(MainMenuSection section) noexcept { return {}; };
        virtual expected<void, std::string> init(EditorApp& app) noexcept { return {}; };
        virtual expected<void, std::string> shutdown() noexcept { return {}; };
        virtual expected<void, std::string> render() noexcept { return {}; };
        virtual expected<void, std::string> update(float deltaTime) noexcept { return {}; };
    };

    class EditorApp final : 
        public IAppDelegate, public IImguiRenderer,
        IEditorAssetsViewDelegate
    {
    public:
        using MainMenuSection = EditorAppMainMenuSection;

		EditorApp(App& app) noexcept;
        ~EditorApp() noexcept;

        static constexpr const char* entityDragType = "Entity";

        // IAppDelegate
        expected<int32_t, std::string> setup(const CmdArgs& args) noexcept override;
        expected<void, std::string> init() noexcept override;
        expected<void, std::string> shutdown() noexcept override;
        expected<void, std::string> update(float deltaTime) noexcept override;

        // IImguiRenderer
        expected<void, std::string> imguiSetup() noexcept override;
        expected<void, std::string> imguiRender() noexcept override;

        EditorProject& getProject() noexcept;
        const EditorProject& getProject() const noexcept;
        const EditorInspectorView& getInspectorView() const noexcept;
        EditorInspectorView& getInspectorView() noexcept;
        const EditorSceneView& getSceneView() const noexcept;
        EditorSceneView& getSceneView() noexcept;

        AssetContext& getAssets() noexcept;
        const AssetContext& getAssets() const noexcept;
		const Window& getWindow() const noexcept;   
        Window& getWindow() noexcept;
        const Input& getInput() const noexcept;
        Input& getInput() noexcept;

        void requestRenderReset() noexcept;

        template<typename T>
        std::optional<std::string> getAssetDragType() const noexcept
        {
            return _assetsView.getAssetDragType(protobuf::getTypeId<T>());
        }

        bool drawFileInput(const char* label, std::filesystem::path& path, FileDialogOptions options) noexcept;
        EntityId getSelectedEntity() const noexcept;

        expected<void, std::string> addComponent(std::unique_ptr<IEditorAppComponent> comp) noexcept;

        template<typename T, typename... A>
        expected<std::reference_wrapper<T>, std::string> addComponent(A&&... args)
        {
            auto ptr = std::make_unique<T>(std::forward<A>(args)...);
            auto& ref = *ptr;
            auto result = addComponent(std::move(ptr));
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            return std::ref(ref);
        }

        expected<bool, std::string> drawAssetComponentMenu(const char* name, const google::protobuf::Message& asset) noexcept;

        template<typename T, typename Def = T::Definition>
        expected<bool, std::string> drawSceneComponentMenu(const char* name, const Def& def = T::createDefinition()) noexcept
        {
            ImGui::BeginDisabled(!canAddSceneComponent(protobuf::getTypeId<Def>()));
            expected<bool, std::string> result{ false };
            if (ImGui::MenuItem(name))
            {
                if (auto addResult = _proj.addSceneComponent<T, Def>(def))
                {
                    result = true;
                }
                else
                {
                    result = unexpected{ std::move(result).error() };
                }
            }
            ImGui::EndDisabled();
            return result;
        }

        template<typename T, typename Def = T::Definition>
        expected<bool, std::string> drawEntityComponentMenu(const char* name, const Def& def = T::createDefinition()) noexcept
        {
            ImGui::BeginDisabled(!canAddEntityComponent(protobuf::getTypeId<Def>()));
            expected<bool, std::string> result{ false };
            if (ImGui::MenuItem(name))
            {
                auto entity = _inspectorView.getSelectedEntity();
                if (auto addResult = _proj.addEntityComponent<T, Def>(entity, def))
                {
                    result = true;
                }
                else
                {
                    result = unexpected{ std::move(addResult).error() };
                }
            }
            ImGui::EndDisabled();
            return result;
        }

        template<typename T, typename Def = T::Definition>
        expected<bool, std::string> drawCameraComponentMenu(const char* name, const Def& def = T::createDefinition()) noexcept
        {
            ImGui::BeginDisabled(!canAddCameraComponent(protobuf::getTypeId<Def>()));
            expected<bool, std::string> result{ false };
            if (ImGui::MenuItem(name))
            {
                auto entity = _inspectorView.getSelectedEntity();
                if (auto addResult = _proj.addCameraComponent<T, Def>(entity, def))
                {
                    result = true;
                }
                else
                {
                    result = unexpected{ std::move(addResult).error() };
                }
            }
            ImGui::EndDisabled();
            return result;
        }

        void showError(const std::string& error) noexcept;

    private:
        App& _app;
        ProgramCompilerConfig _progCompConfig;
        std::unordered_map<void*, std::optional<FileDialogResult>> _fileInputResults;
        OptionalRef<ImguiAppComponent> _imgui;
        EditorProject _proj;
        EditorSceneView _sceneView;
        EditorPlayerView _playerView;
        EditorInspectorView _inspectorView;
        EditorAssetsView _assetsView;

        ImGuiID _dockDownId;
        ImGuiID _dockRightId;
        ImGuiID _dockLeftId;
        ImGuiID _dockCenterId;

        ImFont* _symbolsFont;
        float _mainToolbarHeight;

        static const ImGuiWindowFlags _fixedFlags;
        static const char* _sceneTreeWindowName;
        std::vector<std::unique_ptr<IEditorAppComponent>> _comps;
        std::string _errorPopup;

        // IEditorAssetsViewDelegate
        std::optional<std::filesystem::path> getSelectedAssetPath() const noexcept override;
        void onAssetPathSelected(const std::filesystem::path& assetPath) noexcept override;
        void onAssetFolderEntered(const std::filesystem::path& assetPath) noexcept override;

        expected<void, std::string> renderMainMenu() noexcept;
        expected<void, std::string> renderMainToolbar() noexcept;
        void renderDockspace() noexcept;
        expected<bool, std::string> renderSceneTree() noexcept;
        void renderAboutDialog() noexcept;

        using TransformDefinition = protobuf::Transform;

        bool renderEntityDragDropTarget(EntityId entityId) noexcept;
        bool renderSceneTreeBranch(EntityId entityId) noexcept;
        void onSceneTreeEntityClicked(EntityId entityId) noexcept;
        void onSceneTreeEntityFocused(EntityId entityId) noexcept;
        void onSceneTreeSceneClicked() noexcept;
        expected<void, std::string> onMainMenuRender(MainMenuSection section) noexcept;

        void onObjectSelected(const SelectableObject& obj) noexcept;

        bool canAddSceneComponent(IdType typeId) const noexcept;
        bool canAddEntityComponent(IdType typeId) const noexcept;
        bool canAddCameraComponent(IdType typeId) const noexcept;
        void focusNextWindowOnPlaybackChange(bool played) noexcept;
    };
}