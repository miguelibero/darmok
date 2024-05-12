#pragma once

#include <memory>
#include <string>
#include <darmok/optional_ref.hpp>
#include <darmok/collection.hpp>
#include <darmok/model.hpp>
#include <bgfx/bgfx.h>
#include <sol/sol.hpp>

namespace darmok
{
	class LuaEntity;
	class LuaScene;
	class LuaAssets;
	class LuaModelNode;
	class Scene;

	class LuaModelNodeChildrenCollection final : public ValCollection<LuaModelNode>
	{
	public:
		LuaModelNodeChildrenCollection(const std::shared_ptr<IModelNode>& node) noexcept;

		[[nodiscard]] size_t size() const noexcept override;
		[[nodiscard]] LuaModelNode operator[](size_t pos) const override;
	private:
		std::shared_ptr<IModelNode> _node;
	};

	class LuaModelNode final
	{
	public:
		LuaModelNode(const std::shared_ptr<IModelNode>& node) noexcept;

		std::shared_ptr<IModelNode> getReal() const noexcept;
		std::string getName() const noexcept;

        glm::mat4 getTransform() const noexcept;
        const LuaModelNodeChildrenCollection& getChildren() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<IModelNode> _node;
		LuaModelNodeChildrenCollection _children;
	};
	  
	class LuaModel final
	{
	public:
		LuaModel(const std::shared_ptr<IModel>& model) noexcept;

		std::shared_ptr<IModel> getReal() const noexcept;
        const LuaModelNode& getRootNode() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<IModel> _model;
		LuaModelNode _rootNode;
	};

	class LuaModelSceneConfigurer final
	{
	public:
		LuaModelSceneConfigurer(const LuaScene& scene, const bgfx::VertexLayout& layout, LuaAssets& assets) noexcept;
		LuaModelSceneConfigurer& setParent(const LuaEntity& parent) noexcept;

		LuaEntity run1(const LuaModel& model) const;
		LuaEntity run2(const LuaModel& model, sol::protected_function callback) const;
		LuaEntity run3(const LuaModelNode& node) const;
		LuaEntity run4(const LuaModelNode& node, sol::protected_function callback) const;

		static void configure(sol::state_view& lua) noexcept;
	private:
		ModelSceneConfigurer _configurer;
		std::shared_ptr<Scene> _scene;
	};

}