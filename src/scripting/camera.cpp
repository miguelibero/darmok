#include "camera.hpp"
#include "program.hpp"
#include "texture.hpp"
#include "scene.hpp"
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/light.hpp>

namespace darmok
{
    LuaCamera::LuaCamera(Camera& camera) noexcept
		: _camera(camera)
	{
	}

	const Camera& LuaCamera::getReal() const
	{
		return _camera.value();
	}

	Camera& LuaCamera::getReal()
	{
		return _camera.value();
	}

	glm::uvec2 LuaCamera::getScreenPoint(const VarLuaTable<glm::vec2>& normPoint) const noexcept
	{
		return _camera->getScreenPoint(LuaGlm::tableGet(normPoint));
	}

	LuaCamera& LuaCamera::setProjection1(float fovy, float aspect, const VarLuaTable<glm::vec2>& range) noexcept
	{
		_camera->setProjection(fovy, aspect, LuaGlm::tableGet(range));
		return *this;
	}

	LuaCamera& LuaCamera::setProjection2(float fovy, float aspect, float near) noexcept
	{
		_camera->setProjection(fovy, aspect, near);
		return *this;
	}

	LuaCamera& LuaCamera::setProjection3(float fovy, const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& range) noexcept
	{
		_camera->setProjection(fovy, LuaGlm::tableGet(size), LuaGlm::tableGet(range));
		return *this;
	}

	LuaCamera& LuaCamera::setProjection4(float fovy, const VarLuaTable<glm::uvec2>& size, float near) noexcept
	{
		_camera->setProjection(fovy, LuaGlm::tableGet(size), near);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho1(const VarLuaTable<glm::vec4>& edges, const VarLuaTable<glm::vec2>& range) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(edges), LuaGlm::tableGet(range));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho2(const VarLuaTable<glm::vec4>& edges) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(edges));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho3(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& range) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(size), LuaGlm::tableGet(range));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho4(const VarLuaTable<glm::uvec2>& size) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(size));
		return *this;
	}

	LuaCamera& LuaCamera::setTargetTextures(const sol::table& textures) noexcept
	{
		std::vector<std::shared_ptr<Texture>> realTextures;
		realTextures.reserve(textures.size());
		for (auto& elm : textures)
		{
			if (elm.second.is<LuaTexture>())
			{
				auto luaTexture = elm.second.as<LuaTexture>();
				realTextures.push_back(luaTexture.getReal());
			}
		}
		_camera->setTargetTextures(realTextures);
		return *this;
	}

	LuaCamera& LuaCamera::addNativeComponent(LuaNativeCameraComponentType type) noexcept
	{
		switch (type)
		{
		case LuaNativeCameraComponentType::PhongLighting:
			_camera->addComponent<PhongLightingComponent>();
			break;
		}
		return *this;
	}

	LuaCamera& LuaCamera::setForwardRenderer(const LuaProgram& program) noexcept
	{
		_camera->setRenderer<ForwardRenderer>(program.getReal());
		return *this;
	}

	std::vector<LuaTexture> LuaCamera::getTargetTextures() noexcept
	{
		auto& textures = _camera->getTargetTextures();
		std::vector<LuaTexture> luaTextures;
		luaTextures.reserve(textures.size());
		for (auto& tex : textures)
		{
			luaTextures.push_back(LuaTexture(tex));
		}
		return luaTextures;
	}

	const glm::mat4& LuaCamera::getMatrix() const noexcept
	{
		return _camera->getMatrix();
	}

	void LuaCamera::setMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept
	{
		_camera->setMatrix(LuaGlm::tableGet(matrix));
	}

	Ray LuaCamera::screenPointToRay(const VarLuaTable<glm::vec2>& point) const noexcept
	{
		return _camera->screenPointToRay(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::worldToScreenPoint(const VarLuaTable<glm::vec3>& position) const noexcept
	{
		return _camera->worldToScreenPoint(LuaGlm::tableGet(position));
	}

	LuaCamera LuaCamera::addEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.addComponent<Camera>();
	}

	std::optional<LuaCamera> LuaCamera::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Camera, LuaCamera>();
	}

	std::optional<LuaEntity> LuaCamera::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_camera.value());
	}

	void LuaCamera::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<LuaNativeCameraComponentType>("CameraComponentType", {
			{ "PhongLighting", LuaNativeCameraComponentType::PhongLighting },
		});

		lua.new_usertype<LuaCamera>("Camera",
			sol::constructors<>(),
			"type_id", &entt::type_hash<Camera>::value,
			"add_entity_component", &LuaCamera::addEntityComponent,
			"get_entity_component", &LuaCamera::getEntityComponent,
			"get_entity", &LuaCamera::getEntity,
			"set_projection", sol::overload(
				&LuaCamera::setProjection1,
				&LuaCamera::setProjection2,
				&LuaCamera::setProjection3,
				&LuaCamera::setProjection4
			),
			"set_ortho", sol::overload(
				&LuaCamera::setOrtho1,
				&LuaCamera::setOrtho2,
				&LuaCamera::setOrtho3,
				&LuaCamera::setOrtho4
			),
			"set_forward_renderer", &LuaCamera::setForwardRenderer,
			"add_component", sol::overload(&LuaCamera::addNativeComponent),
			"matrix", sol::property(&LuaCamera::getMatrix, &LuaCamera::setMatrix),
			"target_textures", sol::property(&LuaCamera::getTargetTextures, &LuaCamera::setTargetTextures),
			"screen_point_to_ray", &LuaCamera::screenPointToRay,
			"world_to_screen_point", &LuaCamera::worldToScreenPoint,
			"get_screen_point", &LuaCamera::getScreenPoint
		);
	}
}