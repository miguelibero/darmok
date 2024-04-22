#include "asset.hpp"
#include "scene.hpp"
#include <darmok/asset.hpp>
#include <darmok/material.hpp>
#include <darmok/mesh.hpp>
#include <darmok/program.hpp>
#include <darmok/texture.hpp>
#include <darmok/texture_atlas.hpp>
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

		lua.new_enum<StandardProgramType>("StandardProgramType", {
			{ "Gui", StandardProgramType::Gui },
			{ "Unlit", StandardProgramType::Unlit },
			{ "ForwardPhong", StandardProgramType::ForwardPhong },
			{ "ForwardPbr", StandardProgramType::ForwardPbr }
		});
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

	void LuaTextureAtlas::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaTextureAtlas>("TextureAtlas",
			sol::constructors<>()
		);
	}

	LuaTextureAtlasMeshCreator::LuaTextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const LuaTextureAtlas& atlas) noexcept
		: _creator(std::make_shared<TextureAtlasMeshCreator>(layout, *atlas.getReal()))
		, _atlas(atlas)
	{
	}

	LuaTextureAtlasMeshCreator::LuaTextureAtlasMeshCreator(const bgfx::VertexLayout& layout, const LuaTextureAtlas& atlas, const Config& cfg) noexcept
		: _creator(std::make_shared<TextureAtlasMeshCreator>(layout, *atlas.getReal(), cfg))
		, _atlas(atlas)
	{
	}

	LuaTextureAtlasMeshCreator::~LuaTextureAtlasMeshCreator()
	{
	}

	LuaTextureAtlasMeshCreator::Config& LuaTextureAtlasMeshCreator::getConfig() noexcept
	{
		return _creator->config;
	}

	void LuaTextureAtlasMeshCreator::setConfig(const Config& config) noexcept
	{
		_creator->config = config;
	}

	bgfx::VertexLayout& LuaTextureAtlasMeshCreator::getVertexLayout()  noexcept
	{
		return _creator->layout;
	}

	const LuaTextureAtlas& LuaTextureAtlasMeshCreator::getTextureAtlas() noexcept
	{
		return _atlas;
	}

	LuaMesh LuaTextureAtlasMeshCreator::createSprite(const std::string& name) const noexcept
	{
		return LuaMesh(_creator->createSprite(name));
	}

	std::vector<AnimationFrame> LuaTextureAtlasMeshCreator::createAnimation1(const std::string& namePrefix) const noexcept
	{
		return _creator->createAnimation(namePrefix);
	}

	std::vector<AnimationFrame> LuaTextureAtlasMeshCreator::createAnimation2(const std::string& namePrefix, float frameDuration) const noexcept
	{
		return _creator->createAnimation(namePrefix, frameDuration);
	}

	void LuaTextureAtlasMeshCreator::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<TextureAtlasMeshCreationConfig>("TextureAtlasMeshCreationConfig",
			sol::constructors<
			MeshCreationConfig(const glm::vec3&, const glm::vec3&, const Color&),
			MeshCreationConfig(const glm::vec3&, const glm::vec3&),
			MeshCreationConfig(const glm::vec3&),
			MeshCreationConfig()
			>(),
			"scale", &TextureAtlasMeshCreationConfig::scale,
			"offset", &TextureAtlasMeshCreationConfig::offset,
			"color", &TextureAtlasMeshCreationConfig::color
		);

		lua.new_usertype<LuaTextureAtlasMeshCreator>("TextureAtlasMeshCreator",
			sol::constructors<
				LuaTextureAtlasMeshCreator(const bgfx::VertexLayout&, const LuaTextureAtlas&),
				LuaTextureAtlasMeshCreator(const bgfx::VertexLayout&, const LuaTextureAtlas&, const Config&)>(),
			"config", sol::property(&LuaTextureAtlasMeshCreator::getConfig, &LuaTextureAtlasMeshCreator::setConfig),
			"vertex_layout", sol::property(&LuaTextureAtlasMeshCreator::getVertexLayout),
			"atlas", sol::property(&LuaTextureAtlasMeshCreator::getTextureAtlas),
			"create_sprite", &LuaTextureAtlasMeshCreator::createSprite,
			"create_animation", sol::overload(
				&LuaTextureAtlasMeshCreator::createAnimation1,
				&LuaTextureAtlasMeshCreator::createAnimation2)
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

	uint8_t LuaMaterial::getShininess() const noexcept
	{
		return _material->getShininess();
	}

	LuaMaterial& LuaMaterial::setShininess(uint8_t v) noexcept
	{
		_material->setShininess(v);
		return *this;
	}

	float LuaMaterial::getSpecularStrength() const noexcept
	{
		return _material->getSpecularStrength();
	}

	LuaMaterial& LuaMaterial::setSpecularStrength(float v) noexcept
	{
		_material->setSpecularStrength(v);
		return *this;
	}

	void LuaMaterial::configure(sol::state_view& lua) noexcept
	{
		lua.new_usertype<LuaMaterial>("Material",
			sol::constructors<LuaMaterial(), LuaMaterial(LuaTexture)>(),
			"shininess", sol::property(&LuaMaterial::getShininess, &LuaMaterial::setShininess),
			"specular_strength", sol::property(&LuaMaterial::getSpecularStrength, &LuaMaterial::setSpecularStrength)
		);
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

	LuaMeshCreator::LuaMeshCreator(const bgfx::VertexLayout& layout, const Config& cfg) noexcept
		: _creator(std::make_shared<MeshCreator>(layout, cfg))
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
			sol::constructors<LuaMeshCreator(const bgfx::VertexLayout&), LuaMeshCreator(const bgfx::VertexLayout&, const MeshCreationConfig&)>(),
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

	LuaProgram LuaAssets::loadStandardProgram(StandardProgramType type)
	{
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
		LuaTextureAtlasMeshCreator::configure(lua);
		LuaMaterial::configure(lua);
		LuaMesh::configure(lua);
		LuaMeshCreator::configure(lua);

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