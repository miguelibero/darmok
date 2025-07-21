#pragma once

#include <darmok/optional_ref.hpp>
#include <darmok/string.hpp>
#include <darmok/protobuf.hpp>
#include <darmok/protobuf/scene.pb.h>

#include <bx/bx.h>

#include <string>
#include <filesystem>
#include <unordered_map>

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
    };

    class EditorAssetsView final
    {
    public:
        using SceneDefinition = protobuf::Scene;
		using Message = google::protobuf::Message;

        static const std::string& getWindowName();

        void init(SceneDefinitionWrapper& scene, IEditorAssetsViewDelegate& delegate);
        void shutdown();
        void focus();
        bool render();

        std::optional<std::string> getAssetDragType(uint32_t assetType) const noexcept;
        std::optional<std::string> getAssetTypeName(uint32_t assetType) const noexcept;

		template<typename T>
        std::filesystem::path addAsset()
        {
            return addAsset(protobuf::getTypeId<T>());
        }

    private:
        OptionalRef<SceneDefinitionWrapper> _scene;
        OptionalRef<IEditorAssetsViewDelegate> _delegate;
		std::filesystem::path _currentPath;

        struct EditorAssetConfig final
        {
            std::string name;
            std::unique_ptr<Message> prototype;
        };
        std::unordered_map<uint32_t, EditorAssetConfig> _assetTypes;

		template<typename T>
        void addAssetType(const char* name, T&& msg)
        {
            auto typeId = protobuf::getTypeId<T>();
            _assetTypes.emplace(typeId, EditorAssetConfig{ name, std::make_unique<T>(std::move(msg)) });
        };

        std::filesystem::path addAsset(uint32_t assetType);
        bool drawFolder(const std::filesystem::path& path, bool selected);
        bool drawAsset(const google::protobuf::Any& asset, const std::filesystem::path& path, bool selected);
    };
    
}