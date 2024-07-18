#include "render.hpp"
#include "scene.hpp"
#include <darmok/render.hpp>
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>

namespace darmok
{
	LuaRenderer::LuaRenderer(IRenderer& renderer) noexcept
		: _renderer(renderer)
	{
	}

	IRenderer& LuaRenderer::getReal() noexcept
	{
		return _renderer.value();
	}

	const IRenderer& LuaRenderer::getReal() const noexcept
	{
		return _renderer.value();
	}

	void LuaRenderer::bind(sol::state_view& lua) noexcept
	{

	}

	LuaRenderable LuaRenderable::addEntityComponent1(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh) noexcept
	{
		return entity.addComponent<Renderable>(mesh);
	}

	LuaRenderable LuaRenderable::addEntityComponent2(LuaEntity& entity, const std::shared_ptr<Material>& material) noexcept
	{
		return entity.addComponent<Renderable>(material);
	}

	LuaRenderable LuaRenderable::addEntityComponent3(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Material>& material) noexcept
	{
		return entity.addComponent<Renderable>(mesh, material);
	}

	LuaRenderable LuaRenderable::addEntityComponent4(LuaEntity& entity, const std::shared_ptr<IMesh>& mesh, const std::shared_ptr<Program>& prog, const std::shared_ptr<Texture>& texture) noexcept
	{
		return entity.addComponent<Renderable>(mesh, prog, texture);
	}

	std::optional<LuaRenderable> LuaRenderable::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Renderable, LuaRenderable>();
	}

	std::optional<LuaEntity> LuaRenderable::getEntity(LuaScene& scene) noexcept
	{
		if (!_renderable)
		{
			return std::nullopt;
		}
		return scene.getEntity(_renderable.value());
	}

	LuaRenderable::LuaRenderable(Renderable& renderable) noexcept
		: _renderable(renderable)
	{
	}

	const Renderable& LuaRenderable::getReal() const
	{
		return _renderable.value();
	}

	Renderable& LuaRenderable::getReal()
	{
		return _renderable.value();
	}

	std::shared_ptr<IMesh> LuaRenderable::getMesh() const noexcept
	{
		return _renderable->getMesh();
	}

	void LuaRenderable::setMesh(const std::shared_ptr<IMesh>& mesh) noexcept
	{
		_renderable->setMesh(mesh);
	}

	std::shared_ptr<Material> LuaRenderable::getMaterial() const noexcept
	{
		return _renderable->getMaterial();
	}

	void LuaRenderable::setMaterial(const std::shared_ptr<Material>& material) noexcept
	{
		_renderable->setMaterial(material);
	}

	bool LuaRenderable::getEnabled() const noexcept
	{
		return _renderable->isEnabled();
	}

	void LuaRenderable::setEnabled(bool enabled) noexcept
	{
		_renderable->setEnabled(enabled);
	}

	void LuaRenderable::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaRenderable>("Renderable", sol::no_constructor,
			"type_id", &entt::type_hash<Renderable>::value,
			"add_entity_component", sol::overload(
				&LuaRenderable::addEntityComponent1,
				&LuaRenderable::addEntityComponent2,
				&LuaRenderable::addEntityComponent3,
				&LuaRenderable::addEntityComponent4
			),
			"get_entity_component", &LuaRenderable::getEntityComponent,
			"get_entity", &LuaRenderable::getEntity,
			"mesh", sol::property(&LuaRenderable::getMesh, &LuaRenderable::setMesh),
			"material", sol::property(&LuaRenderable::getMaterial, &LuaRenderable::setMaterial),
			"enabled", sol::property(&LuaRenderable::getEnabled, &LuaRenderable::setEnabled)
		);
	}
}
