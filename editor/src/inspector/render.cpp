#include <darmok-editor/inspector/render.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok-editor/app.hpp>
#include <darmok/program.hpp>
#include <darmok/asset.hpp>
#include <darmok/mesh.hpp>
#include <darmok/app.hpp>

#include <imgui.h>

namespace darmok::editor
{
	void RenderableInspectorEditor::init(EditorApp& app, ObjectEditorContainer& container)
	{
		_app = app;
	}

	void RenderableInspectorEditor::shutdown()
	{
		_app.reset();
	}

	std::string RenderableInspectorEditor::getTitle() const noexcept
	{
		return "Renderable";
	}

	bool RenderableInspectorEditor::renderType(Renderable::Definition& renderable) noexcept
	{
		auto changed = false;

		std::string meshDragType;
		std::string materialDragType;
		if (_app)
		{
			meshDragType = _app->getAssetDragType<Mesh::Source>().value_or("");
			materialDragType = _app->getAssetDragType<Material::Definition>().value_or("");
		}
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
}