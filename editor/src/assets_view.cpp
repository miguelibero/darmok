#include <darmok-editor/assets_view.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/scene_serialize.hpp>

#include <imgui.h>
#include <fmt/format.h>

namespace darmok::editor
{
    EditorAssetsView::EditorAssetsView(std::string_view title, std::string_view assetName, const Message& prototype)
        : _title{ title }
        , _assetName{ assetName }
        , _assetType{ protobuf::getTypeId(prototype)}
    {
        _prototype.PackFrom(prototype);
    }

    const std::string& EditorAssetsView::getTitle() const
    {
        return _title;
    }

    uint32_t EditorAssetsView::getAssetType() const
    {
		return _assetType;
    }

    const std::string& EditorAssetsView::getDragType() const
    {
        return _assetName;
    }

    void EditorAssetsView::init(SceneDefinitionWrapper& scene, IEditorAssetsViewDelegate& delegate)
    {
        _scene = scene;
        _delegate = delegate;
    }

    void EditorAssetsView::shutdown()
    {
        _delegate.reset();
    }

    void EditorAssetsView::focus()
    {
        ImGui::SetWindowFocus(_title.c_str());
    }

    bool EditorAssetsView::render()
    {
        static const ImVec2 cellPadding{ 10.0f, 10.0f };
        if (!_scene)
        {
            return false;
        }
        bool changed = false;
        if (ImGui::Begin(_title.c_str()))
        {
            auto winSize = ImGui::GetWindowSize();
            float colWidth = ImguiUtils::getAssetSize().x + (2 * cellPadding.x);
            int cols = winSize.x / colWidth;
            if (cols > 0)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
                ImGui::BeginTable(_title.c_str(), cols);
                std::optional<std::string> selectedPath;
                if (_delegate)
                {
                    selectedPath = _delegate->getSelectedAssetPath(_assetType);
                }
                int i = 0;
                for (auto [path, asset] : _scene->getAssets(_assetType))
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
                if (ImguiUtils::drawAsset("+"))
                {
                    auto newAssetPathPrefix = fmt::format("new_{}", StringUtils::toLower(_assetName));
					_scene->addAsset(newAssetPathPrefix, _prototype);
                }
                ImGui::EndTable();
                ImGui::PopStyleVar();
            }
        }
        ImGui::End();
        return changed;
    }

    bool EditorAssetsView::drawAsset(const google::protobuf::Any& asset, const std::string& path, bool selected)
    {
        std::string name = path;
        auto selectionChanged = false;
        if (ImguiUtils::drawAsset(name.c_str(), selected))
        {
            selected = !selected;
            selectionChanged = true;
        }
		auto& dragType = getDragType();
        if (!dragType.empty() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            if (!path.empty())
            {
                auto accepted = ImGui::SetDragDropPayload(dragType.c_str(), path.c_str(), sizeof(std::string::value_type) * path.size());
                ImguiUtils::drawAsset(name.c_str(), accepted);
            }
            ImGui::EndDragDropSource();
        }
        if (selectionChanged && selected)
        {
			_delegate->onAssetPathSelected(_assetType, path);
        }
        return selected;
    }
}