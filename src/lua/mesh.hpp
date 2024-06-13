#pragma once

#include <bgfx/bgfx.h>
#include <memory>
#include <vector>
#include <optional>
#include <darmok/glm.hpp>
#include <sol/sol.hpp>

namespace darmok
{
    class IMesh;

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
	struct Cuboid;
	struct Sphere;
	struct Rectangle;
	struct Line;
	struct Ray;

	struct LuaMeshCreator final
	{
		using Config = MeshCreationConfig;

		LuaMeshCreator() noexcept;
		LuaMeshCreator(const std::optional<bgfx::VertexLayout>& layout) noexcept;
		~LuaMeshCreator() noexcept;

		Config& getConfig() noexcept;
		void setConfig(const Config& config) noexcept;
		void setVertexLayout(const std::optional<bgfx::VertexLayout>& layout) noexcept;
		std::optional<bgfx::VertexLayout> getVertexLayout() noexcept;

		LuaMesh createMesh(const MeshData& meshData) noexcept;
		LuaMesh createCuboid1() noexcept;
		LuaMesh createCuboid2(const Cuboid& cuboid) noexcept;
		LuaMesh createSphere1() noexcept;
		LuaMesh createSphere2(const Sphere& sphere) noexcept;
		LuaMesh createSphere3(int lod) noexcept;
		LuaMesh createSphere4(const Sphere& sphere, int lod) noexcept;
		LuaMesh createSphere5(const glm::vec3& origin) noexcept;
		LuaMesh createSphere6(const glm::vec3& origin, int lod) noexcept;
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
}