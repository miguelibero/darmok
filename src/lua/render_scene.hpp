#pragma once

#include "lua.hpp"
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <darmok/color_fwd.hpp>
#include <optional>

namespace darmok
{
    class Renderable;
	class LuaEntity;
	class Scene;
    class IMesh;
    class Texture;
    struct Material;
	class Program;

	class LuaRenderable final
	{
	public:
		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Renderable> _renderable;

		static Renderable& addEntityComponent1(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh) noexcept;
		static Renderable& addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& material) noexcept;
		static Renderable& addEntityComponent3(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept;
		static Renderable& addEntityComponent4(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& prog, const std::shared_ptr<Texture>& texture) noexcept;
		static Renderable& addEntityComponent5(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& prog, const Color& color) noexcept;
		static OptionalRef<Renderable>::std_t getEntityComponent(LuaEntity& entity) noexcept;
		static std::optional<LuaEntity> getEntity(const Renderable& renderable, const std::shared_ptr<Scene>& scene) noexcept;
	};
}