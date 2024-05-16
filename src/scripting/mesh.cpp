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

	LuaMesh LuaMeshCreator::createQuad3(const VarUvec2& size) noexcept
	{
		return LuaMesh(_creator->createQuad(Quad(LuaMath::tableToGlm(size))));
	}

	LuaMesh LuaMeshCreator::createLineQuad1() noexcept
	{
		return LuaMesh(_creator->createLineQuad());
	}

	LuaMesh LuaMeshCreator::createLineQuad2(const Quad& quad) noexcept
	{
		return LuaMesh(_creator->createLineQuad(quad));
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
			sol::constructors<MeshCreationConfig()>(),
			"scale", &MeshCreationConfig::scale,
			"offset", &MeshCreationConfig::offset,
			"texture_scale", &MeshCreationConfig::textureScale,
			"texture_offset", &MeshCreationConfig::textureOffset,
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
			"create_quad", sol::overload(
				&LuaMeshCreator::createQuad1,
				&LuaMeshCreator::createQuad2,
				&LuaMeshCreator::createQuad3
			),
			"create_line_quad", sol::overload(&LuaMeshCreator::createLineQuad1, &LuaMeshCreator::createLineQuad2),
			"create_line", &LuaMeshCreator::createLine,
			"create_ray", &LuaMeshCreator::createRay,
			"create_lines", &LuaMeshCreator::createLines,
			"create", sol::overload(
				&LuaMeshCreator::createCube2,
				&LuaMeshCreator::createSphere2,
				&LuaMeshCreator::createSphere3,
				&LuaMeshCreator::createSphere4,
				&LuaMeshCreator::createQuad2,
				&LuaMeshCreator::createLine,
				&LuaMeshCreator::createRay,
				&LuaMeshCreator::createLines
			)
		);
	}

	LuaRenderable LuaRenderable::addEntityComponent1(LuaEntity& entity, const LuaMesh& mesh) noexcept
	{
		return entity.addComponent<Renderable>(mesh.getReal());
	}


	LuaRenderable LuaRenderable::addEntityComponent2(LuaEntity& entity, const LuaMaterial& material) noexcept
	{
		return entity.addComponent<Renderable>(material.getReal());
	}

	LuaRenderable LuaRenderable::addEntityComponent3(LuaEntity& entity, const LuaMesh& mesh, const LuaMaterial& material) noexcept
	{
		return entity.addComponent<Renderable>(mesh.getReal(), material.getReal());
	}

	LuaRenderable LuaRenderable::addEntityComponent4(LuaEntity& entity, const LuaMesh& mesh, const LuaTexture& texture) noexcept
	{
		return entity.addComponent<Renderable>(mesh.getReal(), texture.getReal());
	}

	std::optional<LuaRenderable> LuaRenderable::getEntityComponent(LuaEntity& entity) noexcept
	{
		return entity.getComponent<Renderable, LuaRenderable>();
	}

	std::optional<LuaEntity> LuaRenderable::getEntity(LuaScene& scene) noexcept
	{
		return scene.getEntity(_renderable.value());
	}

	LuaRenderable::LuaRenderable(Renderable& renderable) noexcept
		: _renderable(renderable)
	{
	}

	const Renderable& LuaRenderable::getReal() const
	{
		return _renderable.value();
	}

	Renderable& LuaRenderable::getReal()
	{
		return _renderable.value();
	}

	std::optional<LuaMesh> LuaRenderable::getMesh() const noexcept
	{
		auto mesh = _renderable->getMesh();
		if (mesh == nullptr)
		{
			return std::nullopt;
		}
		return LuaMesh(mesh);
	}

	LuaRenderable& LuaRenderable::setMesh(const LuaMesh& mesh) noexcept
	{
		_renderable->setMesh(mesh.getReal());
		return *this;
	}

	LuaMaterial LuaRenderable::getMaterial() const noexcept
	{
		return LuaMaterial(_renderable->getMaterial());
	}

	LuaRenderable& LuaRenderable::setMaterial(const LuaMaterial& material) noexcept
	{
		_renderable->setMaterial(material.getReal());
		return *this;
	}

	void LuaRenderable::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaRenderable>("Renderable",
			sol::constructors<>(),
			"type_id", &entt::type_hash<Renderable>::value,
			"add_entity_component", sol::overload(
				&LuaRenderable::addEntityComponent1,
				&LuaRenderable::addEntityComponent2,
				&LuaRenderable::addEntityComponent3
			),
			"get_entity_component", &LuaRenderable::getEntityComponent,
			"get_entity", &LuaRenderable::getEntity,
			"mesh", sol::property(&LuaRenderable::getMesh, &LuaRenderable::setMesh)
		);
	}

}