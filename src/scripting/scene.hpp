#pragma once

#include <memory>
#include <optional>
#include <variant>
#include <unordered_map>
#include <glm/glm.hpp>
#include <bx/bx.h>
#include <darmok/scene.hpp>
#include <darmok/optional_ref.hpp>
#include "sol.hpp"

namespace darmok
{
	enum class LuaNativeComponentType
	{
		Transform,
		Camera,
		AmbientLight,
		PointLight,
		Mesh
	};

    class Transform;

    class LuaTransform final
	{
	public:
		using native_t = Transform;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::Transform;

		LuaTransform(Transform& transform) noexcept;
		std::optional<LuaTransform> getParent() noexcept;
		void setParent(std::optional<LuaTransform> parent) noexcept;

		const glm::vec3& getPosition() const noexcept;
		const glm::quat& getRotation() const noexcept;
		glm::vec3 getEulerAngles() const noexcept;
		glm::vec3 getForward() const noexcept;
		glm::vec3 getRight() const noexcept;
		glm::vec3 getUp() const noexcept;
		const glm::vec3& getScale() const noexcept;
		const glm::vec3& getPivot() const noexcept;
		const glm::mat4& getMatrix() const noexcept;
		const glm::mat4& getInverse() const noexcept;

		using VarVec3 = std::variant<glm::vec3, sol::table>;
		using VarQuat = std::variant<glm::quat, sol::table>;

		void setPosition(const VarVec3& v) noexcept;
		void setRotation(const VarQuat& v) noexcept;
		void setEulerAngles(const VarVec3& v) noexcept;
		void setForward(const VarVec3& v) noexcept;
		void setScale(const VarVec3& v) noexcept;
		void setPivot(const VarVec3& v) noexcept;
		void setMatrix(const glm::mat4& v) noexcept;

		void lookDir1(const VarVec3& v) noexcept;
		void lookDir2(const VarVec3& v, const VarVec3& up) noexcept;
		void lookAt1(const VarVec3& v) noexcept;
		void lookAt2(const VarVec3& v, const VarVec3& up) noexcept;

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
		using native_t = Camera;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::Camera;

		using VarVec2 = std::variant<glm::vec2, sol::table>;
		using VarVec4 = std::variant<glm::vec4, sol::table>;

		LuaCamera(Camera& camera) noexcept;
		void setProjection1(float fovy, float aspect, const VarVec2& range) noexcept;
		void setProjection2(float fovy, float aspect, float near) noexcept;
		void setWindowProjection1(float fovy, const VarVec2& range) noexcept;
		void setWindowProjection2(float fovy, float near) noexcept;
		void setWindowProjection3(float fovy) noexcept;
		void setOrtho1(const VarVec4& edges, const VarVec2& range, float offset) noexcept;
		void setOrtho2(const VarVec4& edges, const VarVec2& range) noexcept;
		void setOrtho3(const VarVec4& edges) noexcept;
		void setWindowOrtho1(const VarVec2& range, float offset) noexcept;
		void setWindowOrtho2(const VarVec2& range) noexcept;
		void setWindowOrtho3() noexcept;

		void setForwardPhongRenderer(const LuaProgram& program) noexcept;
		const glm::mat4& getMatrix() const noexcept;
		void setMatrix(const glm::mat4& matrix) noexcept;
		std::optional<Ray> screenPointToRay(const VarVec2& point) const noexcept;

		static void configure(sol::state_view& lua) noexcept;

	private:
		OptionalRef<Camera> _camera;
	};

    class PointLight;

	class LuaPointLight final
	{
	public:
		using native_t = PointLight;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::PointLight;

		LuaPointLight(PointLight& light) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<PointLight> _light;
	};

	class AmbientLight;

	class LuaAmbientLight final
	{
	public:
		using native_t = AmbientLight;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::AmbientLight;

		LuaAmbientLight(AmbientLight& light) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<AmbientLight> _light;
	};

    class MeshComponent;
	class LuaMesh;

	class LuaMeshComponent final
	{
	public:
		using native_t = MeshComponent;
		const static LuaNativeComponentType native_type = LuaNativeComponentType::Mesh;

		LuaMeshComponent(MeshComponent& comp) noexcept;
		std::vector<LuaMesh> getMeshes() const noexcept;
		void setMeshes(const std::vector<LuaMesh>& meshes) noexcept;
		void setMesh(const LuaMesh& mesh) noexcept;
		void addMesh(const LuaMesh& mesh) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		OptionalRef<MeshComponent> _comp;
	};

	class LuaTableComponent final
	{
	public:
		void addEntry(const std::string& name, const sol::table& data);
		void setEntry(const std::string& name, const sol::table& data) noexcept;
		const sol::table& getEntry(const std::string& name) const;
		sol::table& getEntry(const std::string& name);
		bool hasEntry(const std::string& name) const noexcept;
		bool removeEntry(const std::string& name) noexcept;

	private:
		std::unordered_map<std::string, sol::table> _tables;
	};

	class LuaComponent final
	{
	public:
		LuaComponent(const std::string& name, LuaTableComponent& table) noexcept;

		const std::string& getName() const noexcept;
		sol::table& getData();
		void setData(const sol::table& data) noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		std::string _name;
		LuaTableComponent& _table;
	};

    class Scene;
	class LuaMesh;

	using LuaNativeComponent = std::variant<LuaTransform, LuaCamera, LuaPointLight, LuaMeshComponent>;

	/*
	template<LuaNativeComponentType T>
	struct LuaNativeComponentOperations final : public ILuaNativeComponentOperations
	{
		EntityRegistry& registry;
		Entity entity;

		lua_t add()
		{
			return lua_t(registry.emplace<native_t>(_entity));
		}
		
		bool remove() noexcept
		{
			return registry.remove<native_t>(_entity) > 0;
		}

		lua_t get()
		{
			return lua_t(registry.get<native_t>(_entity));
		}

		lua_t getOrAdd() noexcept
		{
			return lua_t(registry.get_or_emplace<native_t>(_entity));
		}

		bool has() const noexcept
		{
			return registry.try_get<native_t>(_entity) != nullptr;
		}

		std::optional<lua_t> tryGet() noexcept
		{
			auto comp = registry.try_get<native_t>(_entity);
			if (comp == nullptr)
			{
				return std::nullopt;
			}
			return lua_t(*comp);
		}
	};*/

	class LuaEntity final
	{
	public:
		LuaEntity(Entity entity, Scene& scene) noexcept;
		LuaComponent addLuaComponent(const std::string& name, const sol::table& data);
		LuaComponent getLuaComponent(const std::string& name);
		bool removeLuaComponent(const std::string& name) noexcept;
		LuaComponent getOrAddLuaComponent(const std::string& name) noexcept;
		std::optional<LuaComponent> tryGetLuaComponent(const std::string& name) noexcept;
		bool hasLuaComponent(const std::string& name) const noexcept;

		template <std::size_t I = 0>
		LuaNativeComponent addNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return addNativeComponent<I + 1>(type);
				}
				return lua_t(getRegistry().emplace<typename lua_t::native_t>(_entity));
			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		LuaNativeComponent getNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return getNativeComponent<I + 1>(type);
				}
				return lua_t(getRegistry().get<typename lua_t::native_t>(_entity));
			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		LuaNativeComponent getOrAddNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return getOrAddNativeComponent<I + 1>(type);
				}
				return lua_t(getRegistry().get_or_emplace<typename lua_t::native_t>(_entity));
			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		std::optional<LuaNativeComponent> tryGetNativeComponent(LuaNativeComponentType type)
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return tryGetNativeComponent<I + 1>(type);
				}
				auto comp = getRegistry().try_get<typename lua_t::native_t>(_entity);
				if (comp == nullptr)
				{
					return std::nullopt;
				}
				return lua_t(*comp);

			}
			throw std::runtime_error("missing native component");
		}

		template <std::size_t I = 0>
		bool removeNativeComponent(LuaNativeComponentType type) noexcept
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return removeNativeComponent<I + 1>(type);
				}
				return getRegistry().remove<typename lua_t::native_t>(_entity) > 0;
			}
			return false;
		}

		template <std::size_t I = 0>
		bool hasNativeComponent(LuaNativeComponentType type) const noexcept
		{
			if constexpr (I < std::variant_size_v<LuaNativeComponent>)
			{
				using lua_t = std::variant_alternative_t<I, LuaNativeComponent>;
				if (lua_t::native_type != type)
				{
					return hasNativeComponent<I + 1>(type);
				}
				return getRegistry().try_get<typename lua_t::native_t>(_entity) != nullptr;
			}
			return false;
		}

		const Entity& getReal() const noexcept;

		static void configure(sol::state_view& lua) noexcept;
	private:
		Entity _entity;
		OptionalRef<Scene> _scene;

		EntityRegistry& getRegistry() noexcept;
		const EntityRegistry& getRegistry() const noexcept;




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