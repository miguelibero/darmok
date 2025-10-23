#include "lua/mesh.hpp"
#include "lua/glm.hpp"
#include "lua/utils.hpp"
#include "lua/protobuf.hpp"
#include "lua/scene_serialize.hpp"
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/shape.hpp>
#include <darmok/shape_serialize.hpp>
#include <entt/entt.hpp>

namespace darmok
{
	void LuaMesh::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Mesh>("Mesh", sol::factories(
				[](const Mesh::Definition& def) { return std::make_shared<Mesh>(def); }
			),
			sol::meta_function::to_string, &Mesh::toString,
			"vertex_layout", sol::property(&Mesh::getVertexLayout),
			"vertex_handle_index", sol::property(&Mesh::getVertexHandleIndex),
			"empty", &Mesh::empty,
			"update_vertices", &Mesh::updateVertices,
			"updateIndices", &Mesh::updateIndices
		);

		LuaUtils::newEnum<Mesh::Type>(lua, "MeshType");
		LuaUtils::newEnum<MeshData::RectangleType>(lua, "RectangleMeshType");
		LuaUtils::newEnum<MeshData::LineType>(lua, "LineMeshType");

		LuaUtils::newProtobuf<Mesh::Source>(lua, "MeshSource")
			.protobufProperty<protobuf::ProgramRef>("program")
			.protobufProperty<protobuf::DataMeshSource>("data")
			.protobufProperty<protobuf::CubeMeshSource>("cube")
			.protobufProperty<protobuf::SphereMeshSource>("sphere")
			.protobufProperty<protobuf::CapsuleMeshSource>("capsule")
			.protobufProperty<protobuf::RectangleMeshSource>("rectangle");

		auto meshDef = LuaUtils::newProtobuf<Mesh::Definition>(lua, "MeshDefinition")
			.protobufProperty<protobuf::VertexLayout>("layout")
			.convertProtobufProperty<BoundingBox, protobuf::BoundingBox>("bounds",
				[](const protobuf::BoundingBox& proto) { return protobuf::convert(proto); },
				[](const BoundingBox& bounds) { return protobuf::convert(bounds); });
		
		meshDef.userType["get_scene_asset"] = [](protobuf::Scene& scene, std::string_view path)
		{
			return SceneDefinitionWrapper{ scene }.getAsset<Mesh::Definition>(path);
		};

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
				[]()
				{
					return MeshData{ Rectangle{} };
				},
				[](const glm::uvec2& size)
				{
					return MeshData{ Rectangle{size} };
				},
				[](const glm::uvec2& size, MeshData::RectangleType type)
				{
					return MeshData{ Rectangle{size}, type };
				},
				[](const VarLuaTable<glm::vec2>& size)
				{
					return MeshData{ Rectangle{LuaGlm::tableGet(size)} };
				},
				[](const VarLuaTable<glm::vec2>& size, MeshData::RectangleType type)
				{
					return MeshData{ Rectangle{LuaGlm::tableGet(size)}, type };
				},
				[](MeshData::RectangleType type)
				{
					return MeshData{ Rectangle{}, type };
				}
			),
			"new_arrow", []() { return MeshData{ Line{}, Mesh::Definition::Arrow }; },
			"default_vertex_layout", sol::property(&MeshData::getDefaultVertexLayout),
			"create_mesh", sol::overload(
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout)
				{ 
					return std::make_shared<Mesh>(data.createMesh(vertexLayout));
				},
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout, const MeshConfig& config)
				{
					return std::make_shared<Mesh>(data.createMesh(vertexLayout, config));
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