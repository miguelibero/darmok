#pragma once

#include <CEGUI/ResourceProvider.h>
#include <unordered_map>
#include <string>

namespace darmok
{
    class IDataLoader;

    class CeguiResourceProvider final : public CEGUI::ResourceProvider
    {
    public:
        CeguiResourceProvider(IDataLoader& loader) noexcept;

        void setResourceGroupDirectory(const CEGUI::String& resourceGroup, const CEGUI::String& directory) noexcept;

        void loadRawDataContainer(const CEGUI::String& filename, CEGUI::RawDataContainer& output, const CEGUI::String& resourceGroup) override;
        void unloadRawDataContainer(CEGUI::RawDataContainer& container) override;

        size_t getResourceGroupFileNames(std::vector<CEGUI::String>& outVec,
            const CEGUI::String& filePattern,
            const CEGUI::String& resourceGroup) override;

    private:
        IDataLoader& _loader;
        std::unordered_map<CEGUI::String, CEGUI::String> _resourceGroups;

        std::string getFinalFilename(const CEGUI::String& filename, const CEGUI::String& resourceGroup) const noexcept;
    };
}