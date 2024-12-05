#include <darmok-editor/inspector.hpp>
#include <darmok-editor/transform_inspector.hpp>
#include <darmok-editor/camera_inspector.hpp>
#include <darmok-editor/light_inspector.hpp>
#include <darmok-editor/scene_inspector.hpp>
#include <darmok/reflect.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

namespace darmok::editor
{
    const std::string EditorInspectorView::_windowName = "Inspector";

    EditorInspectorView::EditorInspectorView()
    : _selectedEntity(entt::null)
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
        _sceneEditor = _editors.add<SceneInspectorEditor>();
    }

    void EditorInspectorView::init(const std::shared_ptr<Scene>& scene)
    {
        _scene = scene;
        _editors.init();
    }

    void EditorInspectorView::shutdown()
    {
        _scene.reset();
        _editors.shutdown();
        _sceneEditor.reset();
    }

    EditorInspectorView& EditorInspectorView::selectEntity(Entity entity)
    {
        _selectedEntity = entity;
        return *this;
    }

    const std::string& EditorInspectorView::getWindowName()
    {
        return _windowName;
    }

    void EditorInspectorView::render()
    {
        if(!_scene)
        {
            return;
        }
        if (ImGui::Begin(_windowName.c_str()))
        {
            if (_selectedEntity != entt::null)
            {
                auto comps = SceneReflectionUtils::getEntityComponents(*_scene, _selectedEntity);
                _editors.render(comps.begin(), comps.end());
            }
            else if(_sceneEditor)
            {
                _sceneEditor->render(*_scene);
            }
        }
        ImGui::End();
    }
}