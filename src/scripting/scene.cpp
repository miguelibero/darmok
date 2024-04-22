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

	void LuaCamera::setProjection1(float fovy, float aspect, const VarVec2& range) noexcept
	{
		_camera->setProjection(fovy, aspect, LuaMath::tableToGlm(range));
	}

	void LuaCamera::setProjection2(float fovy, float aspect, float near) noexcept
	{
		_camera->setProjection(fovy, aspect, near);
	}

	void LuaCamera::setWindowProjection1(float fovy, const VarVec2& range) noexcept
	{
		_camera->setWindowProjection(fovy, LuaMath::tableToGlm(range));
	}

	void LuaCamera::setWindowProjection2(float fovy, float near) noexcept
	{
		_camera->setWindowProjection(fovy, near);
	}

	void LuaCamera::setWindowProjection3(float fovy) noexcept
	{
		_camera->setWindowProjection(fovy);
	}

	void LuaCamera::setOrtho1(const VarVec4& edges, const VarVec2& range, float offset) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(edges), LuaMath::tableToGlm(range), offset);
	}

	void LuaCamera::setOrtho2(const VarVec4& edges, const VarVec2& range) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(edges), LuaMath::tableToGlm(range));
	}

	void LuaCamera::setOrtho3(const VarVec4& edges) noexcept
	{
		_camera->setOrtho(LuaMath::tableToGlm(edges));
	}

	void LuaCamera::setWindowOrtho1(const VarVec2& range, float offset) noexcept
	{
		_camera->setWindowOrtho(LuaMath::tableToGlm(range), offset);
	}

	void LuaCamera::setWindowOrtho2(const VarVec2& range) noexcept
	{
		_camera->setWindowOrtho(LuaMath::tableToGlm(range));
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

	std::optional<Ray> LuaCamera::screenPointToRay(const VarVec2& point) const noexcept
	{
		return _camera->screenPointToRay(LuaMath::tableToGlm(point));
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

	void LuaPointLight::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaPointLight>("PointLight",
			sol::constructors<>()
		);
	}

	LuaAmbientLight::LuaAmbientLight(AmbientLight& light) noexcept
		: _light(light)
	{
	}

	void LuaAmbientLight::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaAmbientLight>("AmbientLight",
			sol::constructors<>()
		);
	}

	LuaMeshComponent::LuaMeshComponent(MeshComponent& comp) noexcept
		: _comp(comp)
	{
	}

	std::vector<LuaMesh> LuaMeshComponent::getMeshes() const noexcept
	{
		std::vector<LuaMesh> luaMeshes;
		auto realMeshes = _comp->getMeshes();
		luaMeshes.reserve(realMeshes.size());
		for (auto& mesh : realMeshes)
		{
			luaMeshes.push_back(LuaMesh(mesh));
		}
		return luaMeshes;
	}

	void LuaMeshComponent::setMeshes(const std::vector<LuaMesh>& meshes) noexcept
	{
		std::vector<std::shared_ptr<Mesh>> realMeshes;
		realMeshes.reserve(meshes.size());
		for (auto& mesh : meshes)
		{
			realMeshes.push_back(mesh.getReal());
		}
		_comp->setMeshes(realMeshes);
	}

	void LuaMeshComponent::setMesh(const LuaMesh& mesh) noexcept
	{
		_comp->setMesh(mesh.getReal());
	}

	void LuaMeshComponent::addMesh(const LuaMesh& mesh) noexcept
	{
		_comp->addMesh(mesh.getReal());
	}

	void LuaMeshComponent::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMeshComponent>("MeshComponent",
			sol::constructors<>(),
			"meshes", sol::property(&LuaMeshComponent::getMeshes, &LuaMeshComponent::setMeshes),
			"set_mesh", &LuaMeshComponent::setMesh,
			"add_mesh", &LuaMeshComponent::addMesh
		);
	}

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

	LuaEntity::LuaEntity(Entity entity, Scene& scene) noexcept
		: _entity(entity)
		, _scene(scene)
	{
	}

	EntityRegistry& LuaEntity::getRegistry() noexcept
	{
		return _scene->getRegistry();
	}

	const EntityRegistry& LuaEntity::getRegistry() const noexcept
	{
		return _scene->getRegistry();
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

	bool LuaEntity::hasLuaComponent(const std::string& name) const noexcept
	{
		auto table = getRegistry().try_get<LuaTableComponent>(_entity);
		return table != nullptr && table->hasEntry(name);
	}

	std::optional<LuaComponent> LuaEntity::tryGetLuaComponent(const std::string& name) noexcept
	{
		auto table = getRegistry().try_get<LuaTableComponent>(_entity);
		if (table == nullptr || !table->hasEntry(name))
		{
			return std::nullopt;
		}
		return LuaComponent(name, *table);
	}

	bool LuaEntity::removeLuaComponent(const std::string& name) noexcept
	{
		auto table = getRegistry().try_get<LuaTableComponent>(_entity);
		if (table == nullptr)
		{
			return false;
		}
		return table->removeEntry(name);
	}

	LuaComponent LuaEntity::getOrAddLuaComponent(const std::string& name) noexcept
	{
		auto& table = getRegistry().get_or_emplace<LuaTableComponent>(_entity);
		return LuaComponent(name, table);
	}

	const Entity& LuaEntity::getReal() const noexcept
	{
		return _entity;
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
			"has_component", sol::overload(&LuaEntity::hasNativeComponent<0>, &LuaEntity::hasLuaComponent)
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

	void LuaScene::configure(sol::state_view& lua) noexcept
	{
		LuaTransform::configure(lua);
		LuaCamera::configure(lua);
		LuaEntity::configure(lua);
		LuaAmbientLight::configure(lua);
		LuaPointLight::configure(lua);
		LuaMeshComponent::configure(lua);
		LuaComponent::configure(lua);

		lua.new_usertype<LuaScene>("Scene", sol::constructors<>(), 
			"create_entity",	&LuaScene::createEntity
		);
	}
}