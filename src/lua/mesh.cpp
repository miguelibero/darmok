#include "mesh.hpp"
#include "glm.hpp"
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

		lua.new_enum<MeshType>("MeshType", {
			{ "Static", MeshType::Static },
			{ "Dynamic", MeshType::Dynamic },
			{ "Transient", MeshType::Transient }
		});

		lua.new_enum<RectangleMeshType>("RectangleMeshType", {
			{ "Full", RectangleMeshType::Full },
			{ "Outline", RectangleMeshType::Outline }
		});

		lua.new_enum<LineMeshType>("LineMeshType", {
			{ "Line", LineMeshType::Line },
			{ "Arrow", LineMeshType::Arrow }
		});

		lua.new_usertype<MeshData>("MeshData",
			sol::constructors<
				MeshData(),
				MeshData(const Cube&),
				MeshData(const Sphere&),
				MeshData(const Sphere&, int),
				MeshData(const Capsule&),
				MeshData(const Capsule&, int),
				MeshData(const Rectangle&),
				MeshData(const Rectangle&, RectangleMeshType),
				MeshData(const Ray&),
				MeshData(const Line&),
				MeshData(const Line&, LineMeshType),
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
					return MeshData(Rectangle());
				},
				[](const glm::uvec2& size) {
					return MeshData(Rectangle(size));
				},
				[](const glm::uvec2& size, RectangleMeshType type) {
					return MeshData(Rectangle(size), type);
				},
				[](const VarLuaTable<glm::vec2>& size) {
					return MeshData(Rectangle(LuaGlm::tableGet(size)));
				},
				[](const VarLuaTable<glm::vec2>& size, RectangleMeshType type) {
					return MeshData(Rectangle(LuaGlm::tableGet(size)), type);
				},
				[](RectangleMeshType type) {
					return MeshData(Rectangle(), type);
				}
			),
			"new_arrow", []() { return MeshData(Line(), LineMeshType::Arrow); },
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
			)
		);
	}
}