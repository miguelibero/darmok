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
	LuaCamera::LuaCamera(Camera& cam, const std::weak_ptr<Scene>& scene) noexcept
		: _cam(cam)
		, _scene(scene)
	{
	}

	Camera& LuaCamera::getReal() noexcept
	{
		return _cam;
	}

	const Camera& LuaCamera::getReal() const noexcept
	{
		return _cam;
	}

	LuaCamera& LuaCamera::setPerspective1(float fovy, float aspect, float near, float far) noexcept
	{
		_cam.setPerspective(fovy, aspect, near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setPerspective2(float fovy, float aspect, float near) noexcept
	{
		_cam.setPerspective(fovy, aspect, near);
		return *this;
	}

	LuaCamera& LuaCamera::setPerspective3(float fovy, const VarLuaTable<glm::uvec2>& size, float near, float far) noexcept
	{
		_cam.setPerspective(fovy, LuaGlm::tableGet(size), near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setPerspective4(float fovy, const VarLuaTable<glm::uvec2>& size, float near) noexcept
	{
		_cam.setPerspective(fovy, LuaGlm::tableGet(size), near);
		return *this;
	}

	LuaCamera& LuaCamera::setViewportPerspective1(float fovy) noexcept
	{
		_cam.setViewportPerspective(fovy);
		return *this;
	}

	LuaCamera& LuaCamera::setViewportPerspective2(float fovy, float near, float far) noexcept
	{
		_cam.setViewportPerspective(fovy, near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho1(const VarViewport& vp, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		_cam.setOrtho(LuaViewport::tableGet(vp), LuaGlm::tableGet(center), near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho2(const VarViewport& vp, const VarLuaTable<glm::vec2>& center) noexcept
	{
		_cam.setOrtho(LuaViewport::tableGet(vp), LuaGlm::tableGet(center));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho3(const VarViewport& vp) noexcept
	{
		_cam.setOrtho(LuaViewport::tableGet(vp));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho4(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		_cam.setOrtho(LuaGlm::tableGet(size), LuaGlm::tableGet(center), near, far);
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho5(const VarLuaTable<glm::uvec2>& size, const VarLuaTable<glm::vec2>& center) noexcept
	{
		_cam.setOrtho(LuaGlm::tableGet(size), LuaGlm::tableGet(center));
		return *this;
	}

	LuaCamera& LuaCamera::setOrtho6(const VarLuaTable<glm::uvec2>& size) noexcept
	{
		_cam.setOrtho(LuaGlm::tableGet(size));
		return *this;
	}

	LuaCamera& LuaCamera::setViewportOrtho1(const VarLuaTable<glm::vec2>& center) noexcept
	{
		_cam.setViewportOrtho(LuaGlm::tableGet(center));
		return *this;
	}

	LuaCamera& LuaCamera::setViewportOrtho2(const VarLuaTable<glm::vec2>& center, float near, float far) noexcept
	{
		_cam.setViewportOrtho(LuaGlm::tableGet(center), near, far);
		return *this;
	}

	void LuaCamera::setViewport(std::optional<VarViewport> viewport) noexcept
	{
		_cam.setViewport(LuaViewport::tableGet(viewport));
	}

	std::optional<Viewport> LuaCamera::getViewport() noexcept
	{
		return _cam.getViewport();
	}

	Viewport LuaCamera::getCurrentViewport() noexcept
	{
		return _cam.getCurrentViewport();
	}

	OptionalRef<Transform>::std_t LuaCamera::getTransform() noexcept
	{
		return _cam.getTransform();
	}

	glm::mat4 LuaCamera::getModelMatrix() const noexcept
	{
		return _cam.getModelMatrix();
	}

	glm::mat4 LuaCamera::getModelInverse() const noexcept
	{
		return _cam.getModelInverse();
	}

	RenderGraphDefinition& LuaCamera::getRenderGraph() const noexcept
	{
		return _cam.getRenderGraph();
	}

	RenderChain& LuaCamera::getRenderChain() const noexcept
	{
		return _cam.getRenderChain();
	}

	const std::string& LuaCamera::getName() const noexcept
	{
		return _cam.getName();
	}

	void LuaCamera::setName(const std::string& name) noexcept
	{
		_cam.setName(name);
	}

	bool LuaCamera::getEnabled() const noexcept
	{
		return _cam.isEnabled();
	}

	void LuaCamera::setEnabled(bool enabled) noexcept
	{
		_cam.setEnabled(enabled);
	}

	const glm::mat4& LuaCamera::getProjectionMatrix() const noexcept
	{
		return _cam.getProjectionMatrix();
	}

	void LuaCamera::setProjectionMatrix(const VarLuaTable<glm::mat4>& matrix) noexcept
	{
		_cam.setProjectionMatrix(LuaGlm::tableGet(matrix));
	}

	Ray LuaCamera::screenPointToRay1(const glm::vec2& point) noexcept
	{
		return _cam.screenPointToRay(glm::vec3(point, 0));
	}

	Ray LuaCamera::screenPointToRay2(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.screenPointToRay(LuaGlm::tableGet(point));
	}

	Ray LuaCamera::viewportPointToRay(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.viewportPointToRay(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::worldToScreenPoint(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.worldToScreenPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::worldToViewportPoint(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.worldToViewportPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::screenToWorldPoint(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.screenToWorldPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::viewportToWorldPoint(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.viewportToWorldPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::viewportToScreenPoint(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.viewportToScreenPoint(LuaGlm::tableGet(point));
	}

	glm::vec3 LuaCamera::screenToViewportPoint(const VarLuaTable<glm::vec3>& point) noexcept
	{
		return _cam.screenToViewportPoint(LuaGlm::tableGet(point));
	}

	LuaCamera& LuaCamera::addEntityComponent(LuaEntity& entity) noexcept
	{
		auto& cam = entity.addComponent<Camera>();
		return entity.addComponent<LuaCamera>(cam, entity.getWeakScene());
	}

	OptionalRef<LuaCamera>::std_t LuaCamera::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<LuaCamera>();
	}

	std::optional<LuaEntity> LuaCamera::getEntity(const LuaCamera& cam, LuaScene& scene) noexcept
	{
		return scene.getEntity(cam);
	}

	void LuaCamera::setEntityFilter(const sol::object& filter) noexcept
	{
		_cam.setEntityFilter(std::make_unique<LuaEntityFilter>(filter, _scene));
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

		Scene::registerComponentDependency<Camera, LuaCamera>();

		lua.new_usertype<LuaCamera>("Camera", sol::no_constructor,
			"type_id", sol::property(&entt::type_hash<LuaCamera>::value),
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
			"name", sol::property(&LuaCamera::getName, &LuaCamera::setName),
			"enabled", sol::property(&LuaCamera::getEnabled, &LuaCamera::setEnabled),
			"projection_matrix", sol::property(&LuaCamera::getProjectionMatrix, &LuaCamera::setProjectionMatrix),
			"viewport", sol::property(&LuaCamera::getViewport, &LuaCamera::setViewport),
			"current_viewport", sol::property(&LuaCamera::getCurrentViewport),
			"transform", sol::property(&LuaCamera::getTransform),
			"model_matrix", sol::property(&LuaCamera::getModelMatrix),
			"model_inverse", sol::property(&LuaCamera::getModelInverse),
			"screen_point_to_ray", sol::overload(&LuaCamera::screenPointToRay1, &LuaCamera::screenPointToRay2),
			"viewport_point_to_ray", &LuaCamera::viewportPointToRay,
			"world_to_screen_point", &LuaCamera::worldToScreenPoint,
			"world_to_viewport_point", &LuaCamera::worldToViewportPoint,
			"screen_to_world_point", &LuaCamera::screenToWorldPoint,
			"viewport_to_world_point", &LuaCamera::viewportToWorldPoint,
			"viewport_to_screen_point", &LuaCamera::viewportToScreenPoint,
			"screen_to_viewport_point", &LuaCamera::screenToViewportPoint,
			"render_graph", sol::property(&LuaCamera::getRenderGraph),
			"render_chain", sol::property(&LuaCamera::getRenderChain),
			"entity_filter", sol::property(&LuaCamera::setEntityFilter)
		);
	}
}