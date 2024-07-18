#pragma once

#include <sol/sol.hpp>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <optional>

namespace darmok
{
	class IRenderer;

	class LuaRenderer final
	{
	public:
		LuaRenderer(IRenderer& renderer) noexcept;

		IRenderer& getReal() noexcept;
		const IRenderer& getReal() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<IRenderer> _renderer;
	};

    class Renderable;
	class LuaEntity;
	class LuaScene;
    class IMesh;
    class Texture;
    class Material;
	class Program;

	class LuaRenderable final
	{
	public:
		LuaRenderable(Renderable& renderable) noexcept;

		const Renderable& getReal() const;
		Renderable& getReal();

		std::shared_ptr<IMesh> getMesh() const noexcept;
		void setMesh(const std::shared_ptr<IMesh>& mesh) noexcept;
		std::shared_ptr<Material> getMaterial() const noexcept;
		void setMaterial(const std::shared_ptr<Material>& material) noexcept;
		bool getEnabled() const noexcept;
		void setEnabled(bool enabled) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Renderable> _renderable;

		static LuaRenderable addEntityComponent1(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh) noexcept;
		static LuaRenderable addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& material) noexcept;
		static LuaRenderable addEntityComponent3(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept;
		static LuaRenderable addEntityComponent4(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& prog, const std::shared_ptr<Texture>& texture) noexcept;
		static std::optional<LuaRenderable> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}