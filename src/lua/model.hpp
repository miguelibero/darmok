#pragma once

#include "lua.hpp"

#include <darmok/optional_ref.hpp>
#include <darmok/collection.hpp>
#include <darmok/model.hpp>

#include <string>

namespace darmok
{
	class LuaEntity;
	class AssetContext;
	class LuaModelNode;
	class Scene;
	struct Model;
	struct ModelNode;

	struct LuaModel final
	{
		static void bind(sol::state_view& lua) noexcept;
	};

	class LuaModelSceneConfigurer final
	{
	public:
		LuaModelSceneConfigurer(const std::shared_ptr<Scene>& scene, AssetContext& assets) noexcept;
		LuaModelSceneConfigurer& setParent(const LuaEntity& parent) noexcept;
		LuaModelSceneConfigurer& setTextureFlags(uint64_t flags) noexcept;

		LuaEntity run1(const Model& model);
		LuaEntity run2(const Model& model, sol::protected_function callback);
		LuaEntity run3(const ModelNode& node);
		LuaEntity run4(const ModelNode& node, sol::protected_function callback);

		static void bind(sol::state_view& lua) noexcept;
	private:
		ModelSceneConfigurer _configurer;
		std::shared_ptr<Scene> _scene;
	};

}