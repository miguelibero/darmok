#include <darmok-editor/inspector.hpp>
#include <darmok-editor/transform_inspector.hpp>
#include <darmok-editor/camera_inspector.hpp>
#include <darmok-editor/light_inspector.hpp>
#include <darmok-editor/scene_inspector.hpp>
#include <darmok-editor/render_inspector.hpp>
#include <darmok-editor/material_inspector.hpp>
#include <darmok/reflect.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    const std::string EditorInspectorView::_windowName = "Inspector";

    EditorInspectorView::EditorInspectorView()
    : _selected(Entity(entt::null))
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
        _sceneEditor = _editors.add<SceneInspectorEditor>();
        _materialEditor = _editors.add<MaterialInspectorEditor>();
    }

    void EditorInspectorView::init()
    {
        _editors.init();
    }

    void EditorInspectorView::shutdown()
    {
        _editors.shutdown();
        _sceneEditor.reset();
    }

    EditorInspectorView& EditorInspectorView::selectObject(SelectedObject obj, const std::shared_ptr<Scene>& scene)
    {
        _selected = obj;
        _scene = scene;
        return *this;
    }

    std::shared_ptr<Scene> EditorInspectorView::getSelectedScene() const noexcept
    {
        auto ptr = std::get_if<Entity>(&_selected);
        if (ptr != nullptr && *ptr == entt::null)
        {
            return _scene;
        }
        return nullptr;
    }

    std::shared_ptr<Material> EditorInspectorView::getSelectedMaterial() const noexcept
    {
        auto ptr = std::get_if<std::shared_ptr<Material>>(&_selected);
        return ptr == nullptr ? nullptr : *ptr;
    }

    Entity EditorInspectorView::getSelectedEntity() const noexcept
    {
        auto ptr = std::get_if<Entity>(&_selected);
        return ptr == nullptr ? entt::null : *ptr;
    }

    const std::string& EditorInspectorView::getWindowName()
    {
        return _windowName;
    }

    void EditorInspectorView::render()
    {
        if (ImGui::Begin(_windowName.c_str()))
        {
            auto entity = getSelectedEntity();
            if (entity != entt::null)
            {
                auto comps = SceneReflectionUtils::getEntityComponents(*_scene, entity);
                _editors.render(comps.begin(), comps.end());
            }
            else if(auto scene = getSelectedScene())
            {
                _sceneEditor->render(*scene);
            }
            else if (auto mat = getSelectedMaterial())
            {
                _materialEditor->render(*mat);
            }
        }
        ImGui::End();
    }
}