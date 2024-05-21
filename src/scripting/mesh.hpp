#pragma once

#include <bgfx/bgfx.h>
#include <memory>
#include <vector>
#include <optional>
#include <darmok/optional_ref.hpp>
#include <darmok/scene_fwd.hpp>
#include <glm/glm.hpp>
#include <sol/sol.hpp>

namespace darmok
{
    class IMesh;
    class LuaMaterial;

	class LuaMesh final
	{
	public:
		LuaMesh(const std::shared_ptr<IMesh>& mesh) noexcept;

		std::string to_string() const noexcept;

		std::shared_ptr<IMesh> getReal() const noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<IMesh> _mesh;
	};

	struct MeshCreator;
	struct MeshData;
	struct MeshCreationConfig;
	struct Cube;
	struct Sphere;
	struct Rectangle;
	struct Line;
	struct Ray;
	class LuaTexture;
	class LuaRenderable;

	struct LuaMeshCreator final
	{
		using Config = MeshCreationConfig;

		LuaMeshCreator(const bgfx::VertexLayout& layout) noexcept;
		~LuaMeshCreator();

		Config& getConfig() noexcept;
		void setConfig(const Config& config) noexcept;
		bgfx::VertexLayout& getVertexLayout()  noexcept;

		LuaMesh createMesh(const MeshData& meshData) noexcept;
		LuaMesh createCube1() noexcept;
		LuaMesh createCube2(const Cube& cube) noexcept;
		LuaMesh createSphere1() noexcept;
		LuaMesh createSphere2(const Sphere& sphere) noexcept;
		LuaMesh createSphere3(int lod) noexcept;
		LuaMesh createSphere4(const Sphere& sphere, int lod) noexcept;
		LuaMesh createRectangle1() noexcept;
		LuaMesh createRectangle2(const Rectangle& rectangle) noexcept;
		LuaMesh createRectangle3(const glm::uvec2& size) noexcept;
		LuaMesh createLineRectangle1() noexcept;
		LuaMesh createLineRectangle2(const Rectangle& rectangle) noexcept;
		LuaMesh createRay(const Ray& ray) noexcept;
		LuaMesh createLine(const Line& line) noexcept;
		LuaMesh createLines(const std::vector<Line>& lines) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		std::shared_ptr<MeshCreator> _creator;
	};

	class Renderable;
	class LuaEntity;
	class LuaScene;

	class LuaRenderable final
	{
	public:
		LuaRenderable(Renderable& renderable) noexcept;

		const Renderable& getReal() const;
		Renderable& getReal();

		std::optional<LuaMesh> getMesh() const noexcept;
		LuaRenderable& setMesh(const LuaMesh& mesh) noexcept;
		LuaMaterial getMaterial() const noexcept;
		LuaRenderable& setMaterial(const LuaMaterial& material) noexcept;

		static void bind(sol::state_view& lua) noexcept;
	private:
		OptionalRef<Renderable> _renderable;

		static LuaRenderable addEntityComponent1(LuaEntity& entity, const LuaMesh& mesh) noexcept;
		static LuaRenderable addEntityComponent2(LuaEntity& entity, const LuaMaterial& material) noexcept;
		static LuaRenderable addEntityComponent3(LuaEntity& entity, const LuaMesh& mesh, const LuaMaterial& material) noexcept;
		static LuaRenderable addEntityComponent4(LuaEntity& entity, const LuaMesh& mesh, const LuaTexture& texture) noexcept;
		static std::optional<LuaRenderable> getEntityComponent(LuaEntity& entity) noexcept;
		std::optional<LuaEntity> getEntity(LuaScene& scene) noexcept;
	};
}