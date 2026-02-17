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
#include <darmok-editor/inspector/script.hpp>
#include <darmok/scene_serialize.hpp>

#include <imgui.h>

namespace darmok::editor
{
    const std::string EditorInspectorView::_windowName = "Inspector";

    EditorInspectorView::EditorInspectorView() noexcept
        : _selected{ nullEntityId }
    {
    }

    expected<void, std::string> EditorInspectorView::addEditor(std::unique_ptr<IObjectEditor> editor) noexcept
    {
        return _editors.add(std::move(editor));
    }

    expected<void, std::string> EditorInspectorView::setup() noexcept
    {
        DARMOK_TRY(addEditor<SceneInspectorEditor>());
        DARMOK_TRY(addEditor<TransformInspectorEditor>());
        DARMOK_TRY(addEditor<CameraInspectorEditor>());
        DARMOK_TRY(addEditor<PointLightInspectorEditor>());
        DARMOK_TRY(addEditor<DirectionalLightInspectorEditor>());
        DARMOK_TRY(addEditor<SpotLightInspectorEditor>());
        DARMOK_TRY(addEditor<AmbientLightInspectorEditor>());
        DARMOK_TRY(addEditor<RenderableInspectorEditor>());
        DARMOK_TRY(addEditor<PrefabInspectorEditor>());
        DARMOK_TRY(addEditor<CubeInspectorEditor>());
        DARMOK_TRY(addEditor<SphereInspectorEditor>());
        DARMOK_TRY(addEditor<CapsuleInspectorEditor>());
        DARMOK_TRY(addEditor<RectangleInspectorEditor>());
        DARMOK_TRY(addEditor<PolygonInspectorEditor>());
        DARMOK_TRY(addEditor<BoundingBoxInspectorEditor>());
        DARMOK_TRY(addEditor<ConeInspectorEditor>());
        DARMOK_TRY(addEditor<CylinderInspectorEditor>());
        DARMOK_TRY(addEditor<MaterialInspectorEditor>());
        DARMOK_TRY(addEditor<ProgramSourceInspectorEditor>());
        DARMOK_TRY(addEditor<ProgramRefInspectorEditor>());
        DARMOK_TRY(addEditor<MeshSourceInspectorEditor>());
        DARMOK_TRY(addEditor<TextureInspectorEditor>());

        DARMOK_TRY(addEditor<LuaScriptInspectorEditor>());
        DARMOK_TRY(addEditor<LuaScriptRunnerInspectorEditor>());
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
        for (auto& comp : _sceneDef->getComponents(entity))
        {
            ImGui::PushID(i);
            auto result = _editors.render(comp, true);
            ImGui::PopID();
            if (!result)
            {
                ImguiUtils::drawProtobufError(comp, result.error());
            }
            else if (result.value())
            {
                changed = true;
            }
            ++i;
        }
        return {};
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