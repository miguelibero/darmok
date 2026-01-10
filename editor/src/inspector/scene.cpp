#include <darmok-editor/inspector/scene.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/scene_assimp.hpp>
#include <imgui.h>

namespace darmok::editor
{
    std::string SceneInspectorEditor::getTitle() const noexcept
    {
        return "Scene";
    }

    SceneInspectorEditor::RenderResult SceneInspectorEditor::renderType(Scene::Definition& scene) noexcept
    {
        auto changed = false;
 
        if (ImguiUtils::drawProtobufInput("Name", "name", scene))
        {
            changed = true;
        }

        if (scene.scene_components_size() > 0)
        {
            if (ImguiUtils::beginFrame("Components"))
            {
                for (auto comp : SceneDefinitionWrapper{ scene }.getSceneComponents())
                {
                    auto result = renderChild(comp.get(), true);
                    if (!result)
                    {
                        return unexpected{ std::move(result).error() };
                    }
                    if (result.value())
                    {
                        changed = true;
                    }
                }
            }
            ImguiUtils::endFrame();
        }

        auto fileResult = _fileInput.draw(getApp());
        if (!fileResult)
        {
            return unexpected{ std::move(fileResult).error() };
        }
        if (fileResult.value() && _fileInput.scene)
        {
            auto& assets = getProject().getAssets();
            AssimpSceneDefinitionConverter::ImportConfig importConfig;
            AssimpSceneDefinitionConverter converter{ *_fileInput.scene, scene, importConfig, assets.getAllocator() };
            auto convertResult = converter();
            if (!convertResult)
            {
                return unexpected{ std::move(convertResult).error() };
            }
        }
        changed |= fileResult.value();

        return changed;
    }
}