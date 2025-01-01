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
    class Material;
    class ProgramDefinition;
}

namespace darmok::editor
{
    class EditorApp final : 
        public IAppDelegate, public IImguiRenderer,
        IEditorAssetsViewDelegate<MaterialAsset>,
        IEditorAssetsViewDelegate<ProgramAsset>,
        IEditorAssetsViewDelegate<MeshAsset>
    {
    public:
		EditorApp(App& app) noexcept;
        ~EditorApp() noexcept;

        // IAppDelegate
        std::optional<int32_t> setup(const CmdArgs& args) noexcept;
        void init() override;
		void shutdown() override;
		void update(float deltaTime) override;

        EditorProject& getProject() noexcept;
        const EditorProject& getProject() const noexcept;
        AssetContext& getAssets() noexcept;
        const AssetContext& getAssets() const noexcept;

        bool drawMaterialReference(const char* label, std::shared_ptr<Material>& mat);
        bool drawProgramReference(const char* label, std::shared_ptr<Program>& prog);
        bool drawMeshReference(const char* label, std::shared_ptr<IMesh>& mesh, const bgfx::VertexLayout& layout);

        // IImguiRenderer
        void imguiSetup() override;
        void imguiRender() override;

    private:
        App& _app;
        ProgramCompilerConfig _progCompConfig;

        OptionalRef<ImguiAppComponent> _imgui;
        EditorProject _proj;
        EditorSceneView _sceneView;
        EditorInspectorView _inspectorView;
        EditorAssetsView<MaterialAsset> _materialAssetsView;
        EditorAssetsView<ProgramAsset> _programAssetsView;
        EditorAssetsView<MeshAsset> _meshAssetsView;
        bool _scenePlaying;

        ImGuiID _dockDownId;
        ImGuiID _dockRightId;
        ImGuiID _dockLeftId;
        ImGuiID _dockCenterId;

        ImFont* _symbolsFont;
        float _mainToolbarHeight;

        static const ImGuiWindowFlags _fixedFlags;
        static const char* _sceneTreeWindowName;

        // IEditorAssetsViewDelegate<MaterialAsset>
        std::vector<MaterialAsset> getAssets(std::type_identity<MaterialAsset>) const override;
        std::optional<MaterialAsset> getSelectedAsset(std::type_identity<MaterialAsset>) const override;
        std::string getAssetName(const MaterialAsset& asset) const override;
        void onAssetSelected(const MaterialAsset& asset) override;
        void addAsset(std::type_identity<MaterialAsset>) override;

        // IEditorAssetsViewDelegate<ProgramAsset>
        std::vector<ProgramAsset> getAssets(std::type_identity<ProgramAsset>) const override;
        std::optional<ProgramAsset> getSelectedAsset(std::type_identity<ProgramAsset>) const override;
        std::string getAssetName(const ProgramAsset& asset) const override;
        void onAssetSelected(const ProgramAsset& asset) override;
        void addAsset(std::type_identity<ProgramAsset>) override;

        // IEditorAssetsViewDelegate<MeshAsset>
        std::vector<MeshAsset> getAssets(std::type_identity<MeshAsset>) const override;
        std::optional<MeshAsset> getSelectedAsset(std::type_identity<MeshAsset>) const override;
        std::string getAssetName(const MeshAsset& asset) const override;
        void onAssetSelected(const MeshAsset& asset) override;
        void addAsset(std::type_identity<MeshAsset>) override;

        void renderMainMenu();
        void renderMainToolbar();
        void renderDockspace();
        void renderSceneTree();

        void onSceneTreeTransformClicked(Transform& trans);
        void onSceneTreeSceneClicked();

        void onObjectSelected(const SelectableObject& obj) noexcept;

        void playScene();
        void stopScene();
        void pauseScene();

        void addEntityComponent(const entt::meta_type& type);


    };
}