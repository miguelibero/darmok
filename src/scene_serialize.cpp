#include <darmok/scene_serialize.hpp>
#include <darmok/scene.hpp>
#include "scene_serialize.hpp"

#include <darmok/camera.hpp>
#include <darmok/transform.hpp>
#include <darmok/render_scene.hpp>
#include <darmok/skeleton.hpp>
#include <darmok/light.hpp>

namespace darmok
{
    SceneArchive::SceneArchive(const protobuf::Scene& sceneDef, Scene& scene, const AssetPackFallbacks& assetPackFallbacks)
        : _sceneDef{ sceneDef }
        , _scene{ scene }
		, _assetPack{ sceneDef.assets(), assetPackFallbacks }
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
        for (auto& func : _compLoadFunctions)
        {
            auto funcResult = func();
            if (!funcResult)
            {
                result = std::move(funcResult);
			}
        }
        _compLoadFunctions.clear();
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

    SceneImporterImpl::SceneImporterImpl(const AssetPackFallbacks& assetPackFallbacks)
        : _assetPackFallbacks{ assetPackFallbacks }
    {
	}

    SceneImporterImpl::Result SceneImporterImpl::operator()(Scene& scene, const Definition& def)
    {
		SceneArchive archive{ def, scene, _assetPackFallbacks };

		archive.registerComponent<Transform>();
        archive.registerComponent<Renderable>();
        archive.registerComponent<Camera>();
        archive.registerComponent<Skinnable>();
        archive.registerComponent<PointLight>();
        archive.registerComponent<DirectionalLight>();
        archive.registerComponent<SpotLight>();
        archive.registerComponent<AmbientLight>();

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

    SceneImporter::SceneImporter(const AssetPackFallbacks& assetPackFallbacks)
        : _impl(std::make_unique<SceneImporterImpl>(assetPackFallbacks))
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

    SceneLoader::SceneLoader(ISceneDefinitionLoader& defLoader, const AssetPackFallbacks& assetPackFallbacks)
        : _defLoader{ defLoader }
        , _importer{ assetPackFallbacks }
	{
	}

    SceneLoader::Result SceneLoader::operator()(Scene& scene, std::filesystem::path path)
    {
		auto defResult = _defLoader(path);
        if (!defResult)
        {
			return unexpected<Error>{ defResult.error() };
        }
		return _importer(scene, **defResult);
    }
}