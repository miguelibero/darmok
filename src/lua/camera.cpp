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
#ifdef _DEBUG
#define PHYSICS_DEBUG_RENDER
#include "physics3d_debug.hpp"
#endif
#endif

#ifdef DARMOK_OZZ
#include "skeleton.hpp"
#include <darmok/skeleton.hpp>
#endif

namespace darmok
{
	Camera& LuaCamera::setPerspective1(Camera& cam, float fovy, float aspect, float near, float far) noexcept
	{
		return cam.setPerspective(fovy, aspect, near, far);
	}

	Camera& LuaCamera::setPerspective2(Camera& cam, float fovy, float aspect, float near) noexcept
	{
		return cam.setPerspective(fovy, aspect, near);
	}

	Camera& LuaCamera::setPerspective3(Camera& cam, float fovy, const VarLuaTable<glm::uvec2>& size, float near, float far) noexcept
	{
		return cam.setPerspective(fovy, LuaGlm::tableGet(size), near, far);
	}

	Camera& LuaCamera::setPerspective4(Camera& cam, float fovy, const VarLuaTable<glm::uvec2>& size, float near) noexcept
	{
		return cam.setPerspective(fovy, LuaGlm::tableGet(size), near);
	}

	Camera& LuaCamera::setViewportPerspective1(Camera& cam, float fovy) noexcept
	{
		return cam.setViewportPerspective(fovy);
	}

	Camera& LuaCamera::setViewportPerspective2(Camera& cam, float fovy, float near, float far) noexcept
	{
		return cam.setViewportPerspective(fovy, near, far);
	}

	Camera& LuaCamera::setOrtho1(Camera& cam, const VarViewport& vp, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		return cam.setOrtho(LuaViewport::tableGet(vp), LuaGlm::tableGet(center), near, far);
	}

	Camera& LuaCamera::setOrtho2(Camera& cam, const VarViewport& vp, const VarLuaTable<glm::vec2>& center) noexcept
	{
		return cam.setOrtho(LuaViewport::tableGet(vp), LuaGlm::tableGet(center));
	}

	Camera& LuaCamera::setOrtho3(Camera& cam, const VarViewport& vp) noexcept
	{
		return cam.setOrtho(LuaViewport::tableGet(vp));
	}

	Camera& LuaCamera::setOrtho4(Camera& cam, const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		return cam.setOrtho(LuaGlm::tableGet(size), LuaGlm::tableGet(center), near, far);
	}

	Camera& LuaCamera::setOrtho5(Camera& cam, const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center) noexcept
	{
		return cam.setOrtho(LuaGlm::tableGet(size), LuaGlm::tableGet(center));
	}

	Camera& LuaCamera::setOrtho6(Camera& cam, const VarLuaTable<glm::uvec2>& size) noexcept
	{
		return cam.setOrtho(LuaGlm::tableGet(size));
	}

	Camera& LuaCamera::setViewportOrtho1(Camera& cam, const VarLuaTable<glm::vec2>& center) noexcept
	{
		return cam.setViewportOrtho(LuaGlm::tableGet(center));
	}

	Camera& LuaCamera::setViewportOrtho2(Camera& cam, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		return cam.setViewportOrtho(LuaGlm::tableGet(center), near, far);
	}

	void LuaCamera::setViewport(Camera& cam, std::optional<VarViewport> viewport) noexcept
	{
		cam.setViewport(LuaViewport::tableGet(viewport));
	}

	std::optional<Viewport> LuaCamera::getViewport(const Camera& cam) noexcept
	{
		return cam.getViewport();
	}

	OptionalRef<Transform>::std_t LuaCamera::getTransform(const Camera& cam) noexcept
	{
		return cam.getTransform();
	}

	void LuaCamera::setProjectionMatrix(Camera& cam, const VarLuaTable<glm::mat4>& matrix) noexcept
	{
		cam.setProjectionMatrix(LuaGlm::tableGet(matrix));
	}

	Ray LuaCamera::screenPointToRay1(const Camera& cam, const glm::vec2& point) noexcept
	{
		return cam.screenPointToRay(glm::vec3(point, 0));
	}

	Ray LuaCamera::screenPointToRay2(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.screenPointToRay(LuaGlm::tableGet(point));
	}

	Ray LuaCamera::viewportPointToRay(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.viewportPointToRay(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::worldToScreenPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.worldToScreenPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::worldToViewportPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.worldToViewportPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::screenToWorldPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.screenToWorldPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::viewportToWorldPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.viewportToWorldPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::viewportToScreenPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.viewportToScreenPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::screenToViewportPoint(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.screenToViewportPoint(LuaGlm::tableGet(point));
	}

	Camera& LuaCamera::addEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.addComponent<Camera>();
	}

	OptionalRef<Camera>::std_t LuaCamera::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Camera>();
	}

	std::optional<LuaEntity> LuaCamera::getEntity(const Camera& cam, LuaScene& scene) noexcept
	{
		return scene.getEntity(cam);
	}

	void LuaCamera::bind(sol::state_view& lua) noexcept
	{
		LuaViewport::bind(lua);
		LuaForwardRenderer::bind(lua);
		LuaLightingRenderComponent::bind(lua);

#ifdef PHYSICS_DEBUG_RENDER
		physics3d::LuaPhysicsDebugRenderer::bind(lua);
#endif

#ifdef DARMOK_OZZ
		LuaSkeletalAnimationRenderComponent::bind(lua);
#endif

		lua.new_usertype<Camera>("Camera", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<Camera>::value),
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
			"set_viewport_perspective", sol::overload(
				&LuaCamera::setViewportPerspective1,
				&LuaCamera::setViewportPerspective2
			),
			"set_viewport_ortho", sol::overload(
				&LuaCamera::setViewportOrtho1,
				&LuaCamera::setViewportOrtho2
			),
			"enabled", sol::property(&Camera::isEnabled, &Camera::setEnabled),
			"projection_matrix", sol::property(&Camera::getProjectionMatrix, &LuaCamera::setProjectionMatrix),
			"viewport", sol::property(&LuaCamera::getViewport, &LuaCamera::setViewport),
			"current_viewport", sol::property(&Camera::getCurrentViewport),
			"transform", sol::property(&LuaCamera::getTransform),
			"model_matrix", sol::property(&Camera::getModelMatrix),
			"screen_point_to_ray", sol::overload(&LuaCamera::screenPointToRay1, &LuaCamera::screenPointToRay2),
			"viewport_point_to_ray", &LuaCamera::viewportPointToRay,
			"world_to_screen_point", &LuaCamera::worldToScreenPoint,
			"world_to_viewport_point", &LuaCamera::worldToViewportPoint,
			"screen_to_world_point", &LuaCamera::screenToWorldPoint,
			"viewport_to_world_point", &LuaCamera::viewportToWorldPoint,
			"viewport_to_screen_point", &LuaCamera::viewportToScreenPoint,
			"screen_to_viewport_point", &LuaCamera::screenToViewportPoint,
			"render_graph", sol::resolve<RenderGraphDefinition&()>(&Camera::getRenderGraph),
			"render_chain", sol::resolve<RenderChain&()>(&Camera::getRenderChain)
		);
	}
}