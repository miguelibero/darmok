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
				LuaUtils::logError("adding model to scene", result);
			}
		}

	private:
		std::weak_ptr<Scene> _scene;
		const sol::protected_function& _callback;
	};

	void LuaModel::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<ModelMesh>("ModelMesh", sol::default_constructor,
			// "vertex_layout", &ModelMesh::vertexLayout,
			"bounding_box", &ModelMesh::boundingBox,
			sol::meta_function::to_string, &ModelMesh::toString
		);
		lua.new_usertype<ModelRenderable>("ModelRenderable", sol::default_constructor,
			"mesh", &ModelRenderable::mesh,
			"material", &ModelRenderable::material,
			sol::meta_function::to_string, &ModelRenderable::toString
		);
		lua.new_usertype<ModelNode>("ModelNode", sol::default_constructor,
			"name", &ModelNode::name,
			"transform", &ModelNode::transform,
			"renderables", &ModelNode::renderables,
			"children", &ModelNode::children,
			"bounding_box", sol::property(&ModelNode::getBoundingBox),
			sol::meta_function::to_string, &ModelNode::toString
		);
		lua.new_usertype<Model>("Model", sol::default_constructor,
			"root_node", &Model::rootNode,
			sol::meta_function::to_string, &Model::toString
		);

		LuaModelSceneConfigurer::bind(lua);
	}

	LuaModelSceneConfigurer::LuaModelSceneConfigurer(const std::shared_ptr<Scene>& scene, AssetContext& assets) noexcept
		: _configurer(*scene, assets)
		, _scene(scene)
	{
	}

	LuaModelSceneConfigurer& LuaModelSceneConfigurer::setParent(const LuaEntity& parent) noexcept
	{
		_configurer.setParent(parent.getReal());
		return *this;
	}

	LuaModelSceneConfigurer& LuaModelSceneConfigurer::setTextureFlags(uint64_t flags) noexcept
	{
		_configurer.setTextureFlags(flags);
		return *this;
	}

	void LuaModelSceneConfigurer::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaModelSceneConfigurer>("ModelSceneConfigurer",
			sol::constructors<LuaModelSceneConfigurer(const std::shared_ptr<Scene>&, AssetContext&)>(),
			"parent", sol::property(&LuaModelSceneConfigurer::setParent),
			"texture_flags", sol::property(&LuaModelSceneConfigurer::setTextureFlags),
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