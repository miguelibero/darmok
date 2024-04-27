#pragma once

#include <memory>
#include <string>
#include <darmok/optional_ref.hpp>
#include <bgfx/bgfx.h>
#include "sol.hpp"

namespace darmok
{
	class AssimpNode;
	class LuaEntity;
	class LuaScene;

	class ConstLuaAssimpNode final
	{
	public:
		ConstLuaAssimpNode(const AssimpNode& node) noexcept;
		const AssimpNode& getReal() const noexcept;
		std::string getName() const noexcept;

		LuaEntity addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout) const;
		LuaEntity addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback) const;
		LuaEntity addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent) const;
		LuaEntity addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback) const;

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<const AssimpNode> _node;
	};

    class AssimpScene;

	class LuaAssimpScene final
	{
	public:
		LuaAssimpScene(const std::shared_ptr<AssimpScene>& scene) noexcept;
		const std::shared_ptr<AssimpScene>& getReal() const noexcept;

		LuaEntity addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout) const;
		LuaEntity addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback) const;
		LuaEntity addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent) const;
		LuaEntity addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback) const;

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<AssimpScene> _scene;
	};

}