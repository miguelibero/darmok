#include <darmok/scene_serialize.hpp>
#include <darmok/scene.hpp>
#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_scene.hpp>
#include "scene_serialize.hpp"

namespace darmok
{
    SceneArchive::SceneArchive(const protobuf::Scene& scene)
        : _scene{ scene }
		, _assetPack{ _scene.assets() }
        , _count{ 0 }
        , _type{ 0 }
    {
    }

    void SceneArchive::operator()(std::underlying_type_t<Entity>& count)
    {
        if (_type == 0)
        {
            count = _scene.registry().entities();
        }
        else
        {
            auto& typeComps = _scene.registry().components();
            auto itr = typeComps.find(_type);
            if (itr != typeComps.end())
            {
				count = itr->second.components_size();
            }
        }
		_error.reset();
        _count = 0;
    }

    SceneImporterImpl::Result SceneImporterImpl::operator()(Scene& scene, const Definition& def)
    {
        auto& registry = scene.getRegistry();
        entt::continuous_loader loader{ registry };
		SceneArchive archive{ def };

        loader.get<entt::entity>(archive);

        {
            auto result = archive.load<Transform>(loader);
            if (!result)
            {
                return unexpected{ result.error() };
            }
        }
        {
            auto result = archive.load<Renderable>(loader);
            if (!result)
            {
                return unexpected{ result.error() };
            }
        }

        loader.orphans();

        return {};
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