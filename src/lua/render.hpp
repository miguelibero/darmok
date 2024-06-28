#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <optional>

namespace darmok
{
    class Renderable;
	class LuaEntity;
	class LuaScene;
    class LuaMesh;
    class LuaTexture;
    class LuaMaterial;
	class LuaProgram;

	class LuaRenderable final
	{
	public:
		LuaRenderable(Renderable& renderable) noexcept;

		const Renderable& getReal() const;
		Renderable& getReal();

		std::optional<LuaMesh> getMesh() const noexcept;
		LuaRenderable& setMesh(const LuaMesh& mesh) noexcept;
		LuaMaterial getMaterial() const noexcept;
		LuaRenderable& setMaterial(const LuaMaterial& material) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Renderable> _renderable;

		static LuaRenderable addEntityComponent1(LuaEntity& entity, const LuaMesh& mesh) noexcept;
		static LuaRenderable addEntityComponent2(LuaEntity& entity, const LuaMaterial& material) noexcept;
		static LuaRenderable addEntityComponent3(LuaEntity& entity, const LuaMesh& mesh, const LuaMaterial& material) noexcept;
		static LuaRenderable addEntityComponent4(LuaEntity& entity, const LuaMesh& mesh, const LuaProgram& prog, const LuaTexture& texture) noexcept;
		static std::optional<LuaRenderable> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}