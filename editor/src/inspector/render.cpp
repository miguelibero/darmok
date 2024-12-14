#include <darmok-editor/inspector/render.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/program.hpp>
#include <darmok/asset.hpp>
#include <darmok/mesh.hpp>

#include <imgui.h>

namespace darmok::editor
{
	void RenderableInspectorEditor::init(AssetContext& assets, EditorProject& proj, ObjectEditorContainer& container)
	{
		_assets = assets;
	}

	void RenderableInspectorEditor::shutdown()
	{
		_assets.reset();
	}

	bool RenderableInspectorEditor::render(Renderable& renderable) noexcept
	{
		if (ImGui::CollapsingHeader("Renderable"))
		{
            auto mat = renderable.getMaterial();
            if(ImguiUtils::drawMaterialReference("Material", mat))
			{
				renderable.setMaterial(mat);
            }
			if (_assets)
			{
				auto mesh = renderable.getMesh();
				if (ImguiUtils::drawMeshReference("Mesh", mesh, _assets->getMeshLoader()))
				{
					renderable.setMesh(mesh);
				}
			}
			auto enabled = renderable.isEnabled();
			if (ImGui::Checkbox("Enabled", &enabled))
			{
				renderable.setEnabled(enabled);
			}
		}
		return true;
	}
}