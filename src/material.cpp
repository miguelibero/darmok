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
		, _baseColor(Colors::magenta())
		, _metallicFactor(0.F)
		, _roughnessFactor(0.F)
		, _normalScale(1.F)
		, _occlusionStrength(0.F)
		, _emissiveColor(Colors::white())
		, _multipleScattering(false)
		, _whiteFurnance(0.F)
		, _opaque(false)
		, _twoSided(false)
	{
		if (_program == nullptr)
		{
			_program = std::make_shared<Program>(StandardProgramType::Unlit);
		}
	}

	Material::Material(const std::shared_ptr<Texture>& diffuseTexture) noexcept
		: Material(nullptr, diffuseTexture)
	{
	}

	Material::Material(const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& diffuseTexture) noexcept
		: Material(program)
	{
		if (diffuseTexture != nullptr)
		{
			setTexture(TextureType::Base, diffuseTexture);
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
		return setTexture(TextureType::Base, texture);
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

	void MaterialRenderComponent::init(Camera& cam, Scene& scene, App& app)
	{
		_samplerUniforms = std::vector<Sampler>{
			{ TextureType::Base, bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler), RenderSamplers::PBR_BASECOLOR},
			{ TextureType::MetallicRoughness, bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler), RenderSamplers::PBR_METALROUGHNESS},
			{ TextureType::Normal, bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler), RenderSamplers::PBR_NORMAL},
			{ TextureType::Occlusion, bgfx::createUniform("s_texOcclusion", bgfx::UniformType::Sampler), RenderSamplers::PBR_OCCLUSION},
			{ TextureType::Emissive, bgfx::createUniform("s_texEmissive", bgfx::UniformType::Sampler), RenderSamplers::PBR_EMISSIVE},
		};
		_baseColorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
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
		}
		bgfx::destroy(_baseColorUniform);
		bgfx::destroy(_metallicRoughnessNormalOcclusionUniform);
		bgfx::destroy(_emissiveColorUniform);
		bgfx::destroy(_hasTexturesUniform);
		bgfx::destroy(_multipleScatteringUniform);

		_defaultTexture.reset();
	}

	void MaterialRenderComponent::renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder, const Material& mat) const noexcept
	{
		glm::vec4 hasTextures(0);
		for (uint8_t i = 0; i < _samplerUniforms.size(); i++)
		{
			auto& def = _samplerUniforms[i];
			auto tex = mat.getTexture(def.type);
			if (!tex)
			{
				tex = _defaultTexture;
			}
			hasTextures.x += 1 << i;
			encoder.setTexture(def.stage, def.handle, tex->getHandle());
		}
		encoder.setUniform(_hasTexturesUniform, glm::value_ptr(hasTextures));

		auto v = Colors::normalize(mat.getBaseColor());
		encoder.setUniform(_baseColorUniform, glm::value_ptr(v));
		v = glm::vec4(mat.getMetallicFactor(), mat.getRoughnessFactor(), mat.getNormalScale(), mat.getOcclusionStrength());
		encoder.setUniform(_metallicRoughnessNormalOcclusionUniform, glm::value_ptr(v));
		v = glm::vec4(mat.getEmissiveColor(), 0);
		encoder.setUniform(_emissiveColorUniform, glm::value_ptr(v));
		v = glm::vec4(mat.getMultipleScattering() ? 1.F : 0.F, mat.getWhiteFurnanceFactor(), 0, 0);
		encoder.setUniform(_multipleScatteringUniform, glm::value_ptr(v));

		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CCW
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_ALPHA
			;

		if (mat.getPrimitiveType() == MaterialPrimitiveType::Line)
		{
			state |= BGFX_STATE_PT_LINES;
		}

		encoder.setState(state);
		encoder.submit(viewId, mat.getProgramHandle());
	}
}
