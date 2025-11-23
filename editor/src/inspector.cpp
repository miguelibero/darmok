#include <darmok-editor/inspector.hpp>
#include <darmok-editor/app.hpp>
#include <darmok-editor/project.hpp>
#include <darmok-editor/inspector/transform.hpp>
#include <darmok-editor/inspector/camera.hpp>
#include <darmok-editor/inspector/light.hpp>
#include <darmok-editor/inspector/scene.hpp>
#include <darmok-editor/inspector/render.hpp>
#include <darmok-editor/inspector/material.hpp>
#include <darmok-editor/inspector/program.hpp>
#include <darmok-editor/inspector/shape.hpp>
#include <darmok-editor/inspector/mesh.hpp>
#include <darmok-editor/inspector/texture.hpp>
#include <darmok-editor/inspector/skeleton.hpp>
#include <darmok/scene_serialize.hpp>

#include <imgui.h>

namespace darmok::editor
{
    const std::string EditorInspectorView::_windowName = "Inspector";

    EditorInspectorView::EditorInspectorView()
        : _selected{ nullEntityId }
    {
    }

    void EditorInspectorView::setup()
    {
        _editors.add<TransformInspectorEditor>();
        _editors.add<CameraInspectorEditor>();
        _editors.add<PointLightInspectorEditor>();
        _editors.add<DirectionalLightInspectorEditor>();
        _editors.add<SpotLightInspectorEditor>();
        _editors.add<AmbientLightInspectorEditor>();
        _editors.add<RenderableInspectorEditor>();
        _editors.add<CubeInspectorEditor>();
        _editors.add<SphereInspectorEditor>();
        _editors.add<CapsuleInspectorEditor>();
        _editors.add<RectangleInspectorEditor>();
        _editors.add<SceneInspectorEditor>();
        _editors.add<MaterialInspectorEditor>();
        _editors.add<ProgramSourceInspectorEditor>();
        _editors.add<ProgramRefInspectorEditor>();
        _editors.add<MeshSourceInspectorEditor>();
        _editors.add<TextureInspectorEditor>();
        _editors.add<ArmatureInspectorEditor>();
        _editors.add<SkinnableInspectorEditor>();
        _editors.add<SkeletalAnimatorInspectorEditor>();
    }

    void EditorInspectorView::init(EditorApp& app)
    {
        _editors.init(app);
		_sceneDef = app.getProject().getSceneDefinition();
    }

    void EditorInspectorView::shutdown()
    {
        _sceneDef.reset();
        _editors.shutdown();
    }

    void EditorInspectorView::selectObject(const SelectableObject& obj)
    {
        _selected = obj;
    }

    EntityId EditorInspectorView::getSelectedEntity() const noexcept
    {
        auto ptr = std::get_if<EntityId>(&_selected);
        return ptr == nullptr ? entt::null : *ptr;
    }

    bool EditorInspectorView::isSceneSelected() const noexcept
    {
        auto ptr = std::get_if<EntityId>(&_selected);
		return ptr != nullptr && *ptr == 0;
    }

    std::optional<std::filesystem::path> EditorInspectorView::getSelectedAssetPath() const noexcept
    {
        if (auto ptr = std::get_if<std::filesystem::path>(&_selected))
        {
            return *ptr;
        }
		return std::nullopt;
    }

    const std::string& EditorInspectorView::getWindowName()
    {
        return _windowName;
    }

    EditorInspectorView::RenderResult EditorInspectorView::renderEntity(EntityId entity)
    {
        if (!_sceneDef)
        {
            return unexpected{ "inspector view not initialized" };
        }
        bool changed = false;
        int i = 0;
		std::vector<std::string> errors;
        for (auto& comp : _sceneDef->getComponents(entity))
        {
            ImGui::PushID(i);
            auto result = _editors.render(comp, true);
            ImGui::PopID();
            if (!result)
            {
                errors.push_back(std::move(result).error());
            }
            else if (result.value())
            {
                changed = true;
            }
            ++i;
        }
        if (!errors.empty())
        {
			return unexpected{ StringUtils::joinExpectedErrors(errors) };
        }
        return changed;
    }

    EditorInspectorView::RenderResult EditorInspectorView::renderAsset(std::filesystem::path path)
    {
        if (!_sceneDef)
        {
            return unexpected{ "inspector view not initialized" };
        }
        auto asset = _sceneDef->getAsset(std::move(path));
        if(!asset)
        {
            return unexpected{ "could not find asset" };
		}
        return _editors.render(*asset, true);
    }

    EditorInspectorView::RenderResult EditorInspectorView::render()
    {
        if (!_sceneDef)
        {
            return unexpected{ "inspector view not initialized" };
        }
		RenderResult result = false;
        if (ImGui::Begin(_windowName.c_str()))
        {
            auto entity = getSelectedEntity();
            if (entity != entt::null)
            {
                result = renderEntity(entity);
            }
            else if (auto selectedPath = getSelectedAssetPath())
            {
				result = renderAsset(*selectedPath);
			}
        }
        ImGui::End();

        if (!result)
        {
            return unexpected{ std::move(result).error() };
        }
        return *result;
    }
}