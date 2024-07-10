#include "camera.hpp"
#include "scene.hpp"
#include "render_forward.hpp"
#include "light.hpp"
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/light.hpp>

#ifdef DARMOK_JOLT
#include "physics3d_debug.hpp"
#endif

#ifdef DARMOK_OZZ
#include "skeleton.hpp"
#include <darmok/skeleton.hpp>
#endif

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

	LuaCamera& LuaCamera::setOrtho1(const VarViewport& vp, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		_camera->setOrtho(LuaViewport::tableGet(vp), LuaGlm::tableGet(center), near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho2(const VarViewport& vp, const VarLuaTable<glm::vec2>& center) noexcept
	{
		_camera->setOrtho(LuaViewport::tableGet(vp), LuaGlm::tableGet(center));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho3(const VarViewport& vp) noexcept
	{
		_camera->setOrtho(LuaViewport::tableGet(vp));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho4(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(size), LuaGlm::tableGet(center), near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho5(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(size), LuaGlm::tableGet(center));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho6(const VarLuaTable<glm::uvec2>& size) noexcept
	{
		_camera->setOrtho(LuaGlm::tableGet(size));
		return *this;
	}

	void LuaCamera::setTargetTextures(const std::vector<std::shared_ptr<Texture>>& textures) noexcept
	{
		_camera->setTargetTextures(textures);
	}

	std::vector<std::shared_ptr<Texture>> LuaCamera::getTargetTextures() noexcept
	{
		return _camera->getTargetTextures();
	}

	std::optional<Viewport> LuaCamera::getViewport() const noexcept
	{
		return _camera->getViewport();
	}


	void LuaCamera::setViewport(std::optional<VarViewport> viewport) noexcept
	{
		_camera->setViewport(LuaViewport::tableGet(viewport));
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

	bool LuaCamera::getEnabled() const noexcept
	{
		return _camera->isEnabled();
	}

	void LuaCamera::setEnabled(bool enabled) noexcept
	{
		_camera->setEnabled(enabled);
	}

	bool LuaCamera::getRendererEnabled() const noexcept
	{
		return _camera->isRendererEnabled();
	}

	void LuaCamera::setRendererEnabled(bool enabled) noexcept
	{
		_camera->setRendererEnabled(enabled);
	}

	void LuaCamera::setProjectionMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept
	{
		_camera->setProjectionMatrix(LuaGlm::tableGet(matrix));
	}

	Ray LuaCamera::screenPointToRay1(const glm::vec2& point) const noexcept
	{
		return _camera->screenPointToRay(glm::vec3(point, 0));
	}

	Ray LuaCamera::screenPointToRay2(const VarLuaTable<glm::vec3>& point) const noexcept
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
		LuaForwardRenderer::bind(lua);
		LuaPhongLightingComponent::bind(lua);

#ifdef DARMOK_JOLT
		physics3d::LuaPhysicsDebugRenderer::bind(lua);
#endif

#ifdef DARMOK_OZZ
		LuaSkeletalAnimationCameraComponent::bind(lua);
#endif

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
				&LuaCamera::setOrtho4,
				&LuaCamera::setOrtho5,
				&LuaCamera::setOrtho6
			),
			"enabled", sol::property(&LuaCamera::getEnabled, &LuaCamera::setEnabled),
			"renderer_enabled", sol::property(&LuaCamera::getRendererEnabled, &LuaCamera::setRendererEnabled),
			"projection_matrix", sol::property(&LuaCamera::getProjectionMatrix, &LuaCamera::setProjectionMatrix),
			"target_textures", sol::property(&LuaCamera::getTargetTextures, &LuaCamera::setTargetTextures),
			"viewport", sol::property(&LuaCamera::getViewport, &LuaCamera::setViewport),
			"current_viewport", sol::property(&LuaCamera::getCurrentViewport),
			"transform", sol::property(&LuaCamera::getTransform),
			"model_matrix", sol::property(&LuaCamera::getModelMatrix),
			"screen_point_to_ray", sol::overload(&LuaCamera::screenPointToRay1, &LuaCamera::screenPointToRay2),
			"viewport_point_to_ray", &LuaCamera::viewportPointToRay,
			"world_to_screen_point", &LuaCamera::worldToScreenPoint,
			"world_to_viewport_point", &LuaCamera::worldToViewportPoint,
			"screen_to_world_point", &LuaCamera::screenToWorldPoint,
			"viewport_to_world_point", &LuaCamera::viewportToWorldPoint,
			"viewport_to_screen_point", &LuaCamera::viewportToScreenPoint,
			"screen_to_viewport_point", &LuaCamera::screenToViewportPoint
		);

		lua.script(R"(
function Camera:add_component(type, ...)
	return type.add_camera_component(self, ...)
end
function Camera:set_renderer(type, ...)
	return type.set_camera_renderer(self, ...)
end
)");
	}
}