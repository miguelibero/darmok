#include <darmok-editor/inspector/render.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/mesh.hpp>

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

	PrefabInspectorEditor::RenderResult PrefabInspectorEditor::renderType(Prefab::Definition& prefab) noexcept
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
}