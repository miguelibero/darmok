#pragma once

#include <memory>
#include <optional>
#include <glm/glm.hpp>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include "sol.hpp"

namespace darmok
{
    class Transform;

    class LuaTransform final
	{
	public:
		LuaTransform(Transform& transform) noexcept;
		std::optional<LuaTransform> getParent() noexcept;
		void setParent(std::optional<LuaTransform> parent) noexcept;

		const glm::vec3& getPosition() const noexcept;
		const glm::vec3& getRotation() const noexcept;
		const glm::vec3& getScale() const noexcept;
		const glm::vec3& getPivot() const noexcept;
		const glm::mat4& getMatrix() const noexcept;
		const glm::mat4& getInverse() const noexcept;

		void setPosition(const glm::vec3& v) noexcept;
		void setRotation(const glm::vec3& v) noexcept;
		void setScale(const glm::vec3& v) noexcept;
		void setPivot(const glm::vec3& v) noexcept;
		void setMatrix(const glm::mat4& v) noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Transform> _transform;
	};

    class Camera;
	class LuaProgram;
	class Ray;

	class LuaCamera final
	{
	public:
		LuaCamera(Camera& camera) noexcept;
		void setProjection1(float fovy, float aspect, const glm::vec2& range) noexcept;
		void setProjection2(float fovy, float aspect, float near) noexcept;
		void setWindowProjection1(float fovy, const glm::vec2& range) noexcept;
		void setWindowProjection2(float fovy, float near) noexcept;
		void setOrtho1(const glm::vec4& edges, const glm::vec2& range, float offset) noexcept;
		void setOrtho2(const glm::vec4& edges, const glm::vec2& range) noexcept;
		void setOrtho3(const glm::vec4& edges) noexcept;
		void setWindowOrtho1(const glm::vec2& range, float offset) noexcept;
		void setWindowOrtho2(const glm::vec2& range) noexcept;
		void setWindowOrtho3() noexcept;

		void setForwardPhongRenderer(const LuaProgram& program) noexcept;
		const glm::mat4& getMatrix() const noexcept;
		void setMatrix(const glm::mat4& matrix) noexcept;
		std::optional<Ray> screenPointToRay(const glm::vec2& point) const noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Camera> _camera;
	};

    class PointLight;

	class LuaPointLight final
	{
	public:
		LuaPointLight(PointLight& light) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<PointLight> _light;
	};

    class MeshComponent;

	class LuaMeshComponent final
	{
	public:
		LuaMeshComponent(MeshComponent& comp) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<MeshComponent> _comp;
	};

	class LuaInternalComponent final
	{
	public:
		LuaInternalComponent(const sol::table& table) noexcept;
	private:
		sol::table _table;
	};

	class LuaComponent final
	{
	public:
		LuaComponent(LuaInternalComponent& comp) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<LuaInternalComponent> _comp;
	};

    class Scene;
	class LuaMesh;

	class LuaEntity final
	{
	public:
		LuaEntity(Entity entity, Scene& scene) noexcept;
		LuaComponent addComponent(const sol::table& table) noexcept;
		LuaTransform addTransformComponent() noexcept;
		LuaCamera addCameraComponent() noexcept;
		LuaPointLight addPointLightComponent() noexcept;
		LuaMeshComponent addMeshComponent(const LuaMesh& mesh) noexcept;
		const Entity& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		Entity _entity;
		OptionalRef<Scene> _scene;

		EntityRegistry& getRegistry() noexcept;
	};

	class LuaScene final
	{
	public:
		LuaScene(Scene& scene) noexcept;
		EntityRegistry& getRegistry() noexcept;
		LuaEntity createEntity() noexcept;
		const Scene& getReal() const noexcept;
		Scene& getReal() noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Scene> _scene;
	};
}