#pragma once

#include <memory>
#include <bgfx/bgfx.h>
#include "sol.hpp"

namespace darmok
{
    class Model;
	class LuaEntity;
	class LuaScene;

	class LuaModel final
	{
	public:
		LuaModel(const std::shared_ptr<Model>& model) noexcept;
		const std::shared_ptr<Model>& getReal() const noexcept;

		LuaEntity addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout);
		LuaEntity addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback);
		LuaEntity addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent);
		LuaEntity addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback);

		static void configure(sol::state_view& lua) noexcept;

	private:
		std::shared_ptr<Model> _model;
	};

}