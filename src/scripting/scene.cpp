#include "scene.hpp"
#include "asset.hpp"
#include "math.hpp"
#include <darmok/camera.hpp>
#include <darmok/light.hpp>
#include <darmok/mesh.hpp>
#include <darmok/render_forward.hpp>
#include <darmok/scene.hpp>
#include <darmok/transform.hpp>

namespace darmok
{
    LuaTransform::LuaTransform(Transform& transform) noexcept
		: _transform(transform)
	{
	}

	std::optional<LuaTransform> LuaTransform::getParent() noexcept
	{
		auto parent = _transform->getParent();
		if (parent)
		{
			return LuaTransform(parent.value());
		}
		return std::nullopt;
	}

	void LuaTransform::setParent(std::optional<LuaTransform> parent) noexcept
	{
		OptionalRef<Transform> p = nullptr;
		if (parent.has_value())
		{
			p = parent.value()._transform;
		}
		_transform->setParent(p);
	}

	const glm::vec3& LuaTransform::getPosition() const noexcept
	{
		return _transform->getPosition();
	}

	const glm::quat& LuaTransform::getRotation() const noexcept
	{
		return _transform->getRotation();
	}

	glm::vec3 LuaTransform::getEulerAngles() const noexcept
	{
		return glm::degrees(glm::eulerAngles(getRotation()));
	}

	glm::vec3 LuaTransform::getForward() const noexcept
	{
		static const glm::vec3 dir(0, 0, 1);
		return _transform->getRotation() * dir;
	}

	glm::vec3 LuaTransform::getRight() const noexcept
	{
		static const glm::vec3 dir(1, 0, 0);
		return _transform->getRotation() * dir;
	}

	glm::vec3 LuaTransform::getUp() const noexcept
	{
		static const glm::vec3 dir(0, 1, 0);
		return _transform->getRotation() * dir;
	}

	const glm::vec3& LuaTransform::getScale() const noexcept
	{
		return _transform->getScale();
	}

	const glm::vec3& LuaTransform::getPivot() const noexcept
	{
		return _transform->getPivot();
	}

	const glm::mat4& LuaTransform::getMatrix() const noexcept
	{
		return _transform->getMatrix();
	}

	const glm::mat4& LuaTransform::getInverse() const noexcept
	{
		return _transform->getInverse();
	}

	void LuaTransform::setPosition(const VarVec3& v) noexcept
	{
		_transform->setPosition(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setEulerAngles(const VarVec3& v) noexcept
	{
		_transform->setRotation(glm::quat(glm::radians(LuaMath::tableToGlm(v))));
	}

	void LuaTransform::setRotation(const VarQuat& v) noexcept
	{
		_transform->setRotation(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setForward(const VarVec3& v) noexcept
	{
		_transform->setForward(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setScale(const VarVec3& v) noexcept
	{
		_transform->setScale(LuaMath::tableToGlm(v));
	}

	void LuaTransform::setPivot(const VarVec3& v) noexcept
	{
		_transform->setPivot(LuaMath::tableToGlm(v));
	}

	void LuaTransform::lookDir1(const VarVec3& v) noexcept
	{
		_transform->lookDir(LuaMath::tableToGlm(v));
	}

	void LuaTransform::lookDir2(const VarVec3& v, const VarVec3& up) noexcept
	{
		_transform->lookDir(LuaMath::tableToGlm(v), LuaMath::tableToGlm(up));
	}

	void LuaTransform::lookAt1(const VarVec3& v) noexcept
	{
		_transform->lookAt(LuaMath::tableToGlm(v));
	}

	void LuaTransform::lookAt2(const VarVec3& v, const VarVec3& up) noexcept
	{
		_transform->lookAt(LuaMath::tableToGlm(v), LuaMath::tableToGlm(up));
	}

	void LuaTransform::setMatrix(const glm::mat4& v) noexcept
	{
		_transform->setMatrix(v);
	}

	void LuaTransform::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaTransform>("Transform", sol::constructors<>(),
			"position", sol::property(&LuaTransform::getPosition, &LuaTransform::setPosition),
			"rotation", sol::property(&LuaTransform::getRotation, &LuaTransform::setRotation),
			"euler_angles", sol::property(&LuaTransform::getEulerAngles, &LuaTransform::setEulerAngles),
			"forward", sol::property(&LuaTransform::getForward),
			"right", sol::property(&LuaTransform::getRight),
			"up", sol::property(&LuaTransform::getUp),
			"scale", sol::property(&LuaTransform::getScale, &LuaTransform::setScale),
			"pivot", sol::property(&LuaTransform::getPivot, &LuaTransform::setPivot),
			"matrix", sol::property(&LuaTransform::getMatrix, &LuaTransform::setMatrix),
			"inverse", sol::property(&LuaTransform::getInverse),
			"parent", sol::property(&LuaTransform::getParent, &LuaTransform::setParent),
			"look_dir", sol::overload(&LuaTransform::lookDir1, &LuaTransform::lookDir2),
			"look_at", sol::overload(&LuaTransform::lookAt1, &LuaTransform::lookAt2)
		);
	}

	LuaCamera::LuaCamera(Camera& camera) noexcept
		: _camera(camera)
	{
	}

	void LuaCamera::setProjection1(float fovy, float aspect, const glm::vec2& range) noexcept
	{
		_camera->setProjection(fovy, aspect, range);
	}

	void LuaCamera::setProjection2(float fovy, float aspect, float near) noexcept
	{
		_camera->setProjection(fovy, aspect, near);
	}

	void LuaCamera::setWindowProjection1(float fovy, const glm::vec2& range) noexcept
	{
		_camera->setWindowProjection(fovy, range);
	}

	void LuaCamera::setWindowProjection2(float fovy, float near) noexcept
	{
		_camera->setWindowProjection(fovy, near);
	}

	void LuaCamera::setWindowProjection3(float fovy) noexcept
	{
		_camera->setWindowProjection(fovy);
	}

	void LuaCamera::setOrtho1(const glm::vec4& edges, const glm::vec2& range, float offset) noexcept
	{
		_camera->setOrtho(edges, range, offset);
	}

	void LuaCamera::setOrtho2(const glm::vec4& edges, const glm::vec2& range) noexcept
	{
		_camera->setOrtho(edges, range);
	}

	void LuaCamera::setOrtho3(const glm::vec4& edges) noexcept
	{
		_camera->setOrtho(edges);
	}

	void LuaCamera::setWindowOrtho1(const glm::vec2& range, float offset) noexcept
	{
		_camera->setWindowOrtho(range, offset);
	}

	void LuaCamera::setWindowOrtho2(const glm::vec2& range) noexcept
	{
		_camera->setWindowOrtho(range);
	}

	void LuaCamera::setWindowOrtho3() noexcept
	{
		_camera->setWindowOrtho();
	}

	void LuaCamera::setForwardPhongRenderer(const LuaProgram& program) noexcept
	{
		_camera->setRenderer<ForwardRenderer>(program.getReal(), _camera->addComponent<PhongLightingComponent>());
	}

	const glm::mat4& LuaCamera::getMatrix() const noexcept
	{
		return _camera->getMatrix();
	}

	void LuaCamera::setMatrix(const glm::mat4& matrix) noexcept
	{
		_camera->setMatrix(matrix);
	}

	std::optional<Ray> LuaCamera::screenPointToRay(const glm::vec2& point) const noexcept
	{
		return _camera->screenPointToRay(point);
	}

	void LuaCamera::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaCamera>("Camera", sol::constructors<>(),
			"set_projection", sol::overload(
				&LuaCamera::setProjection1,
				&LuaCamera::setProjection2,
				&LuaCamera::setWindowProjection1,
				&LuaCamera::setWindowProjection2,
				&LuaCamera::setWindowProjection3
			),
			"set_ortho", sol::overload(
				&LuaCamera::setOrtho1,
				&LuaCamera::setOrtho2,
				&LuaCamera::setOrtho3,
				&LuaCamera::setWindowOrtho1,
				&LuaCamera::setWindowOrtho2,
				&LuaCamera::setWindowOrtho3
			),
			"set_forward_phong_renderer", &LuaCamera::setForwardPhongRenderer,
			"matrix", sol::property(&LuaCamera::getMatrix, &LuaCamera::setMatrix),
			"screen_point_to_ray", &LuaCamera::screenPointToRay
		);
	}

	LuaPointLight::LuaPointLight(PointLight& light) noexcept
		: _light(light)
	{
	}

	LuaMeshComponent::LuaMeshComponent(MeshComponent& comp) noexcept
		: _comp(comp)
	{
	}

	LuaComponent::LuaComponent(LuaInternalComponent& comp) noexcept
		: _comp(comp)
	{
	}

	LuaInternalComponent::LuaInternalComponent(const sol::table& table) noexcept
		: _table(table)
	{
	}

	LuaEntity::LuaEntity(Entity entity, Scene& scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
	}

	EntityRegistry& LuaEntity::getRegistry() noexcept
	{
		return _scene->getRegistry();
	}

	LuaComponent LuaEntity::addComponent(const sol::table& table) noexcept
	{
		return LuaComponent(getRegistry().emplace<LuaInternalComponent>(_entity, table));
	}

	LuaTransform LuaEntity::getTransform() noexcept
	{
		return LuaTransform(getRegistry().get_or_emplace<Transform>(_entity));
	}

	LuaCamera LuaEntity::getCamera() noexcept
	{
		return LuaCamera(getRegistry().get_or_emplace<Camera>(_entity));
	}

	LuaPointLight LuaEntity::getPointLight() noexcept
	{
		return LuaPointLight(getRegistry().get_or_emplace<PointLight>(_entity));
	}

	LuaMeshComponent LuaEntity::addMesh(const LuaMesh& mesh) noexcept
	{
		auto& registry = getRegistry();
		auto comp = registry.try_get<MeshComponent>(_entity);
		if (comp != nullptr)
		{
			comp->addMesh(mesh.getReal());
			return LuaMeshComponent(*comp);
		}
		return LuaMeshComponent(getRegistry().emplace<MeshComponent>(_entity, mesh.getReal()));
	}

	const Entity& LuaEntity::getReal() const noexcept
	{
		return _entity;
	}

	void LuaEntity::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEntity>("Entity", sol::constructors<>(),
			"add_component", &LuaEntity::addComponent,
			"get_camera", &LuaEntity::getCamera,
			"get_transform", &LuaEntity::getTransform,
			"get_point_light", &LuaEntity::getPointLight,
			"add_mesh", &LuaEntity::addMesh
		);
	}

	LuaScene::LuaScene(Scene& scene) noexcept
		: _scene(scene)
	{
	}

	EntityRegistry& LuaScene::getRegistry() noexcept
	{
		return _scene->getRegistry();
	}

	LuaEntity LuaScene::createEntity() noexcept
	{
		return LuaEntity(getRegistry().create(), _scene.value());
	}

	const Scene& LuaScene::getReal() const noexcept
	{
		return _scene.value();
	}

	Scene& LuaScene::getReal() noexcept
	{
		return _scene.value();
	}

	void LuaPointLight::configure(sol::state_view& lua) noexcept
	{
	}

	void LuaMeshComponent::configure(sol::state_view& lua) noexcept
	{
	}

	void LuaComponent::configure(sol::state_view& lua) noexcept
	{
	}

	void LuaScene::configure(sol::state_view& lua) noexcept
	{
		LuaTransform::configure(lua);
		LuaCamera::configure(lua);
		LuaEntity::configure(lua);
		LuaPointLight::configure(lua);
		LuaMeshComponent::configure(lua);
		LuaComponent::configure(lua);

		lua.new_usertype<LuaScene>("Scene", sol::constructors<>(), 
			"create_entity",	&LuaScene::createEntity
		);
	}
}