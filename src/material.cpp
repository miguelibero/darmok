#include <darmok/material.hpp>
#include <darmok/light.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>
#include <darmok/image.hpp>
#include <darmok/app.hpp>
#include <darmok/asset.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "render_samplers.hpp"

namespace darmok
{
	Material::Material(const std::shared_ptr<Program>& program) noexcept
		: _primitive(MaterialPrimitiveType::Triangle)
		, _program(program)
		, _baseColor(Colors::white())
		, _specularColor(Colors::black3())
		, _metallicFactor(0.F)
		, _roughnessFactor(0.F)
		, _normalScale(1.F)
		, _occlusionStrength(0.F)
		, _emissiveColor(Colors::black3())
		, _multipleScattering(false)
		, _whiteFurnance(0.F)
		, _opacity(Opacity::Transparent)
		, _shininess(32)
		, _twoSided(false)
	{
		if (_program == nullptr)
		{
			_program = std::make_shared<Program>(StandardProgramType::Unlit);
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

	Material& Material::setShininess(uint16_t v) noexcept
	{
		_shininess = v;
		return *this;
	}

	Material::Opacity Material::getOpacity() const noexcept
	{
		return _opacity;
	}

	Material& Material::setOpacity(Opacity v) noexcept
	{
		_opacity = v;
		return *this;
	}

	void MaterialRenderComponent::init(Camera& cam, Scene& scene, App& app)
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

		Image img(Colors::white(), app.getAssets().getAllocator());
		_defaultTexture = std::make_shared<Texture>(img);
	}

	void MaterialRenderComponent::shutdown()
	{
		for (auto& elm : _samplerUniforms)
		{
			bgfx::destroy(elm.handle);
			elm.handle.idx = bgfx::kInvalidHandle;
		}
		std::vector<std::reference_wrapper<bgfx::UniformHandle>> uniforms = {
			_albedoLutSamplerUniform, _baseColorUniform, _specularColorUniform,
			_metallicRoughnessNormalOcclusionUniform, _emissiveColorUniform,
			_hasTexturesUniform, _multipleScatteringUniform
		};
		for (auto& uniform : uniforms)
		{
			if (isValid(uniform))
			{
				bgfx::destroy(uniform);
				uniform.get().idx = bgfx::kInvalidHandle;
			}
		}

		_defaultTexture.reset();
	}

	void MaterialRenderComponent::renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, const Material& mat) const noexcept
	{
		glm::vec4 hasTextures(0);
		for (uint8_t i = 0; i < _samplerUniforms.size(); i++)
		{
			auto& def = _samplerUniforms[i];
			auto tex = mat.getTexture(def.type);
			if (tex)
			{
				hasTextures.x += 1 << i;
			}
			else
			{
				tex = _defaultTexture;
			}
			encoder.setTexture(def.stage, def.handle, tex->getHandle());
		}
		encoder.setUniform(_hasTexturesUniform, glm::value_ptr(hasTextures));

		encoder.setTexture(RenderSamplers::MATERIAL_ALBEDO_LUT, _albedoLutSamplerUniform, _defaultTexture->getHandle());

		auto v = Colors::normalize(mat.getBaseColor());
		encoder.setUniform(_baseColorUniform, glm::value_ptr(v));
		v = glm::vec4(Colors::normalize(mat.getSpecularColor()), mat.getShininess());
		encoder.setUniform(_specularColorUniform, glm::value_ptr(v));
		
		v = glm::vec4(mat.getMetallicFactor(), mat.getRoughnessFactor(), mat.getNormalScale(), mat.getOcclusionStrength());
		encoder.setUniform(_metallicRoughnessNormalOcclusionUniform, glm::value_ptr(v));
		v = glm::vec4(Colors::normalize(mat.getEmissiveColor()), 0);
		encoder.setUniform(_emissiveColorUniform, glm::value_ptr(v));
		v = glm::vec4(mat.getMultipleScattering() ? 1.F : 0.F, mat.getWhiteFurnanceFactor(), 0, 0);
		encoder.setUniform(_multipleScatteringUniform, glm::value_ptr(v));

		uint64_t state = BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK;
		if (!mat.getTwoSided())
		{
			state |= BGFX_STATE_CULL_CCW;
		}
		if (mat.getPrimitiveType() == MaterialPrimitiveType::Line)
		{
			state |= BGFX_STATE_PT_LINES;
		}
		if (mat.getOpacity() == MaterialOpacity::Transparent)
		{
			state |= BGFX_STATE_BLEND_ALPHA;
			state &= ~BGFX_STATE_DEPTH_TEST_MASK;
			state |= BGFX_STATE_DEPTH_TEST_LEQUAL;
		}
		else if (mat.getOpacity() == MaterialOpacity::Cutout)
		{
			state |= BGFX_STATE_BLEND_ALPHA;
		}
		encoder.setState(state);
		auto prog = mat.getProgramHandle();
		encoder.submit(viewId, prog);
	}
}
