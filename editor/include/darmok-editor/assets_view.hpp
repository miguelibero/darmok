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
        virtual std::optional<std::filesystem::path> getSelectedAssetPath() const = 0;
        virtual void onAssetPathSelected(const std::filesystem::path& assetPath) = 0;
        virtual std::optional<std::string> getAssetDragType(uint32_t typeId) const = 0;
    };

    class EditorAssetsView final
    {
    public:
        using SceneDefinition = protobuf::Scene;
		using Message = google::protobuf::Message;

        static const std::string& getTitle();

        void init(SceneDefinitionWrapper& scene, IEditorAssetsViewDelegate& delegate);
        void shutdown();
        void focus();
        bool render();
    private:
        OptionalRef<SceneDefinitionWrapper> _scene;
        OptionalRef<IEditorAssetsViewDelegate> _delegate;
		std::map<uint32_t, std::string> _assetDragTypes;

        bool drawAsset(const google::protobuf::Any& asset, const std::filesystem::path& path, bool selected);
    };
    
}