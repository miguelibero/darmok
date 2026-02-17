#include <darmok-editor/inspector/scene.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
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
                for (auto compRef : SceneDefinitionWrapper{ scene }.getSceneComponents())
                {
                    auto& comp = compRef.get();
                    auto result = renderChild(comp, true);
                    if (!result)
                    {
                        ImguiUtils::drawProtobufError(comp, result.error());
                    }
                    else if (result.value())
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

        SceneDefinitionWrapper wrapper{ scene };

        if (fileResult.value() && _fileInput.scene)
        {
            auto& assets = getProject().getAssets();
            AssimpSceneDefinitionConverter::ImportConfig importConfig;
            importConfig.set_embed_textures(true);
            importConfig.mutable_program()->set_standard(Program::Standard::Forward);
            importConfig.mutable_program_source()->set_standard(Program::Standard::Forward);
            FileDataLoader dataLoader;
            dataLoader.setAbsolutePathsAllowed(true);
            dataLoader.setBasePath(_fileInput.path.parent_path());
            dataLoader.addRootPath(_fileInput.path.parent_path());
            ImageTextureSourceLoader texLoader{ dataLoader };
            ProgramSourceLoader progLoader{ dataLoader };

			scene = Scene::Definition{};
            AssimpSceneDefinitionConverter converter{ *_fileInput.scene, scene, importConfig, assets.getAllocator(), texLoader, progLoader };
            auto convertResult = converter();
            if (!convertResult)
            {
                getApp().showError(convertResult.error());
            }
            else
            {
                changed = true;
            }
        }

		auto transCount = wrapper.getComponentCount<protobuf::Transform>();
        if (transCount > 0)
        {
			ImGui::Text("%d transforms", transCount);
        }

        return changed;
    }
}