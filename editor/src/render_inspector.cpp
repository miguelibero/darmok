#include <darmok-editor/render_inspector.hpp>
#include <imgui.h>

namespace darmok::editor
{
	bool RenderableInspectorEditor::render(Renderable& renderable) noexcept
	{
		if (ImGui::CollapsingHeader("Renderable"))
		{
		}

		return true;
	}
}