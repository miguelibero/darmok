#include "mesh.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "scene.hpp"
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>
#include <entt/entt.hpp>

namespace darmok
{
	LuaMesh::LuaMesh(const std::shared_ptr<IMesh>& mesh) noexcept
		: _mesh(mesh)
	{
	}

	std::string LuaMesh::to_string() const noexcept
	{
		return _mesh->to_string();
	}

	std::shared_ptr<IMesh> LuaMesh::getReal() const noexcept
	{
		return _mesh;
	}

	void LuaMesh::bind(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMesh>("Mesh", sol::no_constructor
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
			{ "Diamond", LineMeshType::Diamond }
		});

		lua.new_usertype<MeshDataConfig>("MeshDataConfig",
			sol::constructors<MeshDataConfig()>(),
			"scale", &MeshDataConfig::scale,
			"offset", &MeshDataConfig::offset,
			"texture_scale", &MeshDataConfig::textureScale,
			"texture_offset", &MeshDataConfig::textureOffset,
			"color", &MeshDataConfig::color,
			"type", &MeshDataConfig::type
		);

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
			"new_cube", []() { return MeshData(Cube()); },
			"new_sphere", []() { return MeshData(Sphere()); },
			"new_capsule", []() { return MeshData(Capsule()); },
			"new_rectangle", []() { return MeshData(Rectangle()); },
			"new_rectangle", [](RectangleMeshType type) { return MeshData(Rectangle(), type); },
			"new_bone", []() { return MeshData(Line(), LineMeshType::Diamond); },
			"config", &MeshData::config,
			"default_vertex_layout", sol::property(&MeshData::getDefaultVertexLayout),
			"create_mesh", sol::overload(
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout)
				{ 
					return LuaMesh(data.createMesh(vertexLayout));
				},
				[](const MeshData& data, const bgfx::VertexLayout& vertexLayout, const MeshConfig& config)
				{ 
					return LuaMesh(data.createMesh(vertexLayout, config));
				}
			)
		);
	}
}