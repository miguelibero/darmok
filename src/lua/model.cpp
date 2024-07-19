#include "model.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include <darmok/scene.hpp>
#include <darmok/program.hpp>
#include <darmok/asset.hpp>

namespace darmok
{
	class LuaModelAddToSceneCallback final
	{
	public:
		LuaModelAddToSceneCallback(const std::weak_ptr<Scene>& scene, const sol::protected_function& callback)
			: _scene(scene)
			, _callback(callback)
		{
		}

		void operator()(const ModelNode& node, Entity entity) const noexcept
		{
			auto result = _callback(node, LuaEntity(entity, _scene));
			if (!result.valid())
			{
				logLuaError("adding model to scene", result);
			}
		}

	private:
		std::weak_ptr<Scene> _scene;
		const sol::protected_function& _callback;
	};

	void LuaModel::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<ModelNode>("ModelNode", sol::no_constructor,
			"name", &ModelNode::name,
			"transform", &ModelNode::transform,
			"children", &ModelNode::children
		);
		lua.new_usertype<Model>("Model", sol::no_constructor,
			"root_node", &Model::rootNode
		);

		LuaModelSceneConfigurer::bind(lua);
	}

	LuaModelSceneConfigurer::LuaModelSceneConfigurer(const LuaScene& scene, AssetContext& assets) noexcept
		: _configurer(*scene.getReal(), assets)
		, _scene(scene.getReal())
	{
	}

	LuaModelSceneConfigurer& LuaModelSceneConfigurer::setParent(const LuaEntity& parent) noexcept
	{
		_configurer.setParent(parent.getReal());
		return *this;
	}

	void LuaModelSceneConfigurer::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaModelSceneConfigurer>("ModelSceneConfigurer",
			sol::constructors<LuaModelSceneConfigurer(const LuaScene&, AssetContext&)>(),
			"parent", sol::property(&LuaModelSceneConfigurer::setParent),
			sol::meta_function::call, sol::overload(
				&LuaModelSceneConfigurer::run1,
				&LuaModelSceneConfigurer::run2,
				&LuaModelSceneConfigurer::run3,
				&LuaModelSceneConfigurer::run4
			)
		);
	}

	LuaEntity LuaModelSceneConfigurer::run1(const Model& model)
	{
		auto entity = _configurer(model);
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaModelSceneConfigurer::run2(const Model& model, sol::protected_function callback)
	{
		auto entity = _configurer(model, LuaModelAddToSceneCallback(_scene, callback));
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaModelSceneConfigurer::run3(const ModelNode& node)
	{
		auto entity = _configurer(node);
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaModelSceneConfigurer::run4(const ModelNode& node, sol::protected_function callback)
	{
		auto entity = _configurer(node, LuaModelAddToSceneCallback(_scene, callback));
		return LuaEntity(entity, _scene);
	}
}