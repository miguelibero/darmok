#include <darmok/material.hpp>
#include <darmok/light.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>
#include <darmok/image.hpp>
#include <darmok/app.hpp>
#include <darmok/string.hpp>
#include <darmok/reflect_serialize.hpp>

#include <glm/gtc/type_ptr.hpp>
#include "render_samplers.hpp"

namespace darmok
{
	Material::Material(const std::shared_ptr<Program>& program) noexcept
		: _primitive(MaterialPrimitiveType::Triangle)
		, _program(program)
		, _baseColor(Colors::white())
		, _specularColor(Colors::black3())
		, _metallicFactor(defaultMetallicFactor)
		, _roughnessFactor(defaultRoughnessFactor)
		, _normalScale(defaultNormalScale)
		, _occlusionStrength(defaultOcclusionStrength)
		, _emissiveColor(Colors::black3())
		, _multipleScattering(false)
		, _whiteFurnance(defaultWhiteFurnance)
		, _opacityType(OpacityType::Transparent)
		, _shininess(defaultShininess)
		, _twoSided(false)
		, _uniforms(false)
		, _textureUniforms(false)
	{
		if (_program == nullptr)
		{
			_program = StandardProgramLoader::load(StandardProgramType::Unlit);
		}
	}

	Material::Material(const std::shared_ptr<Texture>& texture) noexcept
		: Material(nullptr, texture)
	{
	}

	Material::Material(const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& texture) noexcept
		: Material(program)
	{
		if (texture != nullptr)
		{
			setTexture(texture);
		}
	}

	Material::Material(const std::shared_ptr<Program>& program, const Color& color) noexcept
		: Material(program)
	{
		setBaseColor(color);
	}

	bool Material::valid() const noexcept
	{
		return isValid(_program->getHandle(_programDefines));
	}

	const std::string& Material::getName() const noexcept
	{
		return _name;
	}

	Material& Material::setName(const std::string& name) noexcept
	{
		_name = name;
		return *this;
	}

	const ProgramDefines& Material::getProgramDefines() noexcept
	{
		return _programDefines;
	}

	Material& Material::setProgramDefines(const ProgramDefines& defines) noexcept
	{
		_programDefines = defines;
		return *this;
	}

	Material& Material::setProgramDefine(const std::string& define, bool enabled) noexcept
	{
		if (enabled)
		{
			_programDefines.insert(define);
		}
		else
		{
			_programDefines.erase(define);
		}
		return *this;
	}

	std::shared_ptr<Program> Material::getProgram() const noexcept
	{
		return _program;
	}

	Material& Material::setProgram(const std::shared_ptr<Program>& prog) noexcept
	{
		_program = prog;
		return *this;
	}

	bgfx::ProgramHandle Material::getProgramHandle() const noexcept
	{
		if (_program == nullptr)
		{
			return { bgfx::kInvalidHandle };
		}
		return _program->getHandle(_programDefines);
	}

	const Material::Textures& Material::getTextures() const noexcept
	{
		return _textures;
	}

	std::shared_ptr<Texture> Material::getTexture(TextureType type) const noexcept
	{
		auto itr = _textures.find(type);
		if (itr == _textures.end())
		{
			return nullptr;
		}
		return itr->second;
	}

	Material& Material::setTexture(const std::shared_ptr<Texture>& texture) noexcept
	{
		return setTexture(TextureType::BaseColor, texture);
	}

	Material& Material::setTexture(TextureType type, const std::shared_ptr<Texture>& texture) noexcept
	{
		_textures[type] = texture;
		return *this;
	}

	Material& Material::setTexture(const std::string& name, uint8_t stage, const std::shared_ptr<Texture>& texture) noexcept
	{
		_textureUniforms.set(name, stage, texture);
		return *this;
	}

	Material& Material::setUniform(const std::string& name, std::optional<UniformValue> value) noexcept
	{
		_uniforms.set(name, value);
		return *this;
	}

	const UniformContainer& Material::getUniformContainer() const noexcept
	{
		return _uniforms;
	}

	UniformContainer& Material::getUniformContainer() noexcept
	{
		return _uniforms;
	}

	const TextureUniformContainer& Material::getTextureUniformContainer() const noexcept
	{
		return _textureUniforms;
	}

	TextureUniformContainer& Material::getTextureUniformContainer() noexcept
	{
		return _textureUniforms;
	}

	Material::PrimitiveType Material::getPrimitiveType() const noexcept
	{
		return _primitive;
	}

	Material& Material::setPrimitiveType(PrimitiveType type) noexcept
	{
		_primitive = type;
		return *this;
	}

	const Color& Material::getBaseColor() const noexcept
	{
		return _baseColor;
	}

	Material& Material::setBaseColor(const Color& v) noexcept
	{
		_baseColor = v;
		return *this;
	}

	const Color3& Material::getSpecularColor() const noexcept
	{
		return _specularColor;
	}

	Material& Material::setSpecularColor(const Color3& v) noexcept
	{
		_specularColor = v;
		return *this;
	}

	float Material::getMetallicFactor() const noexcept
	{
		return _metallicFactor;
	}

	Material& Material::setMetallicFactor(float v) noexcept
	{
		_metallicFactor = v;
		return *this;
	}

	float Material::getRoughnessFactor() const noexcept
	{
		return _roughnessFactor;
	}

	Material& Material::setRoughnessFactor(float v) noexcept
	{
		_roughnessFactor = v;
		return *this;
	}

	float Material::getNormalScale() const noexcept
	{
		return _normalScale;
	}

	Material& Material::setNormalScale(float v) noexcept
	{
		_normalScale = v;
		return *this;
	}

	float Material::getOcclusionStrength() const noexcept
	{
		return _occlusionStrength;
	}

	Material& Material::setOcclusionStrength(float v) noexcept
	{
		_occlusionStrength = v;
		return *this;
	}

	const Color3& Material::getEmissiveColor() const noexcept
	{
		return _emissiveColor;
	}

	Material& Material::setEmissiveColor(const Color3& v) noexcept
	{
		_emissiveColor = v;
		return *this;
	}

	Material& Material::setTwoSided(bool enabled) noexcept
	{
		_twoSided = enabled;
		return *this;
	}

	bool Material::getTwoSided() const noexcept
	{
		return _twoSided;
	}

	Material& Material::setMultipleScattering(bool enabled) noexcept
	{
		_multipleScattering = enabled;
		return *this;
	}

	bool Material::getMultipleScattering() const noexcept
	{
		return _multipleScattering;
	}

	Material& Material::setWhiteFurnanceFactor(float v) noexcept
	{
		_whiteFurnance = v;
		return *this;
	}

	float Material::getWhiteFurnanceFactor() const noexcept
	{
		return _whiteFurnance;
	}

	uint16_t Material::getShininess() const noexcept
	{
		return _shininess;
	}

	Material& Material::setShininess(uint16_t val) noexcept
	{
		_shininess = val;
		return *this;
	}

	OpacityType Material::getOpacityType() const noexcept
	{
		return _opacityType;
	}

	Material& Material::setOpacityType(OpacityType val) noexcept
	{
		_opacityType = val;
		return *this;
	}

	const std::array<std::string, toUnderlying(OpacityType::Count)> Material::_opacityNames
	{
		"Opaque",
		"Mask",
		"Transparent",
	};

	const std::array<std::string, toUnderlying(Material::PrimitiveType::Count)> Material::_primitiveTypeNames
	{
		"Triangle",
		"Line",
	};

	const std::array<std::string, toUnderlying(Material::TextureType::Count)> Material::_texTypeNames
	{
		"BaseColor",
		"Specular",
		"MetallicRoughness",
		"Normal",
		"Occlusion",
		"Emissive",
	};

	std::optional<OpacityType> Material::readOpacityType(std::string_view name) noexcept
	{
		return StringUtils::readEnum<OpacityType>(name, _opacityNames);
	}

	const std::string& Material::getOpacityTypeName(OpacityType opacity) noexcept
	{
		return StringUtils::getEnumName(toUnderlying(opacity), _opacityNames);
	}

	std::optional<Material::PrimitiveType> Material::readPrimitiveType(std::string_view name) noexcept
	{
		return StringUtils::readEnum<PrimitiveType>(name, _primitiveTypeNames);
	}

	const std::string& Material::getPrimitiveTypeName(PrimitiveType prim) noexcept
	{
		return StringUtils::getEnumName(toUnderlying(prim), _primitiveTypeNames);
	}

	std::optional<Material::TextureType> Material::readTextureType(std::string_view name) noexcept
	{
		return StringUtils::readEnum<TextureType>(name, _texTypeNames);
	}

	const std::string& Material::getTextureTypeName(TextureType tex) noexcept
	{
		return StringUtils::getEnumName(toUnderlying(tex), _texTypeNames);
	}

	void Material::bindMeta()
	{
		ReflectionSerializeUtils::metaSerialize<Material>();
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
		_samplerUniforms = std::vector<Sampler>{
			{ TextureType::BaseColor, bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler), RenderSamplers::MATERIAL_ALBEDO},
			{ TextureType::Specular, bgfx::createUniform("s_texSpecular", bgfx::UniformType::Sampler), RenderSamplers::MATERIAL_SPECULAR},
			{ TextureType::MetallicRoughness, bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler), RenderSamplers::MATERIAL_METALLIC_ROUGHNESS},
			{ TextureType::Normal, bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler), RenderSamplers::MATERIAL_NORMAL},
			{ TextureType::Occlusion, bgfx::createUniform("s_texOcclusion", bgfx::UniformType::Sampler), RenderSamplers::MATERIAL_OCCLUSION},
			{ TextureType::Emissive, bgfx::createUniform("s_texEmissive", bgfx::UniformType::Sampler), RenderSamplers::MATERIAL_EMISSIVE},
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
		for (auto& elm : _samplerUniforms)
		{
			bgfx::destroy(elm.handle);
			elm.handle.idx = bgfx::kInvalidHandle;
		}
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
	}

	void MaterialAppComponent::renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, const Material& mat) const noexcept
	{
		glm::vec4 hasTextures(0);
		for (const auto& def : _samplerUniforms)
		{
			auto tex = mat.getTexture(def.type);
			if (tex)
			{
				hasTextures.x += 1 << (int)def.type;
			}
			else
			{
				tex = _defaultTexture;
			}
			encoder.setTexture(def.stage, def.handle, tex->getHandle());
		}
		encoder.setUniform(_hasTexturesUniform, glm::value_ptr(hasTextures));
		encoder.setTexture(RenderSamplers::MATERIAL_ALBEDO_LUT, _albedoLutSamplerUniform, _defaultTexture->getHandle());

		auto val = Colors::normalize(mat.getBaseColor());
		encoder.setUniform(_baseColorUniform, glm::value_ptr(val));
		val = glm::vec4(Colors::normalize(mat.getSpecularColor()), mat.getShininess());
		encoder.setUniform(_specularColorUniform, glm::value_ptr(val));
		
		val = glm::vec4(mat.getMetallicFactor(), mat.getRoughnessFactor(), mat.getNormalScale(), mat.getOcclusionStrength());
		encoder.setUniform(_metallicRoughnessNormalOcclusionUniform, glm::value_ptr(val));
		val = glm::vec4(Colors::normalize(mat.getEmissiveColor()), 0);
		encoder.setUniform(_emissiveColorUniform, glm::value_ptr(val));
		val = glm::vec4(mat.getMultipleScattering() ? 1.F : 0.F, mat.getWhiteFurnanceFactor(), 0, 0);
		encoder.setUniform(_multipleScatteringUniform, glm::value_ptr(val));
		_basicUniforms.configure(encoder);

		mat.getUniformContainer().configure(encoder);
		mat.getTextureUniformContainer().configure(encoder);

		uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;
		state = (state & ~BGFX_STATE_DEPTH_TEST_MASK) | BGFX_STATE_DEPTH_TEST_LEQUAL;
		if (!mat.getTwoSided())
		{
			state |= BGFX_STATE_CULL_CCW;
		}
		if (mat.getPrimitiveType() == MaterialPrimitiveType::Line)
		{
			state &= ~BGFX_STATE_MSAA;
			state |= BGFX_STATE_PT_LINES;
			state |= BGFX_STATE_LINEAA;
		}
		auto opa = mat.getOpacityType();
		if (opa == OpacityType::Transparent || opa == OpacityType::Mask)
		{
			state |= BGFX_STATE_BLEND_ALPHA;
		}
		else
		{
			state &= ~BGFX_STATE_WRITE_A;
		}
		
		encoder.setState(state);
		auto prog = mat.getProgramHandle();
		encoder.submit(viewId, prog);
	}
}
