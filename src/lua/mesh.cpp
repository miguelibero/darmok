#include "lua/mesh.hpp"
#include "lua/glm.hpp"
#include "lua/utils.hpp"
#include "lua/protobuf.hpp"
#include "lua/scene_serialize.hpp"
#include <darmok/mesh.hpp>
#include <darmok/material.hpp>
#include <darmok/texture.hpp>
#include <darmok/shape.hpp>
#include <entt/entt.hpp>

namespace darmok
{
	void LuaMesh::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<Mesh>("Mesh", sol::factories(
				[](const Mesh::Definition& def) {
					auto mesh = LuaUtils::unwrapExpected(Mesh::load(def));
					return std::make_shared<Mesh>(std::move(mesh));
				}
			),
			sol::meta_function::to_string, &Mesh::toString,
			"vertex_layout", sol::property(&Mesh::getVertexLayout),
			"vertex_handle_index", sol::property(&Mesh::getVertexHandleIndex),
			"empty", &Mesh::empty,
			"update_vertices", &Mesh::updateVertices,
			"updateIndices", &Mesh::updateIndices
		);

		LuaUtils::newEnum<Mesh::Type>(lua, "MeshType");
		LuaUtils::newEnum<MeshData::FillType>(lua, "MeshFillType");
		LuaUtils::newEnum<MeshData::LineType>(lua, "LineMeshType");

		auto src = lua.new_usertype<Mesh::Source>("MeshSource",
			sol::factories(
				[]() { return Mesh::createSource(); }
			),
			"get_scene_asset", [](LuaSceneDefinition& scene, std::string_view path)
			{
				return scene.getAsset<Mesh::Source>(path);
			}
		);
		LuaProtobufBinding{ std::move(src) }
			.protobufProperty<protobuf::ProgramRef>("program")
			.protobufProperty<protobuf::DataMeshSource>("data")
			.protobufProperty<protobuf::CubeMeshSource>("cube")
			.protobufProperty<protobuf::SphereMeshSource>("sphere")
			.protobufProperty<protobuf::CapsuleMeshSource>("capsule")
			.protobufProperty<protobuf::RectangleMeshSource>("rectangle")
			;

		auto def = lua.new_usertype<Mesh::Definition>("MeshDefinition",
			sol::factories(
				[]() { return Mesh::createDefinition(); }
			),
			"get_scene_asset", [](LuaSceneDefinition& scene, std::string_view path)
			{
				return scene.getAsset<Mesh::Definition>(path);
			}
		);
		LuaProtobufBinding{ std::move(def) }
			.protobufProperty<protobuf::VertexLayout>("layout")
			.convertProtobufProperty<BoundingBox>("bounds")
			;

		lua.new_usertype<MeshData>("MeshData",
			sol::constructors<
				MeshData(),
				MeshData(const Cube&),
				MeshData(const Sphere&),
				MeshData(const Sphere&, int),
				MeshData(const Capsule&),
				MeshData(const Capsule&, int),
				MeshData(const Rectangle&),
				MeshData(const Rectangle&, MeshData::FillType),
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
				[](const glm::uvec2& size, MeshData::FillType type)
				{
					return MeshData{ Rectangle{size}, type };
				},
				[](const VarLuaTable<glm::vec2>& size)
				{
					return MeshData{ Rectangle{LuaGlm::tableGet(size)} };
				},
				[](const VarLuaTable<glm::vec2>& size, MeshData::FillType type)
				{
					return MeshData{ Rectangle{LuaGlm::tableGet(size)}, type };
				},
				[](MeshData::FillType type)
				{
					return MeshData{ Rectangle{}, type };
				}
			),
			"new_arrow", []() { return MeshData{ Line{}, Mesh::Definition::Arrow }; },
			"default_vertex_layout", sol::property(&MeshData::getDefaultVertexLayout),
			"create_mesh", sol::overload(
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout)
				{ 
					auto mesh = LuaUtils::unwrapExpected(data.createMesh(vertexLayout));
					return std::make_shared<Mesh>(std::move(mesh));
				},
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout, const MeshConfig& config)
				{
					auto mesh = LuaUtils::unwrapExpected(data.createMesh(vertexLayout, config));
					return std::make_shared<Mesh>(std::move(mesh));
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