#include "resource.hpp"
#include <CEGUI/DataContainer.h>
#include <darmok/data.hpp>

namespace darmok
{
	CeguiResourceProvider::CeguiResourceProvider(IDataLoader& loader) noexcept
		: _loader(loader)
	{
	}

	void CeguiResourceProvider::setResourceGroupDirectory(const CEGUI::String& resourceGroup, const CEGUI::String& directory) noexcept
	{
		_resourceGroups[resourceGroup] = directory;
	}

	std::string CeguiResourceProvider::getFinalFilename(const CEGUI::String& filename, const CEGUI::String& resourceGroup) const noexcept
	{
		CEGUI::String finalFilename;

		auto itr = _resourceGroups.find(resourceGroup.empty() ? d_defaultResourceGroup : resourceGroup);
		if (itr != _resourceGroups.end())
		{
			finalFilename = itr->second;
		}
		finalFilename += filename;
		return CEGUI::String::convertUtf32ToUtf8(finalFilename.getString());
	}

	void CeguiResourceProvider::loadRawDataContainer(const CEGUI::String& filename, CEGUI::RawDataContainer& output,
		const CEGUI::String& resourceGroup)
	{
		// TODO: not the best because data could be shared
		auto data = _loader(getFinalFilename(filename, resourceGroup));
		output.setData(static_cast<uint8_t*>(data->ptr()));
		output.setSize(data->size());
		data->release();
	}

	void CeguiResourceProvider::unloadRawDataContainer(CEGUI::RawDataContainer& container)
	{
		container.release();
	}

	size_t CeguiResourceProvider::getResourceGroupFileNames(std::vector<CEGUI::String>& outVec,
		const CEGUI::String& filePattern,
		const CEGUI::String& resourceGroup)
	{
		auto finalPattern = getFinalFilename(filePattern, resourceGroup);
		auto matches = _loader.find(finalPattern);
		for (auto& match : matches)
		{
			outVec.push_back(match);
		}
		return matches.size();
	}
}