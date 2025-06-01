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
#include "render_samplers.hpp"

namespace darmok
{
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

	MaterialLoader::MaterialLoader(IMaterialDefinitionLoader& defLoader, ILoader<Program>& progLoader, ILoader<Texture>& texLoader) noexcept
		: FromDefinitionLoader<IMaterialLoader, IMaterialDefinitionLoader>(defLoader)
		, _progLoader{ progLoader }
		, _texLoader{ texLoader }
	{
	}

	MaterialLoader::Result MaterialLoader::create(const std::shared_ptr<Definition>& def)
	{
		if (!def)
		{
			return unexpected{ "null definition pointer" };
		}
		auto mat = std::make_shared<Material>();
	
		mat->programDefines = ProgramDefines(def->program_defines().begin(), def->program_defines().end());
		mat->baseColor = protobuf::convert(def->base_color());
		mat->emissiveColor = protobuf::convert(def->emissive_color());
		mat->metallicFactor = def->metallic_factor();
		mat->roughnessFactor = def->roughness_factor();
		mat->normalScale = def->normal_scale();
		mat->occlusionStrength = def->occlusion_strength();
		mat->multipleScattering = def->multiple_scattering();
		mat->whiteFurnanceFactor = def->white_furnance_factor();
		mat->specularColor = protobuf::convert(def->specular_color());
		mat->shininess = def->shininess();
		mat->opacityType = def->opacity_type();
		mat->twoSided = def->twosided();
		mat->primitiveType = def->primitive_type();

		if (def->has_standard_program())
		{
			mat->program = StandardProgramLoader::load(def->standard_program());
		}
		else if(!def->program_path().empty())
		{
			auto loadResult = _progLoader(def->program_path());
			if (!loadResult)
			{
				return unexpected{ "failed to load custom program" };
			}
			mat->program = loadResult.value();
		}

		for (auto& defTex : def->textures())
		{
			auto loadResult = _texLoader(defTex.texture_path());
			if (!loadResult)
			{
				return unexpected{ "failed to load texture" };
			}
			auto tex = loadResult.value();
			if (defTex.has_type())
			{
				mat->textures[defTex.type()] = tex;
			}
			else if (defTex.has_uniform())
			{
				TextureUniformKey key;
				key.name = defTex.uniform().name();
				key.stage = defTex.uniform().stage();
				mat->uniformTextures[key] = tex;
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

	MaterialAppComponent::MaterialAppComponent() noexcept
		: _albedoLutSamplerUniform{ bgfx::kInvalidHandle }
		, _baseColorUniform{ bgfx::kInvalidHandle }
		, _specularColorUniform{ bgfx::kInvalidHandle }
		, _metallicRoughnessNormalOcclusionUniform{ bgfx::kInvalidHandle }
		, _emissiveColorUniform{ bgfx::kInvalidHandle }
		, _hasTexturesUniform{ bgfx::kInvalidHandle }
		, _multipleScatteringUniform{ bgfx::kInvalidHandle }
	{
	}

	MaterialAppComponent::~MaterialAppComponent() noexcept
	{
		if (_defaultTexture)
		{
			shutdown();
		}
	}

	void MaterialAppComponent::init(App& app)
	{
		_textureUniformKeys = std::unordered_map<TextureType, TextureUniformKey>{
			{ Material::TextureDefinition::BaseColor, {"s_texBaseColor" , RenderSamplers::MATERIAL_ALBEDO}},
			{ Material::TextureDefinition::MetallicRoughness, { "s_texMetallicRoughness", RenderSamplers::MATERIAL_METALLIC_ROUGHNESS}},
			{ Material::TextureDefinition::Normal, { "s_texNormal", RenderSamplers::MATERIAL_NORMAL}},
			{ Material::TextureDefinition::Occlusion, {"s_texOcclusion", RenderSamplers::MATERIAL_OCCLUSION}},
			{ Material::TextureDefinition::Emissive, { "s_texEmissive", RenderSamplers::MATERIAL_EMISSIVE}},
			{ Material::TextureDefinition::Specular, { "s_texSpecular", RenderSamplers::MATERIAL_SPECULAR}},
		};
		_albedoLutSamplerUniform = bgfx::createUniform("s_texAlbedoLUT", bgfx::UniformType::Sampler);
		_baseColorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
		_specularColorUniform = bgfx::createUniform("u_specularFactorVec", bgfx::UniformType::Vec4);
		_metallicRoughnessNormalOcclusionUniform = bgfx::createUniform("u_metallicRoughnessNormalOcclusionFactor", bgfx::UniformType::Vec4);
		_emissiveColorUniform = bgfx::createUniform("u_emissiveFactorVec", bgfx::UniformType::Vec4);
		_hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
		_multipleScatteringUniform = bgfx::createUniform("u_multipleScatteringVec", bgfx::UniformType::Vec4);
		_basicUniforms.init();

		const Image img(Colors::white(), app.getAssets().getAllocator());
		_defaultTexture = std::make_shared<Texture>(img);
	}

	void MaterialAppComponent::update(float deltaTime)
	{
		if (!_defaultTexture)
		{
			return;
		}
		_basicUniforms.update(deltaTime);
	}

	void MaterialAppComponent::shutdown()
	{
		const std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms = {
			_albedoLutSamplerUniform, _baseColorUniform, _specularColorUniform,
			_metallicRoughnessNormalOcclusionUniform, _emissiveColorUniform,
			_hasTexturesUniform, _multipleScatteringUniform
		};
		for (const auto& uniform : uniforms)
		{
			if (isValid(uniform))
			{
				bgfx::destroy(uniform);
				uniform.get().idx = bgfx::kInvalidHandle;
			}
		}
		_basicUniforms.shutdown();
		_defaultTexture.reset();
		_uniformHandles.shutdown();
	}

	void MaterialAppComponent::renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, const Material& mat) const noexcept
	{
		glm::vec4 hasTextures{ 0 };

		for (const auto& [type, key] : _textureUniformKeys)
		{
			auto itr = mat.textures.find(type);
			std::shared_ptr<Texture> tex;
			if (itr != mat.textures.end())
			{
				hasTextures.x += 1 << (int)type;
				tex = itr->second;
			}
			else
			{
				tex = _defaultTexture;
			}
			_uniformHandles.configure(encoder, key, tex);
		}

		// pbr
		encoder.setTexture(RenderSamplers::MATERIAL_ALBEDO_LUT, _albedoLutSamplerUniform, _defaultTexture->getHandle());
		auto val = Colors::normalize(mat.baseColor);
		encoder.setUniform(_baseColorUniform, glm::value_ptr(val));
		val = glm::vec4{ mat.metallicFactor, mat.roughnessFactor, mat.normalScale, mat.occlusionStrength };
		encoder.setUniform(_metallicRoughnessNormalOcclusionUniform, glm::value_ptr(val));
		val = glm::vec4{ Colors::normalize(mat.emissiveColor), 0 };
		encoder.setUniform(_emissiveColorUniform, glm::value_ptr(val));
		val = glm::vec4{ mat.multipleScattering ? 1.F : 0.F, mat.whiteFurnanceFactor, 0, 0 };
		encoder.setUniform(_multipleScatteringUniform, glm::value_ptr(val));
		
		// phong
		val = glm::vec4{ Colors::normalize(mat.specularColor), mat.shininess };
		encoder.setUniform(_specularColorUniform, glm::value_ptr(val));

		encoder.setUniform(_hasTexturesUniform, glm::value_ptr(hasTextures));
		_basicUniforms.configure(encoder);
		_uniformHandles.configure(encoder, mat.uniformValues);

		uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;
		state = (state & ~BGFX_STATE_DEPTH_TEST_MASK) | BGFX_STATE_DEPTH_TEST_LEQUAL;
		if (!mat.twoSided)
		{
			state |= BGFX_STATE_CULL_CCW;
		}
		if (mat.primitiveType == Material::Definition::Line)
		{
			state &= ~BGFX_STATE_MSAA;
			state |= BGFX_STATE_PT_LINES;
			state |= BGFX_STATE_LINEAA;
		}
		auto opa = mat.opacityType;
		if (opa == Material::Definition::Transparent || opa == Material::Definition::Mask)
		{
			state |= BGFX_STATE_BLEND_ALPHA;
		}
		else
		{
			state &= ~BGFX_STATE_WRITE_A;
		}
		
		encoder.setState(state);
		auto prog = mat.program->getHandle(mat.programDefines);
		encoder.submit(viewId, prog);
	}
}
