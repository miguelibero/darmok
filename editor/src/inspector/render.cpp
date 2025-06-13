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

	bool RenderableInspectorEditor::renderType(Renderable::Definition& renderable) noexcept
	{
		auto changed = false;
		if (ImGui::CollapsingHeader("Renderable"))
		{
		}
		return changed;
	}
}