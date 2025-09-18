#include <darmok-editor/inspector/skeleton.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/assimp.hpp>
#include <darmok/skeleton_assimp.hpp>
#include <darmok/stream.hpp>

#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <fmt/format.h>

namespace darmok::editor
{

    std::string ArmatureInspectorEditor::getTitle() const noexcept
    {
        return "Armature";
    }

    expected<void, std::string> ArmatureInspectorEditor::loadSceneMesh(Armature::Definition& armature) noexcept
    {
        if (!_scene)
        {
            return unexpected{ "no scene loaded" };
        }
        if (_meshIndex < 0 || _meshIndex >= _meshes.size())
        {
            return unexpected{ "invalid mesh index" };
        }
        OptionalRef<const aiMesh> assimpMesh;
        auto& meshName = _meshes[_meshIndex];

        for (size_t i = 0; i < _scene->mNumMeshes; i++)
        {
            auto& m = _scene->mMeshes[i];
            if (AssimpUtils::getStringView(m->mName) == meshName)
            {
                assimpMesh = m;
                break;
            }
        }

        if (!assimpMesh)
        {
            return unexpected{ "selected mesh not found in scene" };
        }

        AssimpArmatureDefinitionConverter convert{ *assimpMesh, armature };
        return convert();
    }

    ArmatureInspectorEditor::RenderResult ArmatureInspectorEditor::renderType(Armature::Definition& armature) noexcept
    {
        auto changed = false;
        if (ImguiUtils::drawProtobufInput("Name", "name", armature))
        {
            changed = true;
        }

        std::filesystem::path path;
        FileDialogOptions dialogOptions;
        dialogOptions.filters = { "*.fbx", "*.glb" };
		dialogOptions.filterDesc = "3D Model Files";
        if (getApp().drawFileInput("Load File", path, dialogOptions))
        {
            auto& assets = getApp().getAssets();
            auto dataResult = assets.getDataLoader()(path);
            if (!dataResult)
            {
                return unexpected{ std::move(dataResult).error() };
            }
            AssimpLoader loader;
            AssimpLoader::Config config;
            config.setPath(path);
            auto sceneResult = loader.loadFromMemory(dataResult.value(), config);
            if (!sceneResult)
            {
                return unexpected{ std::move(sceneResult).error() };
            }
            _scene = std::move(*sceneResult);
            _meshes.clear();
            _meshIndex = 0;
            for (size_t i = 0; i < _scene->mNumMeshes; i++)
            {
                auto mesh = _scene->mMeshes[i];
                if(mesh->mNumBones == 0)
                {
                    continue;
				}
                _meshes.push_back(AssimpUtils::getString(mesh->mName));
            }
            auto result = loadSceneMesh(armature);
            if (!result)
            {
                return unexpected{ std::move(result).error() };
            }
            changed = true;
        }
        if (!_meshes.empty())
        {
            if (ImguiUtils::drawListCombo("Mesh Name", _meshIndex, _meshes))
            {
                auto result = loadSceneMesh(armature);
                if (!result)
                {
                    return unexpected{ std::move(result).error() };
                }
                changed = true;
            }
        }

        if (armature.joints_size() > 0)
        {
            auto desc = fmt::format("{} joints", armature.joints_size());
            ImGui::Text(desc.c_str());
        }

        return changed;
    }

    std::string SkinnableInspectorEditor::getTitle() const noexcept
    {
        return "Skinnable";
    }

    SkinnableInspectorEditor::RenderResult SkinnableInspectorEditor::renderType(Skinnable::Definition& skinnable) noexcept
    {
        auto changed = false;

        auto armatureDragType = getApp().getAssetDragType<Armature::Definition>().value_or("");

        auto result = ImguiUtils::drawProtobufAssetReferenceInput("Armature", "armature_path", skinnable, armatureDragType.c_str());
        if (result == ReferenceInputAction::Changed)
        {
            changed = true;
        }

        return changed;
    }

    std::string SkeletalAnimatorInspectorEditor::getTitle() const noexcept
    {
        return "Skeletal Animator";
    }

    SkeletalAnimatorInspectorEditor::RenderResult SkeletalAnimatorInspectorEditor::renderType(SkeletalAnimator::Definition& animator) noexcept
    {
        auto changed = false;
        std::filesystem::path path;
        FileDialogOptions dialogOptions;
        dialogOptions.filters = { "*.json" };
        if (getApp().drawFileInput("Load File", path, dialogOptions))
        {
            auto jsonResult = StreamUtils::parseJson(path);
            if(!jsonResult)
            {
                return unexpected{ "error parsing json: " + jsonResult.error() };
            }

            SkeletalAnimatorDefinitionWrapper wrapper{ animator };
            auto result = wrapper.read(*jsonResult);
            if (!result)
            {
                return unexpected{ result.error() };
            }
            changed = true;
        }
        if(animator.states_size() > 0 || animator.transitions_size() > 0)
        {
            auto desc = fmt::format("{} states, {} transitions", animator.states_size(), animator.transitions_size());
            ImGui::Text(desc.c_str());
		}
        return changed;
    }
}