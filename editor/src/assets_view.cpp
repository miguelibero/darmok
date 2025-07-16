#include <darmok-editor/assets_view.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include <darmok/material.hpp>

#include <imgui.h>
#include <fmt/format.h>

namespace darmok::editor
{
    void EditorAssetsView::init(SceneDefinitionWrapper& scene, IEditorAssetsViewDelegate& delegate)
    {
        _scene = scene;
        _delegate = delegate;
    }

    const std::string& EditorAssetsView::getTitle()
    {
		static const std::string title = "Assets";
        return title;
    }

    void EditorAssetsView::shutdown()
    {
        _delegate.reset();
    }

    void EditorAssetsView::focus()
    {
        ImGui::SetWindowFocus(getTitle().c_str());
    }

    bool EditorAssetsView::render()
    {
        static const ImVec2 cellPadding{ 10.0f, 10.0f };
        if (!_scene)
        {
            return false;
        }
        bool changed = false;
        if (ImGui::Begin(getTitle().c_str()))
        {
            auto winSize = ImGui::GetWindowSize();
            float colWidth = ImguiUtils::getAssetSize().x + (2 * cellPadding.x);
            int cols = winSize.x / colWidth;
            if (cols > 0)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
                ImGui::BeginTable(getTitle().c_str(), cols);
                std::optional<std::filesystem::path> selectedPath;
                if (_delegate)
                {
                    selectedPath = _delegate->getSelectedAssetPath();
                }
                int i = 0;
                for (auto [path, asset] : _scene->getAssets())
                {
                    ImGui::PushID(i);
                    ImGui::TableNextColumn();
                    if (drawAsset(*asset, path, selectedPath == path))
                    {
                        changed = true;
                    }
                    ImGui::PopID();
                    ++i;
                }
                ImGui::TableNextColumn();
                ImGui::EndTable();
                ImGui::PopStyleVar();
            }
        }
        ImGui::End();
        return changed;
    }

    bool EditorAssetsView::drawAsset(const google::protobuf::Any& asset, const std::filesystem::path& path, bool selected)
    {
        std::string name = path.filename().string();
        auto selectionChanged = false;
        if (ImguiUtils::drawAsset(name.c_str(), selected))
        {
            selected = !selected;
            selectionChanged = true;
        }
		auto dragType = _delegate->getAssetDragType(protobuf::getTypeId(asset));
        if (dragType && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            if (!path.empty())
            {
				auto pathStr = path.string();
                auto accepted = ImGui::SetDragDropPayload(dragType->c_str(), pathStr.c_str(), sizeof(std::string::value_type) * pathStr.size());
                ImguiUtils::drawAsset(name.c_str(), accepted);
            }
            ImGui::EndDragDropSource();
        }
        if (selectionChanged && selected)
        {
			_delegate->onAssetPathSelected(path);
        }
        return selected;
    }
}