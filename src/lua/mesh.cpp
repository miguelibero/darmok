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

		LuaUtils::newEnum<MeshData::MeshType::Enum>(lua, "MeshType");
		LuaUtils::newEnum<MeshData::RectangleMeshType::Enum>(lua, "RectangleMeshType");
		LuaUtils::newEnum<MeshData::LineMeshType::Enum>(lua, "LineMeshType");

		lua.new_usertype<MeshData>("MeshData",
			sol::constructors<
				MeshData(),
				MeshData(const Cube&),
				MeshData(const Sphere&),
				MeshData(const Sphere&, int),
				MeshData(const Capsule&),
				MeshData(const Capsule&, int),
				MeshData(const Rectangle&),
				MeshData(const Rectangle&, MeshData::RectangleMeshType::Enum),
				MeshData(const Ray&),
				MeshData(const Line&),
				MeshData(const Line&, MeshData::LineMeshType::Enum),
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
				[](const glm::uvec2& size, MeshData::RectangleMeshType::Enum type) {
					return MeshData{ Rectangle{size}, type };
				},
				[](const VarLuaTable<glm::vec2>& size) {
						return MeshData{ Rectangle{LuaGlm::tableGet(size)} };
				},
				[](const VarLuaTable<glm::vec2>& size, MeshData::RectangleMeshType::Enum type) {
						return MeshData{ Rectangle{LuaGlm::tableGet(size)}, type };
				},
				[](MeshData::RectangleMeshType::Enum type) {
					return MeshData(Rectangle(), type);
				}
			),
			"new_arrow", []() { return MeshData{ Line{}, MeshData::LineMeshType::Arrow }; },
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