#pragma once

#include <darmok/app.hpp>
#include <darmok/imgui.hpp>
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
        std::optional<int32_t> setup(const CmdArgs& args) noexcept override;
        void init() override;
		void shutdown() override;
		void update(float deltaTime) override;

        // IImguiRenderer
        void imguiSetup() override;
        void imguiRender() override;

        EditorProject& getProject() noexcept;
        const EditorProject& getProject() const noexcept;
        AssetContext& getAssets() noexcept;
        const AssetContext& getAssets() const noexcept;

        Entity addEntity() noexcept;

        template<typename T, typename... A>
        OptionalRef<T> addEntityComponent(A&&... args) noexcept
        {
            auto scene = _proj.getScene();
            if (!scene)
            {
                return nullptr;
            }
            auto entity = _inspectorView.getSelectedEntity();
            if (entity == entt::null)
            {
                return nullptr;
            }
            return scene->addComponent<T>(entity, std::forward<A>(args)...);
        }

		std::optional<std::string> getAssetDragType(uint32_t assetType) const noexcept;

        template<typename T>
        std::optional<std::string> getAssetDragType() const noexcept
        {
            return getAssetDragType(protobuf::getTypeId<T>());
        }

    private:
        App& _app;
        ProgramCompilerConfig _progCompConfig;

        OptionalRef<ImguiAppComponent> _imgui;
        EditorProject _proj;
        EditorSceneView _sceneView;
        EditorInspectorView _inspectorView;
        std::vector<EditorAssetsView> _assetsViews;
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
        virtual std::optional<std::string> getSelectedAssetPath(uint32_t assetType) const override;
        virtual void onAssetPathSelected(uint32_t assetType, const std::string& assetPath) override;

        void renderMainMenu();
        void renderMainToolbar();
        void renderDockspace();
        bool renderSceneTree();

        using TransformDefinition = protobuf::Transform;

        bool renderEntityDragDropTarget(Entity entity);
        bool renderSceneTreeBranch(Entity entity);
        void onSceneTreeTransformClicked(Entity entity);
        void onSceneTreeSceneClicked();

        void onObjectSelected(const SelectableObject& obj) noexcept;

        void playScene();
        void stopScene();
        void pauseScene();

        Entity getSelectedEntity() noexcept;

        template<typename T, typename... A>
        void drawEntityComponentMenu(const char* name, A&&... args) noexcept
        {
            auto scene = _proj.getScene();
            auto disabled = true;
            if (scene)
            {
                auto entity = _inspectorView.getSelectedEntity();
                if (entity != entt::null && !scene->hasComponent<T>(entity))
                {
                    disabled = false;
                }
            }
            ImGui::BeginDisabled(disabled);
            if (ImGui::MenuItem(name))
            {
                addEntityComponent<T>(std::forward<A>(args)...);
            }
            ImGui::EndDisabled();
        }

    };
}