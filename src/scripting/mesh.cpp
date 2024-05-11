#include "mesh.hpp"
#include "material.hpp"
#include "texture.hpp"
#include <darmok/mesh.hpp>
#include <darmok/shape.hpp>

namespace darmok
{
	LuaMesh::LuaMesh(const std::shared_ptr<Mesh>& mesh) noexcept
		: _mesh(mesh)
	{
	}

	std::string LuaMesh::to_string() const noexcept
	{
		return _mesh->to_string();
	}

	std::shared_ptr<Mesh> LuaMesh::getReal() const noexcept
	{
		return _mesh;
	}

	void LuaMesh::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMesh>("Mesh",
			sol::constructors<>()
		);
	}

	LuaMeshCreator::LuaMeshCreator(const bgfx::VertexLayout& layout) noexcept
		: _creator(std::make_shared<MeshCreator>(layout))
	{
	}

	LuaMeshCreator::~LuaMeshCreator()
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

	bgfx::VertexLayout& LuaMeshCreator::getVertexLayout()  noexcept
	{
		return _creator->layout;
	}

	LuaMesh LuaMeshCreator::createMesh(const MeshData& meshData) noexcept
	{
		return LuaMesh(_creator->createMesh(meshData));
	}

	LuaMesh LuaMeshCreator::createCube1() noexcept
	{
		return LuaMesh(_creator->createCube());
	}

	LuaMesh LuaMeshCreator::createCube2(const Cube& cube) noexcept
	{
		return LuaMesh(_creator->createCube(cube));
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

	LuaMesh LuaMeshCreator::createQuad1() noexcept
	{
		return LuaMesh(_creator->createQuad());
	}

	LuaMesh LuaMeshCreator::createQuad2(const Quad& quad) noexcept
	{
		return LuaMesh(_creator->createQuad(quad));
	}

	LuaMesh LuaMeshCreator::createLineQuad1() noexcept
	{
		return LuaMesh(_creator->createLineQuad());
	}

	LuaMesh LuaMeshCreator::createLineQuad2(const Quad& quad) noexcept
	{
		return LuaMesh(_creator->createLineQuad(quad));
	}

	LuaMesh LuaMeshCreator::createSprite(const LuaTexture& texture) noexcept
	{
		return LuaMesh(_creator->createSprite(texture.getReal()));
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

	void LuaMeshCreator::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<MeshCreationConfig>("MeshCreationConfig",
			sol::constructors<
			MeshCreationConfig(const glm::vec3&, const glm::vec3&, const glm::vec2&, const Color&),
			MeshCreationConfig(const glm::vec3&, const glm::vec3&, const glm::vec2&),
			MeshCreationConfig(const glm::vec3&, const glm::vec3&),
			MeshCreationConfig(const glm::vec3&),
			MeshCreationConfig()
			>(),
			"scale", &MeshCreationConfig::scale,
			"textureScale", &MeshCreationConfig::textureScale,
			"offset", &MeshCreationConfig::offset,
			"color", &MeshCreationConfig::color
		);

		lua.new_usertype<LuaMeshCreator>("MeshCreator",
			sol::constructors<LuaMeshCreator(const bgfx::VertexLayout&)>(),
			"config", sol::property(&LuaMeshCreator::getConfig, &LuaMeshCreator::setConfig),
			"vertex_layout", sol::property(&LuaMeshCreator::getVertexLayout),
			"create_cube", sol::overload(&LuaMeshCreator::createCube1, &LuaMeshCreator::createCube2),
			"create_sphere", sol::overload(
				&LuaMeshCreator::createSphere1, &LuaMeshCreator::createSphere2,
				&LuaMeshCreator::createSphere3, &LuaMeshCreator::createSphere4),
			"create_quad", sol::overload(&LuaMeshCreator::createQuad1, &LuaMeshCreator::createQuad2),
			"create_line_quad", sol::overload(&LuaMeshCreator::createLineQuad1, &LuaMeshCreator::createLineQuad2),
			"create_sprite", &LuaMeshCreator::createSprite,
			"create_line", &LuaMeshCreator::createLine,
			"create_ray", &LuaMeshCreator::createRay,
			"create_lines", &LuaMeshCreator::createLines,
			"create", sol::overload(
				&LuaMeshCreator::createCube2,
				&LuaMeshCreator::createSphere2,
				&LuaMeshCreator::createSphere3,
				&LuaMeshCreator::createSphere4,
				&LuaMeshCreator::createQuad2,
				&LuaMeshCreator::createSprite,
				&LuaMeshCreator::createLine,
				&LuaMeshCreator::createRay,
				&LuaMeshCreator::createLines
			)
		);
	}

	LuaRenderable::LuaRenderable(Renderable& comp) noexcept
		: _comp(comp)
	{
	}

	const Renderable& LuaRenderable::getReal() const
	{
		return _comp.value();
	}

	Renderable& LuaRenderable::getReal()
	{
		return _comp.value();
	}

	std::optional<LuaMesh> LuaRenderable::getMesh() const noexcept
	{
		auto mesh = _comp->getMesh();
		if (mesh == nullptr)
		{
			return std::nullopt;
		}
		return LuaMesh(mesh);
	}

	LuaRenderable& LuaRenderable::setMesh(const LuaMesh& mesh) noexcept
	{
		_comp->setMesh(mesh.getReal());
		return *this;
	}

	LuaMaterial LuaRenderable::getMaterial() const noexcept
	{
		return LuaMaterial(_comp->getMaterial());
	}

	LuaRenderable& LuaRenderable::setMaterial(const LuaMaterial& material) noexcept
	{
		_comp->setMaterial(material.getReal());
		return *this;
	}

	void LuaRenderable::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaRenderable>("Renderable",
			sol::constructors<>(),
			"type_id", &entt::type_hash<Renderable>::value,
			"mesh", sol::property(&LuaRenderable::getMesh, &LuaRenderable::setMesh)
		);
	}

}