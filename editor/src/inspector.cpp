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
#include <darmok/scene_serialize.hpp>

#include <imgui.h>

namespace darmok::editor
{
    const std::string EditorInspectorView::_windowName = "Inspector";

    EditorInspectorView::EditorInspectorView()
        : _selected{ Entity{entt::null} }
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
        _editors.add<SceneInspectorEditor>();
        _editors.add<MaterialInspectorEditor>();
        _editors.add<ProgramSourceInspectorEditor>();
        _editors.add<MeshSourceInspectorEditor>();
        _editors.add<TextureDefinitionInspectorEditor>();
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

    Entity EditorInspectorView::getSelectedEntity() const noexcept
    {
        auto ptr = std::get_if<Entity>(&_selected);
        return ptr == nullptr ? entt::null : *ptr;
    }

    bool EditorInspectorView::isSceneSelected() const noexcept
    {
        auto ptr = std::get_if<Entity>(&_selected);
		return ptr != nullptr && *ptr == entt::null;
    }

    OptionalRef<const AssetHandler> EditorInspectorView::getSelectedAsset() const noexcept
    {
        auto ptr = std::get_if<AssetHandler>(&_selected);
		return ptr;
    }

    const std::string& EditorInspectorView::getWindowName()
    {
        return _windowName;
    }

    bool EditorInspectorView::render()
    {
        if (!_sceneDef)
        {
            return false;
        }
        auto& sceneDef = *_sceneDef;
		bool changed = false;
        if (ImGui::Begin(_windowName.c_str()))
        {
            auto entity = getSelectedEntity();
            if (entity != entt::null)
            {
                auto comps = sceneDef.getComponents(entity);
                if (_editors.render(comps.begin(), comps.end()))
                {
                    changed = true;
                }
            }
            else if (auto selected = getSelectedAsset())
            {
                if (auto asset = sceneDef.getAsset(selected->type, selected->path))
                {
                    if (_editors.render(*asset))
                    {
                        changed = true;
                    }
                }
			}
        }
        ImGui::End();
        return changed;
    }
}