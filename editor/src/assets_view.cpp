#include <darmok-editor/assets_view.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/scene_serialize.hpp>

#include <imgui.h>

namespace darmok::editor
{
    EditorAssetsView::EditorAssetsView(std::string_view name, std::string_view dragType, uint32_t assetType)
        : _name{ name }
        , _dragType{ dragType }
        , _assetType{ assetType }
    {
    }

    const std::string& EditorAssetsView::getName() const
    {
        return _name;
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
        ImGui::SetWindowFocus(_name.c_str());
    }

    void EditorAssetsView::render()
    {
        static const ImVec2 cellPadding{ 10.0f, 10.0f };
        if (!_scene)
        {
            return;
        }
        if (ImGui::Begin(_name.c_str()))
        {
            auto winSize = ImGui::GetWindowSize();
            float colWidth = ImguiUtils::getAssetSize().x + (2 * cellPadding.x);
            int cols = winSize.x / colWidth;
            if (cols > 0)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
                ImGui::BeginTable(_name.c_str(), cols);
                std::optional<std::string> selectedPath;
                if (_delegate)
                {
                    _delegate->getSelectedAssetPath(_assetType);
                }
                int i = 0;
                for (auto [path, asset] : _scene->getAssets(_assetType))
                {
                    ImGui::PushID(i);
                    ImGui::TableNextColumn();
                    drawAsset(*asset, path, selectedPath == path);
                    ImGui::PopID();
                    ++i;
                }
                ImGui::TableNextColumn();
                if (ImguiUtils::drawAsset("+"))
                {
                    // addAsset();
                }
                ImGui::EndTable();
                ImGui::PopStyleVar();
            }
        }
        ImGui::End();
    }

    bool EditorAssetsView::drawAsset(const google::protobuf::Any& asset, const std::string& path, bool selected)
    {
        std::string name = path;
        if (!_delegate)
        {
            name = _delegate->getAssetName(_assetType, asset);
        }
        auto selectionChanged = false;
        if (ImguiUtils::drawAsset(name.c_str(), selected))
        {
            selected = !selected;
            selectionChanged = true;
        }
        if (!_dragType.empty() && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            if (!path.empty())
            {
                auto accepted = ImGui::SetDragDropPayload(_dragType.c_str(), path.c_str(), path.size());
                ImguiUtils::drawAsset(name.c_str(), accepted);
            }
            ImGui::EndDragDropSource();
        }
        if (selectionChanged && selected)
        {
            // onAssetSelected(asset);
        }
        return selected;
    }
}