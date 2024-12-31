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

	bool RenderableInspectorEditor::render(Renderable& renderable) noexcept
	{
		if (ImGui::CollapsingHeader("Renderable"))
		{
            auto mat = renderable.getMaterial();
            if(_app->drawMaterialReference("Material", mat))
			{
				renderable.setMaterial(mat);
            }
			if (_app)
			{
				auto mesh = renderable.getMesh();
				if (_app->drawMeshReference("Mesh", mesh))
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