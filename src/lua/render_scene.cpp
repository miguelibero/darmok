#include "render_scene.hpp"
#include "scene.hpp"
#include <darmok/render_scene.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	Renderable& LuaRenderable::addEntityComponent1(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh) noexcept
	{
		return entity.addComponent<Renderable>(mesh);
	}

	Renderable& LuaRenderable::addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& material) noexcept
	{
		return entity.addComponent<Renderable>(material);
	}

	Renderable& LuaRenderable::addEntityComponent3(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept
	{
		return entity.addComponent<Renderable>(mesh, material);
	}

	Renderable& LuaRenderable::addEntityComponent4(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& prog, const std::shared_ptr<Texture>& texture) noexcept
	{
		return entity.addComponent<Renderable>(mesh, prog, texture);
	}

	Renderable& LuaRenderable::addEntityComponent5(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& prog, const Color& color) noexcept
	{
		return entity.addComponent<Renderable>(mesh, prog, color);
	}

	OptionalRef<Renderable>::std_t LuaRenderable::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Renderable>();
	}

	std::optional<LuaEntity> LuaRenderable::getEntity(const Renderable& renderable, LuaScene& scene) noexcept
	{
		return scene.getEntity(renderable);
	}

	void LuaRenderable::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Renderable>("Renderable", sol::no_constructor,
			"type_id", &entt::type_hash<Renderable>::value,
			"add_entity_component", sol::overload(
				&LuaRenderable::addEntityComponent1,
				&LuaRenderable::addEntityComponent2,
				&LuaRenderable::addEntityComponent3,
				&LuaRenderable::addEntityComponent4,
				&LuaRenderable::addEntityComponent5
			),
			"get_entity_component", &LuaRenderable::getEntityComponent,
			"get_entity", &LuaRenderable::getEntity,
			"mesh", sol::property(&Renderable::getMesh, &Renderable::setMesh),
			"material", sol::property(&Renderable::getMaterial, &Renderable::setMaterial),
			"enabled", sol::property(&Renderable::isEnabled, &Renderable::setEnabled)
		);
	}
}
