#pragma once

#include <memory>
#include <string>
#include <darmok/optional_ref.hpp>
#include <bgfx/bgfx.h>
#include "sol.hpp"

namespace darmok
{
    class Model;
	class ModelNode;
	class LuaEntity;
	class LuaScene;

	class ConstLuaModelNode final
	{
	public:
		ConstLuaModelNode(const ModelNode& node) noexcept;
		const ModelNode& getReal() const noexcept;
		std::string getName() const noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<const ModelNode> _node;
	};

	class LuaModel final
	{
	public:
		LuaModel(const std::shared_ptr<Model>& model) noexcept;
		const std::shared_ptr<Model>& getReal() const noexcept;

		LuaEntity addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout) const;
		LuaEntity addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback) const;
		LuaEntity addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent) const;
		LuaEntity addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback) const;

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Model> _model;
	};

}