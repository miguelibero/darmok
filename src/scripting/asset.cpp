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
		lua.create_named_table("TextureFlag",
			"NONE", BGFX_TEXTURE_NONE,
			"MSAA_SAMPLE", BGFX_TEXTURE_MSAA_SAMPLE,
			"RT", BGFX_TEXTURE_RT,
			"COMPUTE_WRITE", BGFX_TEXTURE_COMPUTE_WRITE,
			"SRG", BGFX_TEXTURE_SRGB,
			"BLIT_DST", BGFX_TEXTURE_BLIT_DST,
			"READ_BACK", BGFX_TEXTURE_READ_BACK,
			"RT_MSAA_X2", BGFX_TEXTURE_RT_MSAA_X2,
			"RT_MSAA_X4", BGFX_TEXTURE_RT_MSAA_X4,
			"RT_MSAA_X8", BGFX_TEXTURE_RT_MSAA_X8,
			"RT_MSAA_X16", BGFX_TEXTURE_RT_MSAA_X16,
			"RT_MSAA_SHIFT", BGFX_TEXTURE_RT_MSAA_SHIFT,
			"RT_MSAA_MASK", BGFX_TEXTURE_RT_MSAA_MASK,
			"RT_WRITE_ONLY", BGFX_TEXTURE_RT_WRITE_ONLY,
			"RT_SHIFT", BGFX_TEXTURE_RT_SHIFT,
			"RT_MASK", BGFX_TEXTURE_RT_MASK
		);

		lua.create_named_table("SamplerFlag",
			"NONE", BGFX_SAMPLER_NONE,
			"U_MIRROR", BGFX_SAMPLER_U_MIRROR,
			"U_CLAMP", BGFX_SAMPLER_U_CLAMP,
			"U_BORDER", BGFX_SAMPLER_U_BORDER,
			"U_SHIFT", BGFX_SAMPLER_U_SHIFT,
			"U_MASK", BGFX_SAMPLER_U_MASK,
			"V_MIRROR", BGFX_SAMPLER_V_MIRROR,
			"V_CLAMP", BGFX_SAMPLER_V_CLAMP,
			"V_BORDER", BGFX_SAMPLER_V_BORDER,
			"V_SHIFT", BGFX_SAMPLER_V_SHIFT,
			"V_MASK", BGFX_SAMPLER_V_MASK,
			"W_MIRROR", BGFX_SAMPLER_W_MIRROR,
			"W_CLAMP", BGFX_SAMPLER_W_CLAMP,
			"W_BORDER", BGFX_SAMPLER_W_BORDER,
			"W_SHIFT", BGFX_SAMPLER_W_SHIFT,
			"W_MASK", BGFX_SAMPLER_W_MASK,
			"MIN_POINT", BGFX_SAMPLER_MIN_POINT,
			"MIN_ANISOTROPIC", BGFX_SAMPLER_MIN_ANISOTROPIC,
			"MIN_SHIFT", BGFX_SAMPLER_MIN_SHIFT,
			"MIN_MASK", BGFX_SAMPLER_MIN_MASK,
			"MAG_POINT", BGFX_SAMPLER_MAG_POINT,
			"MAG_ANISOTROPIC", BGFX_SAMPLER_MAG_ANISOTROPIC,
			"MAG_SHIFT", BGFX_SAMPLER_MAG_SHIFT,
			"MAG_MASK", BGFX_SAMPLER_MAG_MASK,
			"MIP_POINT", BGFX_SAMPLER_MIP_POINT,
			"MIP_SHIFT", BGFX_SAMPLER_MIP_SHIFT,
			"MIP_MASK", BGFX_SAMPLER_MIP_MASK,
			"COMPARE_LESS", BGFX_SAMPLER_COMPARE_LESS,
			"COMPARE_LEQUAL", BGFX_SAMPLER_COMPARE_LEQUAL,
			"COMPARE_EQUAL", BGFX_SAMPLER_COMPARE_EQUAL,
			"COMPARE_GEQUAL", BGFX_SAMPLER_COMPARE_GEQUAL,
			"COMPARE_GREATER", BGFX_SAMPLER_COMPARE_GREATER,
			"COMPARE_NOTEQUAL", BGFX_SAMPLER_COMPARE_NOTEQUAL,
			"COMPARE_NEVER", BGFX_SAMPLER_COMPARE_NEVER,
			"COMPARE_ALWAYS", BGFX_SAMPLER_COMPARE_ALWAYS,
			"COMPARE_SHIFT", BGFX_SAMPLER_COMPARE_SHIFT,
			"COMPARE_MASK", BGFX_SAMPLER_COMPARE_MASK,
			"BORDER_COLOR_SHIFT", BGFX_SAMPLER_BORDER_COLOR_SHIFT,
			"BORDER_COLOR_MASK", BGFX_SAMPLER_BORDER_COLOR_MASK,
			"RESERVED_SHIFT", BGFX_SAMPLER_RESERVED_SHIFT,
			"RESERVED_MASK", BGFX_SAMPLER_RESERVED_MASK,
			"SAMPLE_STENCIL", BGFX_SAMPLER_SAMPLE_STENCIL,
			"POINT", BGFX_SAMPLER_POINT,
			"UVW_MIRROR", BGFX_SAMPLER_UVW_MIRROR,
			"UVW_CLAMP", BGFX_SAMPLER_UVW_CLAMP,
			"UVW_BORDER", BGFX_SAMPLER_UVW_BORDER,
			"BITS_MASK", BGFX_SAMPLER_BITS_MASK,
			"BORDER_COLOR", [](uint32_t v) { return BGFX_SAMPLER_BORDER_COLOR(v); }
		);

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
				MeshCreationConfig(const glm::vec3&, const glm::vec3&, const Color&, const glm::uvec2& amount),
				MeshCreationConfig(const glm::vec3&, const glm::vec3&, const Color&),
				MeshCreationConfig(const glm::vec3&, const glm::vec3&),
				MeshCreationConfig(const glm::vec3&),
				MeshCreationConfig()
			>(),
			"scale", &TextureAtlasMeshCreationConfig::scale,
			"offset", &TextureAtlasMeshCreationConfig::offset,
			"color", &TextureAtlasMeshCreationConfig::color,
			"amount", &TextureAtlasMeshCreationConfig::amount
		);

		lua.new_usertype<LuaTextureAtlasMeshCreator>("TextureAtlasMeshCreator",
			sol::constructors<
				LuaTextureAtlasMeshCreator(const bgfx::VertexLayout&, const LuaTextureAtlas&)>(),
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

	LuaTextureAtlas LuaAssets::loadTextureAtlas1(const std::string& name)
	{
		return LuaTextureAtlas(_assets->getTextureAtlasLoader()(name));
	}

	LuaTextureAtlas LuaAssets::loadTextureAtlas2(const std::string& name, uint64_t textureFlags)
	{
		return LuaTextureAtlas(_assets->getTextureAtlasLoader()(name, textureFlags));
	}

	LuaTexture LuaAssets::loadTexture1(const std::string& name)
	{
		return LuaTexture(_assets->getTextureLoader()(name));
	}

	LuaTexture LuaAssets::loadTexture2(const std::string& name, uint64_t flags)
	{
		return LuaTexture(_assets->getTextureLoader()(name, flags));
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
			"load_texture", sol::overload(&LuaAssets::loadTexture1, &LuaAssets::loadTexture2),
			"load_color_texture", &LuaAssets::loadColorTexture,
			"load_texture_atlas", sol::overload(&LuaAssets::loadTextureAtlas1, &LuaAssets::loadTextureAtlas2),
			"load_model", &LuaAssets::loadModel
		);
	}
}