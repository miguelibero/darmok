#include <darmok-editor/inspector/render.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/mesh.hpp>
#include <darmok/app.hpp>

#include <imgui.h>

namespace darmok::editor
{
	std::string RenderableInspectorEditor::getTitle() const noexcept
	{
		return "Renderable";
	}

	RenderableInspectorEditor::RenderResult RenderableInspectorEditor::renderType(Renderable::Definition& renderable) noexcept
	{
		auto changed = false;

		auto meshDragType = getApp().getAssetDragType<Mesh::Source>().value_or("");
		auto materialDragType = getApp().getAssetDragType<Material::Definition>().value_or("");

		auto result = ImguiUtils::drawProtobufAssetReferenceInput("Mesh", "mesh_path", renderable, meshDragType.c_str());
		if (result == ReferenceInputAction::Changed)
		{
			changed = true;
		}
		result = ImguiUtils::drawProtobufAssetReferenceInput("Material", "material_path", renderable, materialDragType.c_str());
		if (result == ReferenceInputAction::Changed)
		{
			changed = true;
		}

		return changed;
	}


	std::string PrefabInspectorEditor::getTitle() const noexcept
	{
		return "Prefab";
	}

	PrefabInspectorEditor::RenderResult PrefabInspectorEditor::renderType(protobuf::Prefab& prefab) noexcept
	{
		auto changed = false;
		auto sceneDragType = getApp().getAssetDragType<Scene::Definition>().value_or("");
		auto result = ImguiUtils::drawProtobufAssetReferenceInput("Scene", "scene_path", prefab, sceneDragType.c_str());
		if (result == ReferenceInputAction::Changed)
		{
			changed = true;
		}
		return changed;
	}

	PrefabInspectorEditor::RenderResult PrefabInspectorEditor::beforeRenderAny(Any& any, protobuf::Prefab& prefab) noexcept
	{
		_entityId = BaseObjectEditor::getEntityId(any);
		return ObjectEditor<protobuf::Prefab>::beforeRenderAny(any, prefab);
	}

	PrefabInspectorEditor::RenderResult PrefabInspectorEditor::afterRenderAny(Any& any, protobuf::Prefab& prefab, bool changed) noexcept
	{
		if (!_entityId)
		{
			return unexpected{ "missing entity" };
		}
		auto entityId = *_entityId;
		auto& proj = BaseObjectEditor::getProject();
		if (ImGui::Button("Remove Component"))
		{
			DARMOK_TRY(proj.removePrefab(entityId));
			changed = true;
		}
		else if (changed)
		{
			DARMOK_TRY(proj.updatePrefab(entityId, prefab.scene_path()));
		}
		return ObjectEditor<protobuf::Prefab>::afterRenderAny(any, prefab, changed);
	}
}