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
#include <darmok-editor/inspector/physics3d.hpp>
#include <darmok-editor/inspector/text.hpp>
#include <darmok/scene_serialize.hpp>

#include <imgui.h>

namespace darmok::editor
{
    const std::string EditorInspectorView::_windowName = "Inspector";

    EditorInspectorView::EditorInspectorView() noexcept
        : _selected{ nullEntityId }
    {
    }

    expected<void, std::string> EditorInspectorView::setup() noexcept
    {
        _editors.add<SceneInspectorEditor>();
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
        _editors.add<PolygonInspectorEditor>();
        _editors.add<BoundingBoxInspectorEditor>();
        _editors.add<ConeInspectorEditor>();
        _editors.add<CylinderInspectorEditor>();
        _editors.add<MaterialInspectorEditor>();
        _editors.add<ProgramSourceInspectorEditor>();
        _editors.add<ProgramRefInspectorEditor>();
        _editors.add<MeshSourceInspectorEditor>();
        _editors.add<TextureInspectorEditor>();
        _editors.add<ArmatureInspectorEditor>();
        _editors.add<SkinnableInspectorEditor>();
        _editors.add<SkeletalAnimatorInspectorEditor>();
        _editors.add<SkeletalAnimationSceneComponentInspectorEditor>();
        _editors.add<SkeletalAnimationRenderComponentInspectorEditor>();
        _editors.add<Physics3dShapeInspectorEditor>();
        _editors.add<Physics3dBodyInspectorEditor>();
        _editors.add<Physics3dCharacterControllerInspectorEditor>();
        _editors.add<Physics3dSystemInspectorEditor>();
        _editors.add<FreetypeFontInspectorEditor>();
        _editors.add<TextInspectorEditor>();
        _editors.add<TextRendererInspectorEditor>();
        return {};
    }

    expected<void, std::string> EditorInspectorView::init(EditorApp& app) noexcept
    {
		_sceneDef = app.getProject().getSceneDefinition();
        return _editors.init(app);
    }

    expected<void, std::string> EditorInspectorView::shutdown() noexcept
    {
        _sceneDef.reset();
        return _editors.shutdown();
    }

    void EditorInspectorView::selectObject(const SelectableObject& obj) noexcept
    {
        _selected = obj;
    }

    EntityId EditorInspectorView::getSelectedEntity() const noexcept
    {
        auto ptr = std::get_if<EntityId>(&_selected);
        return ptr == nullptr ? nullEntityId : *ptr;
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

    const std::string& EditorInspectorView::getWindowName() noexcept
    {
        return _windowName;
    }

    EditorInspectorView::RenderResult EditorInspectorView::renderEntity(EntityId entity) noexcept
    {
        if (!_sceneDef)
        {
            return unexpected<std::string>{ "inspector view not initialized" };
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
			return unexpected{ StringUtils::joinErrors(errors) };
        }
        return changed;
    }

    EditorInspectorView::RenderResult EditorInspectorView::renderAsset(std::filesystem::path path) noexcept
    {
        if (!_sceneDef)
        {
            return unexpected<std::string>{ "inspector view not initialized" };
        }
        auto asset = _sceneDef->getAsset(std::move(path));
        if(!asset)
        {
            return {};
		}
        return _editors.render(*asset, true);
    }

    EditorInspectorView::RenderResult EditorInspectorView::render() noexcept
    {
        if (!_sceneDef)
        {
            return unexpected<std::string>{ "inspector view not initialized" };
        }
		RenderResult result = false;
        if (ImGui::Begin(_windowName.c_str()))
        {
            if (auto selectedPath = getSelectedAssetPath())
            {
                result = renderAsset(*selectedPath);
            }
            else
            {
                auto entity = getSelectedEntity();
                if (entity != nullEntityId)
                {
                    result = renderEntity(entity);
                }
                else 
                {
                    result = _editors.render(_sceneDef->getDefinition(), true);
                }
            }
        }
        ImGui::End();

        return result;
    }
}