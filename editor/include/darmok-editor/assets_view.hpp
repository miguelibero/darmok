#pragma once

#include <darmok-editor/utils.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/data.hpp>

#include <vector>
#include <type_traits>
#include <string>
#include <optional>


#include <bx/bx.h>
#include <imgui.h>

namespace darmok::editor
{
    template<typename T>
    class BX_NO_VTABLE IEditorAssetsViewDelegate
    {
    public:
        virtual ~IEditorAssetsViewDelegate() = default;
        virtual std::optional<T> getSelectedAsset(std::type_identity<T>) const = 0;
        virtual std::string getAssetName(const T& asset) const = 0;
        virtual std::vector<T> getAssets(std::type_identity<T>) const = 0;
        virtual void onAssetSelected(const T& asset) = 0;
        virtual void addAsset(std::type_identity<T>) = 0;

        virtual DataView getAssetDropPayload(const T& asset)
        {
            return DataView::fromStatic(asset);
        }
    };

    template<typename T>
    class EditorAssetsView final
    {
    public:
        using Delegate = IEditorAssetsViewDelegate<T>;

        EditorAssetsView(const char* name, const char* dragType = nullptr)
            : _name(name)
            , _dragType(dragType)
        {
        }

        const char* getName() const noexcept
        {
            return _name;
        }

        void init(Delegate& dlg) noexcept
        {
            _delegate = dlg;
        }

        void shutdown() noexcept
        {
            _delegate.reset();
        }

        void render() noexcept
        {
            static const ImVec2 cellPadding(10.0f, 10.0f);

            if(!_delegate)
            {
                return;
            }
            if (ImGui::Begin(_name))
            {
                auto winSize = ImGui::GetWindowSize();
                float colWidth = ImguiUtils::getAssetSize().x + (2 * cellPadding.x);
                int cols = winSize.x / colWidth;
                if (cols > 0)
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, cellPadding);
                    ImGui::BeginTable(_name, cols);
                    auto selectedAsset = getSelectedAsset();
                    for (auto asset : _delegate->getAssets(_typeTag))
                    {
                        ImGui::TableNextColumn();
                        drawAsset(asset, selectedAsset);
                    }
                    ImGui::TableNextColumn();
                    if (ImguiUtils::drawAsset("+"))
                    {
                        addAsset();
                    }
                    ImGui::EndTable();
                    ImGui::PopStyleVar();
                }
            }
            ImGui::End();
        }
    private:
        static const std::type_identity<T> _typeTag;
        const char* _name;
        const char* _dragType;
        OptionalRef<Delegate> _delegate;

        bool drawAsset(const T& asset, const std::optional<T>& selectedAsset)
        {
            auto selected = asset == selectedAsset;
            if (!_delegate)
            {
                return selected;
            }
            auto name = _delegate->getAssetName(asset);
            if (ImguiUtils::drawAsset(name.c_str(), selected))
            {
                selected = !selected;
            }
            if (_dragType != nullptr && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
            {
                auto data = _delegate->getAssetDropPayload(asset);
                if (!data.empty())
                {
                    auto accepted = ImGui::SetDragDropPayload(_dragType, data.ptr(), data.size());
                    ImguiUtils::drawAsset(name.c_str(), accepted);
                }
                ImGui::EndDragDropSource();
            }
            if (selected)
            {
                onAssetSelected(asset);
            }
            return selected;
        }

        void addAsset()
        {
            if (_delegate)
            {
                _delegate->addAsset(_typeTag);
            }
        }

        std::optional<T> getSelectedAsset() const
        {
            if(_delegate)
            {
                return _delegate->getSelectedAsset(_typeTag);
            }
            return std::nullopt;
        }

        void onAssetSelected(const T& asset)
        {
            if(_delegate)
            {
                _delegate->onAssetSelected(asset);
            }
        }
    };

    template<typename T>
    const std::type_identity<T> EditorAssetsView<T>::_typeTag;
}