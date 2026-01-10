#include <darmok-editor/skeleton.hpp>
#include <darmok-editor/inspector/skeleton.hpp>

namespace darmok::editor
{
    expected<void, std::string> SkeletonEditorAppComponent::renderMainMenu(MainMenuSection section) noexcept
    {
        if (!_app)
        {
            return unexpected{ "not initialized" };
        }
        if (section == MainMenuSection::AddAsset)
        {
            if (ImGui::BeginMenu("Animation"))
            {
                DARMOK_TRY(_app->drawAssetComponentMenu("Armature", Armature::createDefinition()));
                DARMOK_TRY(_app->drawAssetComponentMenu("Animator", SkeletalAnimator::createDefinition()));
                ImGui::EndMenu();
            }
        }
        else if (section == MainMenuSection::AddEntityComponent)
        {
            if (ImGui::BeginMenu("Skeletal Animation"))
            {
                DARMOK_TRY(_app->drawEntityComponentMenu<Skinnable>("Skinnable"));
                DARMOK_TRY(_app->drawEntityComponentMenu<SkeletalAnimator>("Animator"));
                ImGui::EndMenu();
            }
        }
        else if (section == MainMenuSection::AddSceneComponent)
        {
            DARMOK_TRY(_app->drawSceneComponentMenu<SkeletalAnimationSceneComponent>("Skeletal Animation"));
        }
        else if (section == MainMenuSection::AddCameraComponent)
        {
            DARMOK_TRY(_app->drawCameraComponentMenu<SkeletalAnimationRenderComponent>("Skeletal Animation"));
        }
        return {};
    }

    expected<void, std::string> SkeletonEditorAppComponent::init(EditorApp& app) noexcept
    {
        _app = app;
        auto& inspector = app.getInspectorView();
        DARMOK_TRY(inspector.addEditor<ArmatureInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<SkinnableInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<SkeletalAnimatorInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<SkeletalAnimationSceneComponentInspectorEditor>());
        DARMOK_TRY(inspector.addEditor<SkeletalAnimationRenderComponentInspectorEditor>());
        return {};
    }

    expected<void, std::string> SkeletonEditorAppComponent::shutdown() noexcept
    {
        _app.reset();
        return {};
    }
}