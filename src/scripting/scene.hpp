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
		void setProjection(float fovy, const glm::uvec2& size, float near, float far) noexcept;
		void setForwardPhongRenderer(const LuaProgram& program) noexcept;
		const glm::mat4& getMatrix() const noexcept;
		void setMatrix(const glm::mat4& matrix) noexcept;
		void setOrtho(const glm::vec4& edges, const glm::vec2& range = glm::vec2(0.f, bx::kFloatLargest), float offset = 0.f) noexcept;
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