#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <bx/bx.h>

#include <string>

namespace darmok
{
    class SceneDefinitionWrapper;
}

namespace darmok::editor
{
    class BX_NO_VTABLE IEditorAssetsViewDelegate
    {
    public:
        ~IEditorAssetsViewDelegate() = default;
        virtual std::optional<std::string> getSelectedAssetPath(uint32_t assetType) const = 0;
        virtual void onAssetPathSelected(uint32_t assetType, const std::string& assetPath) = 0;
    };

    class EditorAssetsView final
    {
    public:
        using SceneDefinition = protobuf::Scene;

        EditorAssetsView(std::string_view name, std::string_view dragType, uint32_t assetType);

        const std::string& getName() const;
        void init(SceneDefinitionWrapper& scene, IEditorAssetsViewDelegate& delegate);
        void shutdown();
        void focus();
        bool render();
    private:
        const uint32_t _assetType;
        const std::string _name;
        const std::string _dragType;
        OptionalRef<SceneDefinitionWrapper> _scene;
        OptionalRef<IEditorAssetsViewDelegate> _delegate;

        bool drawAsset(const google::protobuf::Any& asset, const std::string& path, bool selected);
    };
    
}