#include <darmok/prefab.hpp>
#include <darmok/scene.hpp>
#include <darmok/scene_serialize.hpp>
#include <darmok/asset_pack.hpp>
#include <darmok/transform.hpp>

namespace darmok
{
	Prefab::Prefab(std::string scenePath) noexcept
		: _scenePath{ std::move(scenePath) }
	{
	}

	expected<void, std::string> Prefab::load(const Definition& def, IComponentLoadContext& ctxt) noexcept
	{
		auto& scene = ctxt.getScene();
		auto entity = scene.getEntity(*this);

		// destroy children
		if (auto trans = scene.getComponent<Transform>(entity))
		{
			for (auto child : trans->getChildren())
			{
				auto childEntity = scene.getEntity(child.get());
				scene.destroyEntity(childEntity);
			}
		}

		if (def.scene_path().empty())
		{
			return {};
		}
		auto sceneDefResult = ctxt.getAssets().getSceneDefinitionLoader()(def.scene_path());
		if(!sceneDefResult)
		{
			return unexpected<std::string>(fmt::format("loading prefab scene definition: {}", sceneDefResult.error()));
		}

		auto sceneDef = sceneDefResult.value();
		
		SceneLoader loader;
		loader.setAssetPackConfig(AssetPackConfig{
			.fallback = ctxt.getAssets()
		});
		loader.setParent(entity);
		auto loadResult = loader(*sceneDef, scene);
		if (!loadResult)
		{
			return unexpected<std::string>(fmt::format("loading prefab scene: {}", loadResult.error()));
		}
		_scenePath = def.scene_path();

		return {};
	}

	Prefab::Definition Prefab::createDefinition() noexcept
	{
		return {};
	}
}