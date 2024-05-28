#include "model.hpp"
#include "scene.hpp"
#include "asset.hpp"
#include "utils.hpp"
#include "program.hpp"
#include <darmok/scene.hpp>

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

		void operator()(const std::shared_ptr<ModelNode>& node, Entity entity) const noexcept
		{
			auto result = _callback(LuaModelNode(node), LuaEntity(entity, _scene));
			if (!result.valid())
			{
				recoveredLuaError("adding model to scene", result);
			}
		}

	private:
		std::weak_ptr<Scene> _scene;
		const sol::protected_function& _callback;
	};

	LuaModelNodeChildrenCollection::LuaModelNodeChildrenCollection(const std::shared_ptr<ModelNode>& node) noexcept
		: _node(node)
	{
	}

	size_t LuaModelNodeChildrenCollection::size() const noexcept
	{
		return _node->getChildren().size();
	}

	LuaModelNode LuaModelNodeChildrenCollection::operator[](size_t pos) const
	{
		return _node->getChildren()[pos];
	}

	LuaModelNode::LuaModelNode(const std::shared_ptr<ModelNode>& node) noexcept
		: _node(node)
		, _children(node)
	{
	}

	std::shared_ptr<ModelNode> LuaModelNode::getReal() const noexcept
	{
		return _node;
	}

	std::string LuaModelNode::getName() const noexcept
	{
		return std::string(_node->getName());
	}

	glm::mat4 LuaModelNode::getTransform() const noexcept
	{
		return _node->getTransform();
	}

	const LuaModelNodeChildrenCollection& LuaModelNode::getChildren() const noexcept
	{
		return _children;
	}

	void LuaModelNode::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaModelNode>("ModelNode",
			sol::constructors<>(),
			"name", sol::property(&LuaModelNode::getName),
			"transform", sol::property(&LuaModelNode::getTransform),
			"children", sol::property(&LuaModelNode::getChildren)
		);
	}

	LuaModel::LuaModel(const std::shared_ptr<Model>& model) noexcept
		: _model(model)
		, _rootNode(model->getRootNode())
	{
	}

	std::shared_ptr<Model> LuaModel::getReal() const noexcept
	{
		return _model;
	}

	const LuaModelNode& LuaModel::getRootNode() const noexcept
	{
		return _rootNode;
	}

	void LuaModel::bind(sol::state_view& lua) noexcept
	{
		LuaModelNode::bind(lua);
		LuaModelSceneConfigurer::bind(lua);
		lua.new_usertype<LuaModel>("Model",
			sol::constructors<>(),
			"root_node", sol::property(&LuaModel::getRootNode)
		);
	}

	LuaModelSceneConfigurer::LuaModelSceneConfigurer(const LuaScene& scene, const LuaProgram& program, LuaAssets& assets) noexcept
		: _configurer(scene.getReal()->getRegistry(), program.getReal(), assets.getReal())
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
			sol::constructors<LuaModelSceneConfigurer(const LuaScene&, const LuaProgram&, LuaAssets&)>(),
			"parent", sol::property(&LuaModelSceneConfigurer::setParent),
			"run", sol::overload(
				&LuaModelSceneConfigurer::run1,
				&LuaModelSceneConfigurer::run2,
				&LuaModelSceneConfigurer::run3,
				&LuaModelSceneConfigurer::run4
			)
		);
	}

	LuaEntity LuaModelSceneConfigurer::run1(const LuaModel& model) const
	{
		auto entity = _configurer.run(model.getReal());
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaModelSceneConfigurer::run2(const LuaModel& model, sol::protected_function callback) const
	{
		auto entity = _configurer.run(model.getReal(), LuaModelAddToSceneCallback(_scene, callback));
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaModelSceneConfigurer::run3(const LuaModelNode& node) const
	{
		auto entity = _configurer.run(node.getReal());
		return LuaEntity(entity, _scene);
	}

	LuaEntity LuaModelSceneConfigurer::run4(const LuaModelNode& node, sol::protected_function callback) const
	{
		auto entity = _configurer.run(node.getReal(), LuaModelAddToSceneCallback(_scene, callback));
		return LuaEntity(entity, _scene);
	}
}