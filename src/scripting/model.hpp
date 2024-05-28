#pragma once

#include <memory>
#include <string>
#include <darmok/optional_ref.hpp>
#include <darmok/collection.hpp>
#include <darmok/model.hpp>
#include <sol/sol.hpp>

namespace darmok
{
	class LuaEntity;
	class LuaScene;
	class LuaAssets;
	class LuaModelNode;
	class LuaProgram;
	class Scene;

	class LuaModelNodeChildrenCollection final : public ValCollection<LuaModelNode>
	{
	public:
		LuaModelNodeChildrenCollection(const std::shared_ptr<ModelNode>& node) noexcept;

		[[nodiscard]] size_t size() const noexcept override;
		[[nodiscard]] LuaModelNode operator[](size_t pos) const override;
	private:
		std::shared_ptr<ModelNode> _node;
	};

	class LuaModelNode final
	{
	public:
		LuaModelNode(const std::shared_ptr<ModelNode>& node) noexcept;

		std::shared_ptr<ModelNode> getReal() const noexcept;
		std::string getName() const noexcept;

        glm::mat4 getTransform() const noexcept;
        const LuaModelNodeChildrenCollection& getChildren() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<ModelNode> _node;
		LuaModelNodeChildrenCollection _children;
	};
	  
	class LuaModel final
	{
	public:
		LuaModel(const std::shared_ptr<Model>& model) noexcept;

		std::shared_ptr<Model> getReal() const noexcept;
        const LuaModelNode& getRootNode() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<Model> _model;
		LuaModelNode _rootNode;
	};

	class LuaModelSceneConfigurer final
	{
	public:
		LuaModelSceneConfigurer(const LuaScene& scene, const LuaProgram& program, LuaAssets& assets) noexcept;
		LuaModelSceneConfigurer& setParent(const LuaEntity& parent) noexcept;

		LuaEntity run1(const LuaModel& model) const;
		LuaEntity run2(const LuaModel& model, sol::protected_function callback) const;
		LuaEntity run3(const LuaModelNode& node) const;
		LuaEntity run4(const LuaModelNode& node, sol::protected_function callback) const;

		static void bind(sol::state_view& lua) noexcept;
	private:
		ModelSceneConfigurer _configurer;
		std::shared_ptr<Scene> _scene;
	};

}