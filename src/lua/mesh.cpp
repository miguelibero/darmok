#include "mesh.hpp"
#include "glm.hpp"
#include "utils.hpp"
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/shape.hpp>
#include <entt/entt.hpp>

namespace darmok
{
	void LuaMesh::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<IMesh>("Mesh", sol::no_constructor
		);

		LuaUtils::newEnum<Mesh::Type>(lua, "MeshType");
		LuaUtils::newEnum<MeshData::RectangleType>(lua, "RectangleMeshType");
		LuaUtils::newEnum<MeshData::LineType>(lua, "LineMeshType");

		lua.new_usertype<MeshData>("MeshData",
			sol::constructors<
				MeshData(),
				MeshData(const Cube&),
				MeshData(const Sphere&),
				MeshData(const Sphere&, int),
				MeshData(const Capsule&),
				MeshData(const Capsule&, int),
				MeshData(const Rectangle&),
				MeshData(const Rectangle&, MeshData::RectangleType),
				MeshData(const Ray&),
				MeshData(const Line&),
				MeshData(const Line&, MeshData::LineType),
				MeshData(const Triangle&)
			>(),
			"new_cube", sol::overload(
				[]() { return MeshData(Cube()); },
				[](const glm::vec3& size) { return MeshData(Cube(size)); }
			),
			"new_sphere", []() { return MeshData(Sphere()); },
			"new_capsule", []() { return MeshData(Capsule()); },
			"new_rectangle", sol::overload(
				[]() {
					return MeshData{ Rectangle{} };
				},
				[](const glm::uvec2& size) {
					return MeshData{ Rectangle{size} };
				},
				[](const glm::uvec2& size, MeshData::RectangleType type) {
					return MeshData{ Rectangle{size}, type };
				},
				[](const VarLuaTable<glm::vec2>& size) {
						return MeshData{ Rectangle{LuaGlm::tableGet(size)} };
				},
				[](const VarLuaTable<glm::vec2>& size, MeshData::RectangleType type) {
						return MeshData{ Rectangle{LuaGlm::tableGet(size)}, type };
				},
				[](MeshData::RectangleType type) {
					return MeshData{ Rectangle{}, type };
				}
			),
			"new_arrow", []() { return MeshData{ Line{}, Mesh::Definition::Arrow }; },
			"default_vertex_layout", sol::property(&MeshData::getDefaultVertexLayout),
			"create_mesh", sol::overload(
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout)
				{ 
					return std::shared_ptr<IMesh>(data.createMesh(vertexLayout));
				},
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout, const MeshConfig& config)
				{
					return std::shared_ptr<IMesh>(data.createMesh(vertexLayout, config));
				}
			),
			"subdivide_density", &MeshData::subdivideDensity,
			"subdivide", sol::overload(
				[](MeshData& data, size_t amount)
				{
					data.subdivide(amount);
				},
				[](MeshData& data)
				{
					data.subdivide();
				}
			)
		);
	}
}