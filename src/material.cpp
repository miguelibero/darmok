#include <darmok/material.hpp>
#include <darmok/light.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>
#include <darmok/image.hpp>
#include <darmok/app.hpp>
#include <darmok/string.hpp>
#include <darmok/glm_serialize.hpp>

#include <glm/gtc/type_ptr.hpp>
#include "detail/render_samplers.hpp"

namespace darmok
{
	ConstMaterialDefinitionWrapper::ConstMaterialDefinitionWrapper(const Definition& def) noexcept
		: _def{ def }
	{
	}

	std::optional<std::string> ConstMaterialDefinitionWrapper::getTexturePath(TextureType textureType) noexcept
	{
		auto& textures = _def.textures();
		auto itr = std::find_if(textures.begin(), textures.end(),
			[textureType](const Material::TextureDefinition& tex) {
				return tex.type() == textureType;
			});
		if (itr != textures.end())
		{
			return itr->texture_path();
		}
		return std::nullopt;
	}

	std::optional<std::string> ConstMaterialDefinitionWrapper::getTexturePath(const TextureUniformKey& uniformKey) noexcept
	{
		auto& textures = _def.textures();
		auto itr = std::find_if(textures.begin(), textures.end(),
			[uniformKey](const Material::TextureDefinition& tex) {
				return tex.has_uniform() && tex.uniform().SerializeAsString() == uniformKey.SerializeAsString();
			});
		if (itr != textures.end())
		{
			return itr->texture_path();
		}
		return std::nullopt;
	}

	MaterialDefinitionWrapper::MaterialDefinitionWrapper(Definition& def) noexcept
		: ConstMaterialDefinitionWrapper(def)
		, _def{ def }
	{
	}

	bool MaterialDefinitionWrapper::setTexturePath(MaterialTextures& textures, MaterialTextures::iterator itr, const std::string& texturePath) noexcept
	{
		if (texturePath.empty())
		{
			if (itr != textures.end())
			{
				textures.erase(itr);
				return true;
			}
			return false;
		}
		if (itr->texture_path() != texturePath)
		{
			itr->set_texture_path(texturePath);
			return true;
		}
		return false;
	}

	bool MaterialDefinitionWrapper::setTexturePath(TextureType textureType, const std::string& texturePath) noexcept
	{
		auto& textures = *_def.mutable_textures();
		auto itr = std::find_if(textures.begin(), textures.end(),
			[textureType](const Material::TextureDefinition& tex) {
				return tex.has_type() && tex.type() == textureType;
			});
		if (itr == textures.end())
		{
			auto texDef = textures.Add();
			texDef->set_type(textureType);
			itr = textures.end() - 1;
		}
		return setTexturePath(textures, itr, texturePath);
	}

	bool MaterialDefinitionWrapper::setTexturePath(const TextureUniformKey& uniformKey, const std::string& texturePath) noexcept
	{
		auto& textures = *_def.mutable_textures();
		auto itr = std::find_if(textures.begin(), textures.end(),
			[uniformKey](const Material::TextureDefinition& tex) {
				return tex.has_uniform() && tex.uniform().SerializeAsString() == uniformKey.SerializeAsString();
			});
		if (itr == textures.end())
		{
			auto texDef = textures.Add();
			*texDef->mutable_uniform() = uniformKey;
			itr = textures.end() - 1;
		}
		return setTexturePath(textures, itr, texturePath);
	}

	bool Material::valid() const noexcept
	{
		return program && isValid(program->getHandle(programDefines));
	}

	Material::Material(std::shared_ptr<Program> prog, std::shared_ptr<Texture> tex) noexcept
		: program{ prog }
		, textures{ { Material::TextureDefinition::BaseColor, tex} }
	{
	}

	Material::Material(std::shared_ptr<Program> prog, const Color& color) noexcept
		: program{ prog }
		, baseColor{ color }
	{
	}

	Material::Definition Material::createDefinition() noexcept
	{
		Definition def;
		def.mutable_program()->set_standard(Program::Standard::Forward);
		*def.mutable_base_color() = convert<protobuf::Color>(Colors::white());
		def.set_opacity_type(Material::Definition::Opaque);
		def.set_shininess(32.0f);
		return def;
	}

	void Material::renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, OptionalRef<const RenderConfig> optConfig) const noexcept
	{
		std::optional<RenderConfig> defConfig;
		if (!optConfig)
		{
			defConfig = RenderConfig::createDefault();
		}
		auto& config = optConfig ? *optConfig : *defConfig;
		glm::vec4 hasTextures{ 0 };

		for (const auto& [type, key] : config.textureUniformKeys)
		{
			auto itr = textures.find(type);
			std::shared_ptr<Texture> tex;
			if (itr != textures.end())
			{
				hasTextures.x += 1 << (int)type;
				tex = itr->second;
			}
			else
			{
				tex = config.defaultTexture;
			}
			config.uniformHandles.configure(encoder, key, tex);
		}

		// pbr
		if (config.defaultTexture)
		{
			encoder.setTexture(RenderSamplers::MATERIAL_ALBEDO_LUT, config.albedoLutSamplerUniform, config.defaultTexture->getHandle());
		}
		auto val = Colors::normalize(baseColor);
		encoder.setUniform(config.baseColorUniform, glm::value_ptr(val));
		val = glm::vec4{ metallicFactor, roughnessFactor, normalScale, occlusionStrength };
		encoder.setUniform(config.metallicRoughnessNormalOcclusionUniform, glm::value_ptr(val));
		val = glm::vec4{ Colors::normalize(emissiveColor), 0 };
		encoder.setUniform(config.emissiveColorUniform, glm::value_ptr(val));
		val = glm::vec4{ multipleScattering ? 1.F : 0.F, whiteFurnanceFactor, 0, 0 };
		encoder.setUniform(config.multipleScatteringUniform, glm::value_ptr(val));

		// phong
		val = glm::vec4{ Colors::normalize(specularColor), shininess };
		encoder.setUniform(config.specularColorUniform, glm::value_ptr(val));

		encoder.setUniform(config.hasTexturesUniform, glm::value_ptr(hasTextures));
		config.basicUniforms.configure(encoder);
		config.uniformHandles.configure(encoder, uniformValues);

		uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;
		state = (state & ~BGFX_STATE_DEPTH_TEST_MASK) | BGFX_STATE_DEPTH_TEST_LEQUAL;
		if (!twoSided)
		{
			state |= BGFX_STATE_CULL_CCW;
		}
		if (primitiveType == Material::Definition::Line)
		{
			state &= ~BGFX_STATE_MSAA;
			state |= BGFX_STATE_PT_LINES;
			state |= BGFX_STATE_LINEAA;
		}
		auto opa = opacityType;
		if (opa == Material::Definition::Transparent || opa == Material::Definition::Mask)
		{
			state |= BGFX_STATE_BLEND_ALPHA;
		}
		else
		{
			state &= ~BGFX_STATE_WRITE_A;
		}

		encoder.setState(state);
		auto prog = program->getHandle(programDefines);
		encoder.submit(viewId, prog);
	}

	MaterialRenderConfig MaterialRenderConfig::createDefault() noexcept
	{
		MaterialRenderConfig config;
		config.textureUniformKeys = std::unordered_map<TextureType, TextureUniformKey>{
			{ Material::TextureDefinition::BaseColor, Texture::createUniformKey("s_texBaseColor" , RenderSamplers::MATERIAL_ALBEDO)},
			{ Material::TextureDefinition::MetallicRoughness, Texture::createUniformKey("s_texMetallicRoughness", RenderSamplers::MATERIAL_METALLIC_ROUGHNESS) },
			{ Material::TextureDefinition::Normal, Texture::createUniformKey("s_texNormal", RenderSamplers::MATERIAL_NORMAL) },
			{ Material::TextureDefinition::Occlusion, Texture::createUniformKey("s_texOcclusion", RenderSamplers::MATERIAL_OCCLUSION) },
			{ Material::TextureDefinition::Emissive, Texture::createUniformKey("s_texEmissive", RenderSamplers::MATERIAL_EMISSIVE) },
			{ Material::TextureDefinition::Specular, Texture::createUniformKey("s_texSpecular", RenderSamplers::MATERIAL_SPECULAR)},
		};
		config.albedoLutSamplerUniform = { "s_texAlbedoLUT", bgfx::UniformType::Sampler };
		config.baseColorUniform = { "u_baseColorFactor", bgfx::UniformType::Vec4 };
		config.specularColorUniform = { "u_specularFactorVec", bgfx::UniformType::Vec4 };
		config.metallicRoughnessNormalOcclusionUniform = { "u_metallicRoughnessNormalOcclusionFactor", bgfx::UniformType::Vec4 };
		config.emissiveColorUniform = { "u_emissiveFactorVec", bgfx::UniformType::Vec4 };
		config.hasTexturesUniform = { "u_hasTextures", bgfx::UniformType::Vec4 };
		config.multipleScatteringUniform = { "u_multipleScatteringVec", bgfx::UniformType::Vec4 };

		return config;
	}


	void MaterialRenderConfig::reset() noexcept
	{
		albedoLutSamplerUniform.reset();
		baseColorUniform.reset();
		specularColorUniform.reset();
		metallicRoughnessNormalOcclusionUniform.reset();
		emissiveColorUniform.reset();
		hasTexturesUniform.reset();
		multipleScatteringUniform.reset();
		textureUniformKeys.clear();
		defaultTexture.reset();
		basicUniforms.clear();
		uniformHandles.clear();
	}

	MaterialLoader::MaterialLoader(IMaterialDefinitionLoader& defLoader, IProgramLoader& progLoader, ITextureLoader& texLoader) noexcept
		: FromDefinitionLoader<IMaterialFromDefinitionLoader, IMaterialDefinitionLoader>(defLoader)
		, _progLoader{ progLoader }
		, _texLoader{ texLoader }
	{
	}

	MaterialLoader::Result MaterialLoader::create(std::shared_ptr<Definition> def) noexcept
	{
		if (!def)
		{
			return unexpected{ "null definition pointer" };
		}
		auto mat = std::make_shared<Material>();
	
		mat->programDefines = ProgramDefines(def->program_defines().begin(), def->program_defines().end());
		mat->baseColor = convert<Color>(def->base_color());
		mat->emissiveColor = convert<Color3>(def->emissive_color());
		mat->metallicFactor = def->metallic_factor();
		mat->roughnessFactor = def->roughness_factor();
		mat->normalScale = def->normal_scale();
		mat->occlusionStrength = def->occlusion_strength();
		mat->multipleScattering = def->multiple_scattering();
		mat->whiteFurnanceFactor = def->white_furnance_factor();
		mat->specularColor = convert<Color3>(def->specular_color());
		mat->shininess = def->shininess();
		mat->opacityType = def->opacity_type();
		mat->twoSided = def->twosided();
		mat->primitiveType = def->primitive_type();

		auto progResult = Program::loadRef(def->program(), _progLoader);
		if (!progResult)
		{
			return unexpected{ "failed to load program: " + progResult.error() };
		}
		mat->program = progResult.value();

		for (auto& defTex : def->textures())
		{
			auto loadResult = _texLoader(defTex.texture_path());
			if (!loadResult)
			{
				return unexpected{ "failed to load texture: " + loadResult.error() };
			}
			auto tex = loadResult.value();
			if (defTex.has_type())
			{
				mat->textures[defTex.type()] = tex;
			}
			else if (defTex.has_uniform())
			{
				mat->uniformTextures[defTex.uniform()] = tex;
			}
		}

		auto& uniformValues = mat->uniformValues;
		uniformValues.reserve(def->uniform_values_size());
		for (auto& [key, defVal] : def->uniform_values())
		{
			uniformValues.emplace(key, defVal);			
		}

		return mat;
	}

	expected<void, std::string> MaterialAppComponent::init(App& app) noexcept
	{
		_renderConfig = RenderConfig::createDefault();
		auto texResult = Texture::load(Image{ Colors::cyan(), app.getAssets().getAllocator() });
		if (!texResult)
		{
			return unexpected{ std::move(texResult).error() };
		}
		_renderConfig->defaultTexture = std::make_shared<Texture>(std::move(texResult).value());
		return {};
	}

	expected<void, std::string> MaterialAppComponent::update(float deltaTime) noexcept
	{
		if (_renderConfig)
		{
			_renderConfig->basicUniforms.update(deltaTime);
		}
		return {};
	}

	expected<void, std::string> MaterialAppComponent::shutdown() noexcept
	{
		_renderConfig.reset();
		return {};
	}

	void MaterialAppComponent::renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, const Material& material) const noexcept
	{
		if (_renderConfig)
		{
			material.renderSubmit(viewId, encoder, *_renderConfig);
		}
	}
}
