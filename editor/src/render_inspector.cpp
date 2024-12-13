#include <darmok-editor/render_inspector.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/program.hpp>

#include <imgui.h>

namespace darmok::editor
{
	bool RenderableInspectorEditor::render(Renderable& renderable) noexcept
	{
		if (ImGui::CollapsingHeader("Renderable"))
		{
            auto mat = renderable.getMaterial();
            if(ImguiAssetReference("Material", mat, "MATERIAL"))
			{
				renderable.setMaterial(mat);
            }
		}
		return true;
	}
}