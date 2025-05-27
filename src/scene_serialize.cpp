#include <darmok/scene_serialize.hpp>
#include <darmok/scene.hpp>
#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/render_scene.hpp>
#include "scene_serialize.hpp"

namespace darmok
{
    SceneArchive::SceneArchive(const protobuf::Scene& sceneDef, Scene& scene)
        : _sceneDef{ sceneDef }
        , _scene{ scene }
		, _assetPack{ sceneDef.assets() }
        , _count{ 0 }
        , _type{ 0 }
		, _entityOffset{ 0 }
    {
        auto view = scene.getRegistry().view<Entity>();
        if (view.begin() != view.end())
        {
            auto max = *std::max_element(view.begin(), view.end());
            _entityOffset = static_cast<entt::id_type>(max) + 1;
        }
    }

    entt::continuous_loader SceneArchive::createLoader()
    {
		return { _scene.getRegistry() };
    }

    void SceneArchive::operator()(std::underlying_type_t<Entity>& count)
    {
        if (_type == 0)
        {
            count = _sceneDef.registry().entities();
        }
        else
        {
            auto& typeComps = _sceneDef.registry().components();
            auto itr = typeComps.find(_type);
            if (itr != typeComps.end())
            {
				count = itr->second.components_size();
            }
        }
		_error.reset();
        _count = 0;
    }

    AssetPack& SceneArchive::getAssets()
    {
		return _assetPack;
    }

    Entity SceneArchive::getEntity(uint32_t id) const
    {
		return static_cast<Entity>(id + _entityOffset);
    }

    const Scene& SceneArchive::getScene() const
    {
        return _scene;
    }

    Scene& SceneArchive::getScene()
    {
        return _scene;
    }

    expected<Entity, std::string> SceneArchive::finishLoad()
    {
        expected<void, std::string> result;
        for (auto& func : _loadFunctions)
        {
            auto funcResult = func();
            if (!funcResult)
            {
                result = std::move(funcResult);
			}
        }
        _loadFunctions.clear();
        if (!result)
        {
			return unexpected{ result.error() };
        }

        for(uint32_t id = 0; id < _sceneDef.registry().entities(); ++id)
        {
            auto entity = getEntity(id);
            if (auto trans = _scene.getComponent<Transform>(entity))
            {
                if (!trans->getParent())
                {
                    return entity;
                }
            }
		}
        return unexpected{ "could not find root entity" };
    }

    SceneImporterImpl::Result SceneImporterImpl::operator()(Scene& scene, const Definition& def)
    {
		SceneArchive archive{ def, scene };
		auto loader = archive.createLoader();

        loader.get<entt::entity>(archive);
        auto result = archive.loadComponents(loader);
        if (!result)
        {
            return unexpected{ result.error() };
        }
        loader.orphans();
        return archive.finishLoad();
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