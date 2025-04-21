#include <darmok/scene_loader.hpp>
#include "scene_loader.hpp"

namespace darmok
{
    SceneImporterImpl::Result  SceneImporterImpl::operator()(Scene& scene, const Definition& def)
    {
		return unexpected<Error>{ "not implemented" };
    }

    SceneImporter::SceneImporter()
        : _impl(std::make_unique<SceneImporterImpl>())
    {
    }

    SceneImporter::~SceneImporter()
    {
        // empty on purpose
    }

    SceneImporter::Result SceneImporter::operator()(Scene& scene, const Definition& def)
    {
		return (*_impl)(scene, def);
    }

    SceneLoader::SceneLoader(ISceneDefinitionLoader& defLoader)
		: _defLoader(defLoader)
	{
	}

    SceneLoader::Result SceneLoader::operator()(Scene& scene, std::filesystem::path path)
    {
		auto defResult = _defLoader(path);
        if (!defResult)
        {
			return unexpected<Error>{ defResult.error() };
        }
		SceneImporter importer;
		return importer(scene, **defResult);
    }
}