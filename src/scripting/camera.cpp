#include "camera.hpp"
#include "program.hpp"
#include "texture.hpp"
#include "scene.hpp"
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/light.hpp>
#include <darmok/skeleton.hpp>

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

	LuaCamera& LuaCamera::setPerspective1(float fovy, float aspect, float near, float far) noexcept
	{
		_camera->setPerspective(fovy, aspect, near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setPerspective2(float fovy, float aspect, float near) noexcept
	{
		_camera->setPerspective(fovy, aspect, near);
		return *this;
	}

	LuaCamera& LuaCamera::setPerspective3(float fovy, const VarLuaTable<glm::uvec2>& size, float near, float far) noexcept
	{
		_camera->setPerspective(fovy, LuaGlm::tableGet(size), near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setPerspective4(float fovy, const VarLuaTable<glm::uvec2>& size, float near) noexcept
	{
		_camera->setPerspective(fovy, LuaGlm::tableGet(size), near);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho1(const VarViewport& vp, float near, float far) noexcept
	{
		_camera->setOrtho(LuaViewport::tableGet(vp), near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho2(const VarViewport& vp) noexcept
	{
		_camera->setOrtho(LuaViewport::tableGet(vp));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho3(const VarLuaTable<glm::uvec2>& size, float near, float far) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(size), near, far);
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
		case LuaNativeCameraComponentType::SkeletalAnimation:
			_camera->addComponent<SkeletalAnimationCameraComponent>();
			break;
		}
		return *this;
	}

	LuaCamera& LuaCamera::setForwardRenderer() noexcept
	{
		_camera->setRenderer<ForwardRenderer>();
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

	std::optional<Viewport> LuaCamera::getViewport() const noexcept
	{
		return _camera->getViewport();
	}


	LuaCamera& LuaCamera::setViewport(std::optional<VarViewport> viewport) noexcept
	{
		_camera->setViewport(LuaViewport::tableGet(viewport));
		return *this;
	}

	Viewport LuaCamera::getCurrentViewport() const noexcept
	{
		return _camera->getCurrentViewport();
	}

	std::optional<LuaTransform> LuaCamera::getTransform() const noexcept
	{
		auto trans = _camera->getTransform();
		if (trans)
		{
			return LuaTransform(trans.value());
		}
		return std::nullopt;
	}

	glm::mat4 LuaCamera::getModelMatrix() const noexcept
	{
		return _camera->getModelMatrix();
	}

	const glm::mat4& LuaCamera::getProjectionMatrix() const noexcept
	{
		return _camera->getProjectionMatrix();
	}

	void LuaCamera::setProjectionMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept
	{
		_camera->setProjectionMatrix(LuaGlm::tableGet(matrix));
	}

	Ray LuaCamera::screenPointToRay(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->screenPointToRay(LuaGlm::tableGet(point));
	}

	Ray LuaCamera::viewportPointToRay(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->viewportPointToRay(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::worldToScreenPoint(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->worldToScreenPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::worldToViewportPoint(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->worldToViewportPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::screenToWorldPoint(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->screenToWorldPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::viewportToWorldPoint(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->viewportToWorldPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::viewportToScreenPoint(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->viewportToScreenPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::screenToViewportPoint(const VarLuaTable<glm::vec3>& point) const noexcept
	{
		return _camera->screenToViewportPoint(LuaGlm::tableGet(point));
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
		LuaViewport::bind(lua);

		lua.new_enum<LuaNativeCameraComponentType>("CameraComponentType", {
			{ "phong_lighting", LuaNativeCameraComponentType::PhongLighting },
		});

		lua.new_usertype<LuaCamera>("Camera", sol::no_constructor,
			"type_id", &entt::type_hash<Camera>::value,
			"add_entity_component", &LuaCamera::addEntityComponent,
			"get_entity_component", &LuaCamera::getEntityComponent,
			"get_entity", &LuaCamera::getEntity,
			"set_perspective", sol::overload(
				&LuaCamera::setPerspective1,
				&LuaCamera::setPerspective2,
				&LuaCamera::setPerspective3,
				&LuaCamera::setPerspective4
			),
			"set_ortho", sol::overload(
				&LuaCamera::setOrtho1,
				&LuaCamera::setOrtho2,
				&LuaCamera::setOrtho3,
				&LuaCamera::setOrtho4
			),
			"set_forward_renderer", &LuaCamera::setForwardRenderer,
			"add_component", &LuaCamera::addNativeComponent,
			"projection_matrix", sol::property(&LuaCamera::getProjectionMatrix, &LuaCamera::setProjectionMatrix),
			"target_textures", sol::property(&LuaCamera::getTargetTextures, &LuaCamera::setTargetTextures),
			"viewport", sol::property(&LuaCamera::getViewport, &LuaCamera::setViewport),
			"current_viewport", sol::property(&LuaCamera::getCurrentViewport),
			"transform", sol::property(&LuaCamera::getTransform),
			"model_matrix", sol::property(&LuaCamera::getModelMatrix),
			"screen_point_to_ray", &LuaCamera::screenPointToRay,
			"viewport_point_to_ray", &LuaCamera::viewportPointToRay,
			"world_to_screen_point", &LuaCamera::worldToScreenPoint,
			"world_to_viewport_point", &LuaCamera::worldToViewportPoint,
			"screen_to_world_point", &LuaCamera::screenToWorldPoint,
			"viewport_to_world_point", &LuaCamera::viewportToWorldPoint,
			"viewport_to_screen_point", &LuaCamera::viewportToScreenPoint,
			"screen_to_viewport_point", &LuaCamera::screenToViewportPoint
		);
	}
}