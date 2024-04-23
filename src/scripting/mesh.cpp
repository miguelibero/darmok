#include "mesh.hpp"
#include "material.hpp"
#include "texture.hpp"
#include <darmok/mesh.hpp>

namespace darmok
{
	LuaMesh::LuaMesh(const bgfx::VertexLayout& layout) noexcept
		: _mesh(std::make_shared<Mesh>(layout))
	{
	}

	LuaMesh::LuaMesh(const std::shared_ptr<Mesh>& mesh) noexcept
		: _mesh(mesh)
	{
	}

	const std::shared_ptr<Mesh>& LuaMesh::getReal() const noexcept
	{
		return _mesh;
	}

	LuaMaterial LuaMesh::getMaterial() const noexcept
	{
		return LuaMaterial(_mesh->getMaterial());
	}

	void LuaMesh::setMaterial(const LuaMaterial& material) noexcept
	{
		_mesh->setMaterial(material.getReal());
	}

	void LuaMesh::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMesh>("Mesh",
			sol::constructors<LuaMesh(const bgfx::VertexLayout&)>(),
			"material", sol::property(&LuaMesh::getMaterial, &LuaMesh::setMaterial)
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
		return LuaMesh(_creator->createSphere(Sphere::standard, lod));
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

	LuaMeshComponent::LuaMeshComponent(MeshComponent& comp) noexcept
		: _comp(comp)
	{
	}

	std::vector<LuaMesh> LuaMeshComponent::getMeshes() const noexcept
	{
		std::vector<LuaMesh> luaMeshes;
		auto realMeshes = _comp->getMeshes();
		luaMeshes.reserve(realMeshes.size());
		for (auto& mesh : realMeshes)
		{
			luaMeshes.push_back(LuaMesh(mesh));
		}
		return luaMeshes;
	}

	void LuaMeshComponent::setMeshes(const std::vector<LuaMesh>& meshes) noexcept
	{
		std::vector<std::shared_ptr<Mesh>> realMeshes;
		realMeshes.reserve(meshes.size());
		for (auto& mesh : meshes)
		{
			realMeshes.push_back(mesh.getReal());
		}
		_comp->setMeshes(realMeshes);
	}

	void LuaMeshComponent::setMesh(const LuaMesh& mesh) noexcept
	{
		_comp->setMesh(mesh.getReal());
	}

	void LuaMeshComponent::addMesh(const LuaMesh& mesh) noexcept
	{
		_comp->addMesh(mesh.getReal());
	}

	void LuaMeshComponent::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMeshComponent>("MeshComponent",
			sol::constructors<>(),
			"meshes", sol::property(&LuaMeshComponent::getMeshes, &LuaMeshComponent::setMeshes),
			"set_mesh", &LuaMeshComponent::setMesh,
			"add_mesh", &LuaMeshComponent::addMesh
		);
	}

}