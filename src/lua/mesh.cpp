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
	}

	LuaMeshCreator::LuaMeshCreator() noexcept
		: LuaMeshCreator(std::nullopt)
	{
	}

	LuaMeshCreator::LuaMeshCreator(const std::optional<bgfx::VertexLayout>& layout) noexcept
		: _creator(std::make_shared<MeshCreator>(layout))
	{
	}

	LuaMeshCreator::~LuaMeshCreator() noexcept
	{
	}

	LuaMeshCreator::Config& LuaMeshCreator::getConfig() noexcept
	{
		return _creator->config;
	}

	void LuaMeshCreator::setConfig(const Config& config) noexcept
	{
		_creator->config = config;
	}

	void LuaMeshCreator::setVertexLayout(const std::optional<bgfx::VertexLayout>& layout)  noexcept
	{
		_creator->vertexLayout = layout;
	}

	std::optional<bgfx::VertexLayout> LuaMeshCreator::getVertexLayout()  noexcept
	{
		return _creator->vertexLayout;
	}

	LuaMesh LuaMeshCreator::createMesh(const MeshData& meshData) noexcept
	{
		return LuaMesh(_creator->createMesh(meshData));
	}

	LuaMesh LuaMeshCreator::createCuboid1() noexcept
	{
		return LuaMesh(_creator->createCuboid());
	}

	LuaMesh LuaMeshCreator::createCuboid2(const Cuboid& cuboid) noexcept
	{
		return LuaMesh(_creator->createCuboid(cuboid));
	}

	LuaMesh LuaMeshCreator::createSphere1() noexcept
	{
		return LuaMesh(_creator->createSphere());
	}

	LuaMesh LuaMeshCreator::createSphere2(const Sphere& sphere) noexcept
	{
		return LuaMesh(_creator->createSphere(sphere));
	}

	LuaMesh LuaMeshCreator::createSphere3(int lod) noexcept
	{
		return LuaMesh(_creator->createSphere(lod));
	}

	LuaMesh LuaMeshCreator::createSphere4(const Sphere& sphere, int lod) noexcept
	{
		return LuaMesh(_creator->createSphere(sphere, lod));
	}

	LuaMesh LuaMeshCreator::createSphere5(const glm::vec3& origin) noexcept
	{
		return LuaMesh(_creator->createSphere(Sphere{ 1, origin }));
	}

	LuaMesh LuaMeshCreator::createSphere6(const glm::vec3& origin, int lod) noexcept
	{
		return LuaMesh(_creator->createSphere(Sphere{ 1, origin }, lod));
	}

	LuaMesh LuaMeshCreator::createCapsule1() noexcept
	{
		return LuaMesh(_creator->createCapsule());
	}

	LuaMesh LuaMeshCreator::createCapsule2(const Capsule& caps) noexcept
	{
		return LuaMesh(_creator->createCapsule(caps));
	}

	LuaMesh LuaMeshCreator::createRectangle1() noexcept
	{
		return LuaMesh(_creator->createRectangle());
	}

	LuaMesh LuaMeshCreator::createRectangle2(const Rectangle& rect) noexcept
	{
		return LuaMesh(_creator->createRectangle(rect));
	}

	LuaMesh LuaMeshCreator::createRectangle3(const glm::uvec2& size) noexcept
	{
		return LuaMesh(_creator->createRectangle(Rectangle(size)));
	}

	LuaMesh LuaMeshCreator::createLineRectangle1() noexcept
	{
		return LuaMesh(_creator->createLineRectangle());
	}

	LuaMesh LuaMeshCreator::createLineRectangle2(const Rectangle& rect) noexcept
	{
		return LuaMesh(_creator->createLineRectangle(rect));
	}

	LuaMesh LuaMeshCreator::createRay(const Ray& ray) noexcept
	{
		return LuaMesh(_creator->createRay(ray));
	}

	LuaMesh LuaMeshCreator::createLine(const Line& line) noexcept
	{
		return LuaMesh(_creator->createLine(line));
	}

	LuaMesh LuaMeshCreator::createLines(const std::vector<Line>& lines) noexcept
	{
		return LuaMesh(_creator->createLines(lines));
	}

	void LuaMeshCreator::bind(sol::state_view& lua) noexcept
	{
		lua.new_enum<MeshType>("MeshType", {
			{ "Static", MeshType::Static },
			{ "Dynamic", MeshType::Dynamic },
			{ "Transient", MeshType::Transient }
		});

		lua.new_usertype<MeshCreationConfig>("MeshCreationConfig",
			sol::constructors<MeshCreationConfig()>(),
			"scale", &MeshCreationConfig::scale,
			"offset", &MeshCreationConfig::offset,
			"texture_scale", &MeshCreationConfig::textureScale,
			"texture_offset", &MeshCreationConfig::textureOffset,
			"color", &MeshCreationConfig::color,
			"type", &MeshCreationConfig::type
		);

		lua.new_usertype<LuaMeshCreator>("MeshCreator",
			sol::constructors<LuaMeshCreator(), LuaMeshCreator(const std::optional<bgfx::VertexLayout>&)>(),
			"config", sol::property(&LuaMeshCreator::getConfig, &LuaMeshCreator::setConfig),
			"vertex_layout", sol::property(&LuaMeshCreator::getVertexLayout, &LuaMeshCreator::setVertexLayout),
			"create_cuboid", sol::overload(&LuaMeshCreator::createCuboid1, &LuaMeshCreator::createCuboid2),
			"create_ccapsule", sol::overload(&LuaMeshCreator::createCapsule1, &LuaMeshCreator::createCapsule2),
			"create_sphere", sol::overload(
				&LuaMeshCreator::createSphere1, &LuaMeshCreator::createSphere2,
				&LuaMeshCreator::createSphere3, &LuaMeshCreator::createSphere4,
				&LuaMeshCreator::createSphere5, &LuaMeshCreator::createSphere6),
			"create_rectangle", sol::overload(
				&LuaMeshCreator::createRectangle1,
				&LuaMeshCreator::createRectangle2,
				&LuaMeshCreator::createRectangle3
			),
			"create_line_rectangle", sol::overload(&LuaMeshCreator::createLineRectangle1, &LuaMeshCreator::createLineRectangle2),
			"create_line", &LuaMeshCreator::createLine,
			"create_ray", &LuaMeshCreator::createRay,
			"create_lines", &LuaMeshCreator::createLines,
			"create", sol::overload(
				&LuaMeshCreator::createCuboid2,
				&LuaMeshCreator::createSphere2,
				&LuaMeshCreator::createSphere3,
				&LuaMeshCreator::createSphere4,
				&LuaMeshCreator::createRectangle2,
				&LuaMeshCreator::createCapsule2,
				&LuaMeshCreator::createLine,
				&LuaMeshCreator::createRay,
				&LuaMeshCreator::createLines
			)
		);
	}
}