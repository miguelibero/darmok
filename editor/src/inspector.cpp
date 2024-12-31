#include <darmok-editor/inspector.hpp>
#include <darmok-editor/inspector/transform.hpp>
#include <darmok-editor/inspector/camera.hpp>
#include <darmok-editor/inspector/light.hpp>
#include <darmok-editor/inspector/scene.hpp>
#include <darmok-editor/inspector/render.hpp>
#include <darmok-editor/inspector/material.hpp>
#include <darmok-editor/inspector/program.hpp>
#include <darmok-editor/inspector/shape.hpp>
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
        _editors.add<CubeInspectorEditor>();
        _editors.add<SphereInspectorEditor>();
        _sceneEditor = _editors.add<SceneInspectorEditor>();
        _materialEditor = _editors.add<MaterialInspectorEditor>();
        _programEditor = _editors.add<ProgramInspectorEditor>();
    }

    void EditorInspectorView::init(EditorApp& app)
    {
        _editors.init(app);
    }

    void EditorInspectorView::shutdown()
    {
        _editors.shutdown();
        _sceneEditor.reset();
    }

    void EditorInspectorView::selectObject(const SelectableObject& obj, const std::shared_ptr<Scene>& scene)
    {
        _selected = obj;
        _scene = scene;
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
            else if (auto mat = getSelectedObject<MaterialAsset>())
            {
                _materialEditor->render(*mat.value());
            }
            else if (auto prog = getSelectedObject<ProgramAsset>())
            {
                if (auto ptr = std::get_if<std::shared_ptr<ProgramSource>>(&prog.value()))
                {
                    _programEditor->render(**ptr);
                }
            }
        }
        ImGui::End();
    }
}