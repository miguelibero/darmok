#include "camera.hpp"
#include "scene.hpp"
#include "scene_filter.hpp"
#include "render_forward.hpp"
#include "light.hpp"
#include "culling.hpp"
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/camera.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/render_chain.hpp>
#include <darmok/scene_filter.hpp>
#include <darmok/light.hpp>
#include <darmok/transform.hpp>

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
	Camera& LuaCamera::setPerspective1(Camera& cam, float fovy) noexcept
	{
		return cam.setPerspective(fovy);
	}

	Camera& LuaCamera::setPerspective2(Camera& cam, float fovy, float near, float far) noexcept
	{
		return cam.setPerspective(fovy, near, far);
	}

	Camera& LuaCamera::setOrtho1(Camera& cam, const VarLuaTable<glm::vec2>& center) noexcept
	{
		return cam.setOrtho(LuaGlm::tableGet(center));
	}

	Camera& LuaCamera::setOrtho2(Camera& cam, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		return cam.setOrtho(LuaGlm::tableGet(center), near, far);
	}

	std::optional<Viewport> LuaCamera::getViewport(const Camera& cam) noexcept
	{
		return cam.getViewport();
	}

	void LuaCamera::setViewport(Camera& cam, std::optional<VarViewport> viewport) noexcept
	{
		cam.setViewport(LuaViewport::tableGet(viewport));
	}

	OptionalRef<Transform>::std_t LuaCamera::getTransform(const Camera& cam) noexcept
	{
		return cam.getTransform();
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

	bool LuaCamera::isWorldPointVisible(const Camera& cam, const VarLuaTable<glm::vec3>& point) noexcept
	{
		return cam.isWorldPointVisible(LuaGlm::tableGet(point));
	}

	Camera& LuaCamera::addEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.addComponent<Camera>();
	}

	OptionalRef<Camera>::std_t LuaCamera::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Camera>();
	}

	std::optional<LuaEntity> LuaCamera::getEntity(const Camera& cam, const std::shared_ptr<Scene>& scene) noexcept
	{
		return LuaScene::getEntity(scene, cam);
	}

	void LuaCamera::setCullingFilter(Camera& cam, const sol::object& filter) noexcept
	{
		cam.setCullingFilter(LuaEntityFilter::create(filter));
	}

	void LuaCamera::bind(sol::state_view& lua) noexcept
	{
		LuaViewport::bind(lua);
		LuaForwardRenderer::bind(lua);
		LuaLightingRenderComponent::bind(lua);
		LuaOcclusionCuller::bind(lua);
		LuaFrustumCuller::bind(lua);
		LuaCullingDebugRenderer::bind(lua);

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
				&LuaCamera::setPerspective2
			),
			"set_ortho", sol::overload(
				&LuaCamera::setOrtho1,
				&LuaCamera::setOrtho2
			),
			"name", sol::property(&Camera::getName, &Camera::setName),
			"enabled", sol::property(&Camera::isEnabled, &Camera::setEnabled),
			"projection_matrix", sol::property(&Camera::getProjectionMatrix),
			"projection_inverse", sol::property(&Camera::getProjectionInverse),
			"viewport", sol::property(&LuaCamera::getViewport, &LuaCamera::setViewport),
			"current_viewport", sol::property(&Camera::getCurrentViewport),
			"transform", sol::property(&LuaCamera::getTransform),
			"view_matrix", sol::property(&Camera::getViewMatrix),
			"view_inverse", sol::property(&Camera::getViewInverse),
			"view_projection_matrix", sol::property(&Camera::getViewProjectionMatrix),
			"view_projection_inverse", sol::property(&Camera::getViewProjectionInverse),
			"screen_point_to_ray", sol::overload(&LuaCamera::screenPointToRay1, &LuaCamera::screenPointToRay2),
			"viewport_point_to_ray", &LuaCamera::viewportPointToRay,
			"world_to_screen_point", &LuaCamera::worldToScreenPoint,
			"world_to_viewport_point", &LuaCamera::worldToViewportPoint,
			"screen_to_world_point", &LuaCamera::screenToWorldPoint,
			"viewport_to_world_point", &LuaCamera::viewportToWorldPoint,
			"viewport_to_screen_point", &LuaCamera::viewportToScreenPoint,
			"screen_to_viewport_point", &LuaCamera::screenToViewportPoint,
			"is_world_point_visible", &LuaCamera::isWorldPointVisible,
			"render_chain", sol::property(
				sol::resolve<RenderChain&()>(&Camera::getRenderChain)
			),
			"culling_filter", sol::property(&Camera::getCullingFilter, &LuaCamera::setCullingFilter)
		);
	}
}