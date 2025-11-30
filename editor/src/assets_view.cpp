#include <darmok-editor/assets_view.hpp>
#include <darmok-editor/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/program.hpp>
#include <darmok/mesh.hpp>
#include <darmok/texture.hpp>
#include <darmok/material.hpp>
#include <darmok/skeleton.hpp>

#include <imgui.h>
#include <fmt/format.h>

namespace darmok::editor
{
    expected<void, std::string> EditorAssetsView::init(SceneDefinitionWrapper& scene, IEditorAssetsViewDelegate& delegate) noexcept
    {
        _scene = scene;
        _delegate = delegate;

        addAssetName<Program::Source>("program");
        addAssetName<Texture::Source>("texture");
        addAssetName<Mesh::Source>("mesh");
        addAssetName<Material::Definition>("material");
        addAssetName<Armature::Definition>("armature");
        return {};
    }

    const std::string& EditorAssetsView::getWindowName() noexcept
    {
		static const std::string name = "###Assets";
        return name;
    }

    expected<void, std::string> EditorAssetsView::shutdown() noexcept
    {
        _delegate.reset();
        _assetNames.clear();
        return {};
    }

    void EditorAssetsView::focus() noexcept
    {
        ImGui::SetWindowFocus(getWindowName().c_str());
    }

    expected<bool, std::string> EditorAssetsView::render() noexcept
    {
        static const ImVec2 cellPadding{ 10.0f, 10.0f };
        if (!_scene)
        {
            return unexpected<std::string>{ "scene not loaded" };
        }
        bool changed = false;

        std::string title{ "Assets" };
        if (!_currentPath.empty())
        {
            title += " - " + _currentPath.string();
        }

        title += getWindowName();

        std::optional<std::filesystem::path> selectedPath;
        if (_delegate)
        {
            selectedPath = _delegate->getSelectedAssetPath();
        }

        int i = 0;
        auto drawItem = [&i, this, &selectedPath](const std::filesystem::path& path, OptionalRef<SceneDefinitionWrapper::Any> asset)
        {
            ImGui::PushID(i);
            ImGui::TableNextColumn();
            if (!asset)
            {
                drawFolder(path, selectedPath == path);
            }
            else
            {
                drawAsset(*asset, path, selectedPath == path);
            }
            ImGui::PopID();
            ++i;
        };

        if (ImGui::Begin(title.c_str()))
        {
            auto winSize = ImGui::GetWindowSize();
            float colWidth = ImguiUtils::getAssetSize().x + (2 * cellPadding.x);
            int cols = winSize.x / colWidth;
            if (cols > 0)
            {
                ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
                ImGui::BeginTable("main", cols);
                auto assets = _scene->getAssets(_currentPath);
                if (!_currentPath.empty())
                {
                    drawItem("..", nullptr);
                }
                for (auto [path, asset] : assets)
                {
                    drawItem(path, asset);
                }
                ImGui::TableNextColumn();
                ImGui::EndTable();
                ImGui::PopStyleVar();
            }
        }
        ImGui::End();
        return changed;
    }

    bool EditorAssetsView::drawFolder(const std::filesystem::path& path, bool selected) noexcept
    {
        std::string name = path.filename().string();
        auto selectionChanged = false;
        if (ImguiUtils::drawAsset(name.c_str(), selected))
        {
            selected = !selected;
            selectionChanged = true;
        }
        if (selectionChanged && selected)
        {
            _delegate->onAssetPathSelected(path);
        }
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            _currentPath = (_currentPath / path).lexically_normal();
            if(_currentPath == ".")
            {
                _currentPath.clear();
			}
        }
        return selected;
    }

    bool EditorAssetsView::drawAsset(const google::protobuf::Any& asset, const std::filesystem::path& path, bool selected) noexcept
    {
        std::string name = path.filename().string();
        auto selectionChanged = false;
        if (ImguiUtils::drawAsset(name.c_str(), selected))
        {
            selected = !selected;
            selectionChanged = true;
        }
		auto dragType = getAssetDragType(protobuf::getTypeId(asset));
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

    std::optional<std::string> EditorAssetsView::getAssetTypeName(uint32_t assetType) const noexcept
    {
        auto itr = _assetNames.find(assetType);
        if (itr != _assetNames.end())
        {
            return itr->second;
        }
        return std::nullopt;
    }

    std::filesystem::path EditorAssetsView::addAsset(const Message& msg) noexcept
    {
        auto path = _currentPath;
		if (auto name = getAssetTypeName(protobuf::getTypeId(msg)))
        {
            path = path / *name;
        }
        return _scene->addAsset(path.lexically_normal(), msg);

    }

    std::optional<std::string> EditorAssetsView::getAssetDragType(uint32_t assetType) const noexcept
    {
        if (auto name = getAssetTypeName(assetType))
        {
            return StringUtils::toUpper(*name);
        }
        return std::nullopt;
    }
}