#include "scene.hpp"
#include "model.hpp"
#include "app.hpp"
#include <darmok/transform.hpp>
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/mesh.hpp>
#include <darmok/model.hpp>

namespace darmok
{
	void LuaTableComponent::addEntry(const std::string& name, const sol::table& data)
	{
		_tables.emplace(name, data);
	}

	void LuaTableComponent::setEntry(const std::string& name, const sol::table& data) noexcept
	{
		_tables[name] = data;
	}

	const sol::table& LuaTableComponent::getEntry(const std::string& name) const
	{
		return _tables.at(name);
	}

	sol::table& LuaTableComponent::getEntry(const std::string& name)
	{
		return _tables.at(name);
	}

	bool LuaTableComponent::hasEntry(const std::string& name) const noexcept
	{
		return _tables.find(name) != _tables.end();
	}

	bool LuaTableComponent::removeEntry(const std::string& name) noexcept
	{
		return _tables.erase(name) > 0;
	}

	LuaComponent::LuaComponent(const std::string& name, LuaTableComponent& table) noexcept
		: _name(name)
		, _table(table)
	{
	}

	const std::string& LuaComponent::getName() const noexcept
	{
		return _name;
	}

	sol::table& LuaComponent::getData()
	{
		return _table.getEntry(_name);
	}

	void LuaComponent::setData(const sol::table& data) noexcept
	{
		_table.setEntry(_name, data);
	}

	void LuaComponent::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaComponent>("Component",
			sol::constructors<>(),
			"name", sol::property(&LuaComponent::getName),
			"data", sol::property(&LuaComponent::getData, &LuaComponent::setData)
		);
	}

	LuaEntity::LuaEntity(Entity entity, const std::weak_ptr<Scene>& scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
	}

	std::string LuaEntity::to_string() const noexcept
	{
		std::string str = "Entity(" + std::to_string(_entity);
		if (!isValid())
		{
			str += " invalid";
		}
		return str + ")";
	}

	bool LuaEntity::isValid() const noexcept
	{
		if (auto scene = _scene.lock())
		{
			return scene->getRegistry().valid(_entity);
		}
		return false;
	}

	EntityRegistry& LuaEntity::getRegistry()
	{
		if (auto scene = _scene.lock())
		{
			return scene->getRegistry();
		}
		throw std::runtime_error("scene expired");
	}

	const EntityRegistry& LuaEntity::getRegistry() const
	{
		if (auto scene = _scene.lock())
		{
			return scene->getRegistry();
		}
		throw std::runtime_error("scene expired");
	}

	LuaComponent LuaEntity::addLuaComponent(const std::string& name, const sol::table& data)
	{
		auto& table = getRegistry().get_or_emplace<LuaTableComponent>(_entity);
		table.addEntry(name, data);
		return LuaComponent(name, table);
	}

	LuaComponent LuaEntity::getLuaComponent(const std::string& name)
	{
		auto& table = getRegistry().get<LuaTableComponent>(_entity);
		if (!table.hasEntry(name))
		{
			throw std::runtime_error("expected lua component missing");
		}
		return LuaComponent(name, table);
	}

	bool LuaEntity::hasLuaComponent(const std::string& name) const
	{
		auto table = getRegistry().try_get<LuaTableComponent>(_entity);
		return table != nullptr && table->hasEntry(name);
	}

	std::optional<LuaComponent> LuaEntity::tryGetLuaComponent(const std::string& name)
	{
		auto table = getRegistry().try_get<LuaTableComponent>(_entity);
		if (table == nullptr || !table->hasEntry(name))
		{
			return std::nullopt;
		}
		return LuaComponent(name, *table);
	}

	bool LuaEntity::removeLuaComponent(const std::string& name)
	{
		auto table = getRegistry().try_get<LuaTableComponent>(_entity);
		if (table == nullptr)
		{
			return false;
		}
		return table->removeEntry(name);
	}

	LuaComponent LuaEntity::getOrAddLuaComponent(const std::string& name)
	{
		auto& table = getRegistry().get_or_emplace<LuaTableComponent>(_entity);
		return LuaComponent(name, table);
	}

	const Entity& LuaEntity::getReal() const noexcept
	{
		return _entity;
	}

	LuaEntity LuaEntity::addModel1(const LuaModel& model, const bgfx::VertexLayout& layout)
	{
		auto scenePtr = _scene.lock();
		if (scenePtr == nullptr)
		{
			throw std::runtime_error("scene expired");
		}
		LuaScene scene(scenePtr);
		return model.addToScene3(scene, layout, *this);
	}

	LuaEntity LuaEntity::addModel2(const LuaModel& model, const bgfx::VertexLayout& layout, sol::protected_function callback)
	{
		auto scenePtr = _scene.lock();
		if (scenePtr == nullptr)
		{
			throw std::runtime_error("scene expired");
		}
		LuaScene scene(scenePtr);
		return model.addToScene4(scene, layout, *this, callback);
	}

	void LuaEntity::configure(sol::state_view& lua) noexcept
	{
		lua.new_enum<LuaNativeComponentType>("ComponentType", {
			{ "Transform", LuaNativeComponentType::Transform },
			{ "Camera", LuaNativeComponentType::Camera },
			{ "AmbientLight", LuaNativeComponentType::AmbientLight },
			{ "PointLight", LuaNativeComponentType::PointLight },
			{ "Mesh", LuaNativeComponentType::Mesh },
		});

		lua.new_usertype<LuaEntity>("Entity", sol::constructors<>(),
			"add_component", sol::overload(&LuaEntity::addNativeComponent<0>, &LuaEntity::addLuaComponent),
			"get_component", sol::overload(&LuaEntity::getNativeComponent<0>, &LuaEntity::getLuaComponent),
			"remove_component", sol::overload(&LuaEntity::removeNativeComponent<0>, &LuaEntity::removeLuaComponent),
			"get_or_add_component", sol::overload(&LuaEntity::getOrAddNativeComponent<0>, &LuaEntity::getOrAddLuaComponent),
			"try_get_component", sol::overload(&LuaEntity::tryGetNativeComponent<0>, &LuaEntity::tryGetLuaComponent),
			"has_component", sol::overload(&LuaEntity::hasNativeComponent<0>, &LuaEntity::hasLuaComponent),
			"add_model", sol::overload(&LuaEntity::addModel1, &LuaEntity::addModel2)
		);
	}

	LuaScene::LuaScene(const std::shared_ptr<Scene>& scene) noexcept
		: _scene(scene == nullptr ? std::make_shared<Scene>() : scene)
	{
	}

	LuaScene::LuaScene(LuaApp& app) noexcept
		: _scene(std::make_shared<Scene>())
	{
		_scene->init(app.getReal());
	}

	std::string LuaScene::to_string() const noexcept
	{
		return "Scene()";
	}

	EntityRegistry& LuaScene::getRegistry() noexcept
	{
		return _scene->getRegistry();
	}

	LuaEntity LuaScene::createEntity() noexcept
	{
		return LuaEntity(getRegistry().create(), std::weak_ptr<Scene>(_scene));
	}

	bool LuaScene::destroyEntity(const LuaEntity& entity) noexcept
	{
		return getRegistry().destroy(entity.getReal()) != 0;
	}

	const std::shared_ptr<Scene>& LuaScene::getReal() const noexcept
	{
		return _scene;
	}

	std::shared_ptr<Scene>& LuaScene::getReal() noexcept
	{
		return _scene;
	}

	void LuaScene::configure(sol::state_view& lua) noexcept
	{
		LuaTransform::configure(lua);
		LuaCamera::configure(lua);
		LuaEntity::configure(lua);
		LuaAmbientLight::configure(lua);
		LuaPointLight::configure(lua);
		LuaMeshComponent::configure(lua);
		LuaComponent::configure(lua);

		lua.new_usertype<LuaScene>("Scene",
			sol::constructors<LuaScene(LuaApp&)>(),
			"create_entity",	&LuaScene::createEntity,
			"destroy_entity",	&LuaScene::destroyEntity,
			"get_entity",		&LuaScene::getEntity<0>
		);
	}
}