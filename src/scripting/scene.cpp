#include "scene.hpp"
#include "asset.hpp"
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

	const glm::vec3& LuaTransform::getRotation() const noexcept
	{
		return _transform->getPosition();
	}

	const glm::vec3& LuaTransform::getScale() const noexcept
	{
		return _transform->getPosition();
	}

	const glm::vec3& LuaTransform::getPivot() const noexcept
	{
		return _transform->getPosition();
	}

	const glm::mat4& LuaTransform::getMatrix() const noexcept
	{
		return _transform->getMatrix();
	}

	const glm::mat4& LuaTransform::getInverse() const noexcept
	{
		return _transform->getInverse();
	}

	void LuaTransform::setPosition(const glm::vec3& v) noexcept
	{
		_transform->setPosition(v);
	}

	void LuaTransform::setRotation(const glm::vec3& v) noexcept
	{
		_transform->setRotation(v);
	}

	void LuaTransform::setScale(const glm::vec3& v) noexcept
	{
		_transform->setScale(v);
	}

	void LuaTransform::setPivot(const glm::vec3& v) noexcept
	{
		_transform->setPivot(v);
	}

	void LuaTransform::setMatrix(const glm::mat4& v) noexcept
	{
		_transform->setMatrix(v);
	}

	LuaCamera::LuaCamera(Camera& camera) noexcept
		: _camera(camera)
	{
	}

	void LuaCamera::setProjection(float fovy, const glm::uvec2& size, float near, float far) noexcept
	{
		_camera->setProjection(fovy, size, near, far);
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
	void LuaCamera::setOrtho(const glm::vec4& edges, const glm::vec2& range, float offset) noexcept
	{
		_camera->setOrtho(edges, range, offset);
	}

	std::optional<Ray> LuaCamera::screenPointToRay(const glm::vec2& point) const noexcept
	{
		return _camera->screenPointToRay(point);
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

	LuaTransform LuaEntity::addTransformComponent() noexcept
	{
		return LuaTransform(getRegistry().emplace<Transform>(_entity));
	}

	LuaCamera LuaEntity::addCameraComponent() noexcept
	{
		return LuaCamera(getRegistry().emplace<Camera>(_entity));
	}

	LuaPointLight LuaEntity::addPointLightComponent() noexcept
	{
		return LuaPointLight(getRegistry().emplace<PointLight>(_entity));
	}

	LuaMeshComponent LuaEntity::addMeshComponent(const LuaMesh& mesh) noexcept
	{
		return LuaMeshComponent(getRegistry().emplace<MeshComponent>(_entity, mesh.getReal()));
	}

	const Entity& LuaEntity::getReal() const noexcept
	{
		return _entity;
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

	void LuaTransform::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaTransform>("Transform", sol::constructors<>(),
			"position", sol::property(&LuaTransform::getPosition, &LuaTransform::setPosition),
			"rotation", sol::property(&LuaTransform::getRotation, &LuaTransform::setRotation),
			"scale", sol::property(&LuaTransform::getScale, &LuaTransform::setScale),
			"pivot", sol::property(&LuaTransform::getPivot, &LuaTransform::setPivot),
			"matrix", sol::property(&LuaTransform::getMatrix, &LuaTransform::setMatrix),
			"inverse", sol::property(&LuaTransform::getInverse),
			"parent", sol::property(&LuaTransform::getParent, &LuaTransform::setParent)
		);
	}

	void LuaCamera::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaCamera>("Camera", sol::constructors<>(),
			"set_projection", &LuaCamera::setProjection,
			"set_ortho", &LuaCamera::setOrtho,
			"set_forward_phong_renderer", &LuaCamera::setForwardPhongRenderer,
			"matrix", sol::property(&LuaCamera::getMatrix, &LuaCamera::setMatrix),
			"screen_point_to_ray", &LuaCamera::screenPointToRay
		);
	}

	void LuaEntity::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaEntity>("Entity", sol::constructors<>(),
			"add_component", &LuaEntity::addComponent,
			"add_camera_component", &LuaEntity::addCameraComponent,
			"add_transform_component", &LuaEntity::addTransformComponent,
			"add_point_light_component", &LuaEntity::addPointLightComponent,
			"add_mesh_component", &LuaEntity::addMeshComponent
		);
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