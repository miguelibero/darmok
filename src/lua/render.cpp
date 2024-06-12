#include "render.hpp"
#include "material.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "texture.hpp"
#include <darmok/render.hpp>

namespace darmok
{
    
	LuaRenderable LuaRenderable::addEntityComponent1(LuaEntity& entity, const LuaMesh& mesh) noexcept
	{
		return entity.addComponent<Renderable>(mesh.getReal());
	}

	LuaRenderable LuaRenderable::addEntityComponent2(LuaEntity& entity, const LuaMaterial& material) noexcept
	{
		return entity.addComponent<Renderable>(material.getReal());
	}

	LuaRenderable LuaRenderable::addEntityComponent3(LuaEntity& entity, const LuaMesh& mesh, const LuaMaterial& material) noexcept
	{
		return entity.addComponent<Renderable>(mesh.getReal(), material.getReal());
	}

	LuaRenderable LuaRenderable::addEntityComponent4(LuaEntity& entity, const LuaMesh& mesh, const LuaTexture& texture) noexcept
	{
		return entity.addComponent<Renderable>(mesh.getReal(), texture.getReal());
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

	std::optional<LuaMesh> LuaRenderable::getMesh() const noexcept
	{
		auto mesh = _renderable->getMesh();
		if (mesh == nullptr)
		{
			return std::nullopt;
		}
		return LuaMesh(mesh);
	}

	LuaRenderable& LuaRenderable::setMesh(const LuaMesh& mesh) noexcept
	{
		_renderable->setMesh(mesh.getReal());
		return *this;
	}

	LuaMaterial LuaRenderable::getMaterial() const noexcept
	{
		return LuaMaterial(_renderable->getMaterial());
	}

	LuaRenderable& LuaRenderable::setMaterial(const LuaMaterial& material) noexcept
	{
		_renderable->setMaterial(material.getReal());
		return *this;
	}

	void LuaRenderable::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaRenderable>("Renderable", sol::no_constructor,
			"type_id", &entt::type_hash<Renderable>::value,
			"add_entity_component", sol::overload(
				&LuaRenderable::addEntityComponent1,
				&LuaRenderable::addEntityComponent2,
				&LuaRenderable::addEntityComponent3
			),
			"get_entity_component", &LuaRenderable::getEntityComponent,
			"get_entity", &LuaRenderable::getEntity,
			"mesh", sol::property(&LuaRenderable::getMesh, &LuaRenderable::setMesh),
			"material", sol::property(&LuaRenderable::getMaterial, &LuaRenderable::setMaterial)
		);
	}
}
