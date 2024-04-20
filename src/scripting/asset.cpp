#include "asset.hpp"
#include "scene.hpp"
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/math.hpp>
#include <darmok/model.hpp>

namespace darmok
{
    LuaProgram::LuaProgram(const std::shared_ptr<Program>& program) noexcept
		: _program(program)
	{
	}

	const bgfx::VertexLayout& LuaProgram::getVertexLayout() const noexcept
	{
		return _program->getVertexLayout();
	}

	const std::shared_ptr<Program>& LuaProgram::getReal() const noexcept
	{
		return _program;
	}

	void LuaProgram::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaProgram>("Program",
			sol::constructors<>(),
			"vertex_layout", sol::property(&LuaProgram::getVertexLayout)
		);
	}

	LuaTexture::LuaTexture(const std::shared_ptr<Texture>& texture) noexcept
		: _texture(texture)
	{
	}

	const std::shared_ptr<Texture>& LuaTexture::getReal() const noexcept
	{
		return _texture;
	}

	void LuaTexture::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaTexture>("Texture",
			sol::constructors<>()
		);
	}

	LuaTextureAtlas::LuaTextureAtlas(const std::shared_ptr<TextureAtlas>& atlas) noexcept
		: _atlas(atlas)
	{
	}

	const std::shared_ptr<TextureAtlas>& LuaTextureAtlas::LuaTextureAtlas::getReal() const noexcept
	{
		return _atlas;
	}

	LuaMesh LuaTextureAtlas::createSprite1(const bgfx::VertexLayout& layout, const std::string& name, const SpriteCreationConfig& cfg)
	{
		auto elm = _atlas->getElement(name);
		if(!elm)
		{
			throw std::runtime_error("could not find atlas element");
		}
		return LuaMesh(_atlas->createSprite(layout, elm.value(), cfg));
	}

	LuaMesh LuaTextureAtlas::createSprite2(const bgfx::VertexLayout& layout, const std::string& name)
	{
		auto elm = _atlas->getElement(name);
		if(!elm)
		{
			throw std::runtime_error("could not find atlas element");
		}
		return LuaMesh(_atlas->createSprite(layout, elm.value()));
	}


	void LuaTextureAtlas::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<SpriteCreationConfig>("SpriteCreationConfig",
			sol::constructors<
				SpriteCreationConfig(const glm::vec2&, const glm::vec2&, const glm::vec2&, const Color&),
				SpriteCreationConfig(const glm::vec2&, const glm::vec2&, const glm::vec2&),
				SpriteCreationConfig(const glm::vec2&, const glm::vec2&),
				SpriteCreationConfig(const glm::vec2&),
				SpriteCreationConfig()
			>(),
			"scale", &SpriteCreationConfig::scale,
			"textureScale", &SpriteCreationConfig::textureScale,
			"offset", &SpriteCreationConfig::offset,
			"color", &SpriteCreationConfig::color
		);
		lua.new_usertype<LuaTextureAtlas>("TextureAtlas",
			sol::constructors<>(),
			"create_sprite", sol::overload(
				&LuaTextureAtlas::createSprite1,
				&LuaTextureAtlas::createSprite2
			)
		);
	}

	LuaMaterial::LuaMaterial() noexcept
		: _material(std::make_shared<Material>())
	{
	}

	LuaMaterial::LuaMaterial(const std::shared_ptr<Material>& material) noexcept
		: _material(material)
	{
	}

	LuaMaterial::LuaMaterial(const LuaTexture& texture) noexcept
		: _material(std::make_shared<Material>(texture.getReal()))
	{
	}

	const std::shared_ptr<Material>& LuaMaterial::getReal() const noexcept
	{
		return _material;
	}

	void LuaMaterial::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMaterial>("Material",
			sol::constructors<LuaMaterial(), LuaMaterial(LuaTexture)>());
	}

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

	LuaMesh LuaMesh::createCube1(const bgfx::VertexLayout& layout) noexcept
	{
		return LuaMesh(Mesh::createCube(layout));
	}

	LuaMesh LuaMesh::createCube2(const bgfx::VertexLayout& layout, const Cube& cube) noexcept
	{
		return LuaMesh(Mesh::createCube(layout, cube));
	}

	LuaMesh LuaMesh::createSphere1(const bgfx::VertexLayout& layout) noexcept
	{
		return LuaMesh(Mesh::createSphere(layout));
	}

	LuaMesh LuaMesh::createSphere2(const bgfx::VertexLayout& layout, const Sphere& sphere) noexcept
	{
		return LuaMesh(Mesh::createSphere(layout, sphere));
	}

	LuaMesh LuaMesh::createSphere3(const bgfx::VertexLayout& layout, const Sphere& sphere, int lod) noexcept
	{
		return LuaMesh(Mesh::createSphere(layout, sphere, lod));
	}

	LuaMesh LuaMesh::createQuad1(const bgfx::VertexLayout& layout) noexcept
	{
		return LuaMesh(Mesh::createQuad(layout));
	}

	LuaMesh LuaMesh::createQuad2(const bgfx::VertexLayout& layout, const Quad& quad) noexcept
	{
		return LuaMesh(Mesh::createQuad(layout, quad));
	}

	LuaMesh LuaMesh::createLineQuad1(const bgfx::VertexLayout& layout) noexcept
	{
		return LuaMesh(Mesh::createLineQuad(layout, Quad::standard));
	}

	LuaMesh LuaMesh::createLineQuad2(const bgfx::VertexLayout& layout, const Quad& quad) noexcept
	{
		return LuaMesh(Mesh::createLineQuad(layout, quad));
	}

	LuaMesh LuaMesh::createSprite1(const bgfx::VertexLayout& layout, const LuaTexture& texture) noexcept
	{
		return LuaMesh(Mesh::createSprite(layout, texture.getReal()));
	}

	LuaMesh LuaMesh::createSprite2(const bgfx::VertexLayout& layout, const LuaTexture& texture, float scale) noexcept
	{
		return LuaMesh(Mesh::createSprite(layout, texture.getReal(), scale));
	}

	LuaMesh LuaMesh::createLine(const bgfx::VertexLayout& layout, const Line& line) noexcept
	{
		return LuaMesh(Mesh::createLine(layout, line));
	}

	LuaMesh LuaMesh::createRay(const bgfx::VertexLayout& layout, const Ray& ray) noexcept
	{
		return createLine(layout, ray.toLine());
	}

	LuaMesh LuaMesh::createLines(const bgfx::VertexLayout& layout, const std::vector<Line>& lines) noexcept
	{
		return LuaMesh(Mesh::createLines(layout, lines));
	}

	void LuaMesh::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMesh>("Mesh",
			sol::constructors<LuaMesh(const bgfx::VertexLayout&)>(),
			"create_cube", sol::overload(&LuaMesh::createCube1, &LuaMesh::createCube2),
			"create_sphere", sol::overload(&LuaMesh::createSphere1, &LuaMesh::createSphere2, &LuaMesh::createSphere3),
			"create_quad", sol::overload(&LuaMesh::createQuad1, &LuaMesh::createQuad2),
			"create_line_quad", sol::overload(&LuaMesh::createLineQuad1, &LuaMesh::createLineQuad2),
			"create_sprite", sol::overload(&LuaMesh::createSprite1, &LuaMesh::createSprite2),
			"create_line", &LuaMesh::createLine,
			"create_ray", &LuaMesh::createRay,
			"create_lines", &LuaMesh::createLines,
			"create", sol::overload(
				&LuaMesh::createCube2,
				&LuaMesh::createSphere2,
				&LuaMesh::createSphere3,
				&LuaMesh::createQuad2,
				&LuaMesh::createSprite1,
				&LuaMesh::createSprite2,
				&LuaMesh::createLine,
				&LuaMesh::createRay,
				&LuaMesh::createLines
			),
			"material", sol::property(&LuaMesh::getMaterial, &LuaMesh::setMaterial)
		);
	}

	LuaModel::LuaModel(const std::shared_ptr<Model>& model) noexcept
		: _model(model)
		{
		}

	const std::shared_ptr<Model>& LuaModel::getReal() const noexcept
	{
		return _model;
	}

	class LuaModelAddToSceneCallback final
	{
	public:
		LuaModelAddToSceneCallback(Scene& scene, const sol::protected_function& callback)
			: _scene(scene)
			, _callback(callback)
		{
		}

		void operator()(const ModelNode& node, Entity entity) const noexcept
		{
			auto result = _callback(node, LuaEntity(entity, _scene));
			if (result.valid())
			{
				return;
			}
			sol::error err = result;
			std::cerr << "error adding model to scene:" << std::endl;
			std::cerr << err.what() << std::endl;
		}


	private:
		Scene& _scene;
		const sol::protected_function& _callback;
	};

	LuaEntity LuaModel::addToScene1(LuaScene& scene, const bgfx::VertexLayout& layout)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout);
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaModel::addToScene2(LuaScene& scene, const bgfx::VertexLayout& layout, sol::protected_function callback)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout, LuaModelAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaModel::addToScene3(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout, parent.getReal());
		return LuaEntity(entity, realScene);
	}

	LuaEntity LuaModel::addToScene4(LuaScene& scene, const bgfx::VertexLayout& layout, const LuaEntity& parent, sol::protected_function callback)
	{
		auto& realScene = scene.getReal();
		auto entity = _model->addToScene(realScene, layout, parent.getReal(), LuaModelAddToSceneCallback(realScene, callback));
		return LuaEntity(entity, realScene);
	}

	void LuaModel::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaModel>("Model",
			sol::constructors<>(),
			"add_to_scene", sol::overload(
				&LuaModel::addToScene1,
				&LuaModel::addToScene2,
				&LuaModel::addToScene3,
				&LuaModel::addToScene4
			)
		);
	}

	LuaAssets::LuaAssets(AssetContext& assets) noexcept
		: _assets(assets)
	{
	}

	LuaProgram LuaAssets::loadProgram(const std::string& name)
	{
		return LuaProgram(_assets->getProgramLoader()(name));
	}

	LuaProgram LuaAssets::loadStandardProgram(const std::string& name)
	{
		auto type = StandardProgramType::Unlit;
		if (name == "ForwardPhong")
		{
			type = StandardProgramType::ForwardPhong;
		}
		else if (name == "Unlit")
		{
			type = StandardProgramType::Unlit;
		}
		else
		{
			throw std::exception("unknown standard program");
		}
		return LuaProgram(_assets->getStandardProgramLoader()(type));
	}

	LuaTexture LuaAssets::loadColorTexture(const Color& color)
	{
		return LuaTexture(_assets->getColorTextureLoader()(color));
	}

	LuaTextureAtlas LuaAssets::loadTextureAtlas(const std::string& name)
	{
		return LuaTextureAtlas(_assets->getTextureAtlasLoader()(name));
	}

	LuaTexture LuaAssets::loadTexture(const std::string& name)
	{
		return LuaTexture(_assets->getTextureLoader()(name));
	}

	LuaModel LuaAssets::loadModel(const std::string& name)
	{
		return LuaModel(_assets->getModelLoader()(name));
	}

	void LuaAssets::configure(sol::state_view& lua) noexcept
	{
		LuaProgram::configure(lua);
		LuaTexture::configure(lua);
		LuaTextureAtlas::configure(lua);
		LuaMaterial::configure(lua);
		LuaMesh::configure(lua);

		lua.new_usertype<LuaAssets>("Assets",
			sol::constructors<>(),
			"load_program", &LuaAssets::loadProgram,
			"load_standard_program", &LuaAssets::loadStandardProgram,
			"load_texture", &LuaAssets::loadTexture,
			"load_color_texture", &LuaAssets::loadColorTexture,
			"load_texture_atlas", &LuaAssets::loadTextureAtlas,
			"load_model", &LuaAssets::loadModel
		);
	}
}