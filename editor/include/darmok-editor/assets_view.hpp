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
        virtual std::optional<std::filesystem::path> getSelectedAssetPath() const noexcept = 0;
        virtual void onAssetPathSelected(const std::filesystem::path& assetPath) noexcept = 0;
    };

    class EditorAssetsView final
    {
    public:
        using SceneDefinition = protobuf::Scene;
		using Message = google::protobuf::Message;

        static const std::string& getWindowName() noexcept;

        expected<void, std::string> init(SceneDefinitionWrapper& scene, IEditorAssetsViewDelegate& delegate) noexcept;
        expected<void, std::string> shutdown() noexcept;
        expected<bool, std::string> render() noexcept;
        void focus() noexcept;

        std::optional<std::string> getAssetDragType(uint32_t assetType) const noexcept;
        std::optional<std::string> getAssetTypeName(uint32_t assetType) const noexcept;

        std::filesystem::path addAsset(const Message& msg) noexcept;

    private:
        OptionalRef<SceneDefinitionWrapper> _scene;
        OptionalRef<IEditorAssetsViewDelegate> _delegate;
		std::filesystem::path _currentPath;

        std::unordered_map<uint32_t, std::string> _assetNames;

		template<typename T>
        void addAssetName(std::string name) noexcept
        {
            auto typeId = protobuf::getTypeId<T>();
            _assetNames.emplace(typeId, std::move(name));
        };

        bool drawFolder(const std::filesystem::path& path, bool selected) noexcept;
        bool drawAsset(const google::protobuf::Any& asset, const std::filesystem::path& path, bool selected) noexcept;
    };
    
}