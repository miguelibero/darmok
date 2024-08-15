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

	struct Material::StaticConfig final
	{
	public:
		StaticConfig(App& app)
		{
			samplerUniforms = std::vector<Sampler>{
				{ TextureType::Base, bgfx::createUniform("s_texBaseColor", bgfx::UniformType::Sampler), RenderSamplers::PBR_BASECOLOR},
				{ TextureType::MetallicRoughness, bgfx::createUniform("s_texMetallicRoughness", bgfx::UniformType::Sampler), RenderSamplers::PBR_METALROUGHNESS},
				{ TextureType::Normal, bgfx::createUniform("s_texNormal", bgfx::UniformType::Sampler), RenderSamplers::PBR_NORMAL},
				{ TextureType::Occlusion, bgfx::createUniform("s_texOcclusion", bgfx::UniformType::Sampler), RenderSamplers::PBR_OCCLUSION},
				{ TextureType::Emissive, bgfx::createUniform("s_texEmissive", bgfx::UniformType::Sampler), RenderSamplers::PBR_EMISSIVE},
			};
			baseColorUniform = bgfx::createUniform("u_baseColorFactor", bgfx::UniformType::Vec4);
			metallicRoughnessNormalOcclusionUniform = bgfx::createUniform("u_metallicRoughnessNormalOcclusionFactor", bgfx::UniformType::Vec4);
			emissiveColorUniform = bgfx::createUniform("u_emissiveFactorVec", bgfx::UniformType::Vec4);
			hasTexturesUniform = bgfx::createUniform("u_hasTextures", bgfx::UniformType::Vec4);
			multipleScatteringUniform = bgfx::createUniform("u_multipleScatteringVec", bgfx::UniformType::Vec4);

			Image img(Colors::white(), app.getAssets().getAllocator());
			defaultTexture = std::make_shared<Texture>(img);
		}

		~StaticConfig()
		{
			for (auto& elm : samplerUniforms)
			{
				bgfx::destroy(elm.handle);
			}
			bgfx::destroy(baseColorUniform);
			bgfx::destroy(metallicRoughnessNormalOcclusionUniform);
			bgfx::destroy(emissiveColorUniform);
			bgfx::destroy(hasTexturesUniform);
			bgfx::destroy(multipleScatteringUniform);

			defaultTexture.reset();
		}

		using TextureType = MaterialTextureType;

		struct Sampler final
		{
			TextureType type;
			bgfx::UniformHandle handle;
			uint8_t stage;
		};

		std::vector<Sampler> samplerUniforms;
		bgfx::UniformHandle baseColorUniform;
		bgfx::UniformHandle metallicRoughnessNormalOcclusionUniform;
		bgfx::UniformHandle emissiveColorUniform;
		bgfx::UniformHandle hasTexturesUniform;
		bgfx::UniformHandle multipleScatteringUniform;

		std::shared_ptr<Texture> defaultTexture;
	};

	std::unique_ptr<Material::StaticConfig> Material::_staticConfig;

	void Material::staticInit(App& app) noexcept
	{
		_staticConfig = std::make_unique<StaticConfig>(app);
	}

	void Material::staticShutdown() noexcept
	{
		_staticConfig.reset();
	}

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

	void Material::renderSubmit(bgfx::ViewId viewId, bgfx::Encoder& encoder) const noexcept
	{
		if (_staticConfig == nullptr)
		{
			return;
		}
		glm::vec4 hasTextures(0);
		for (uint8_t i = 0; i < _staticConfig->samplerUniforms.size(); i++)
		{
			auto& def = _staticConfig->samplerUniforms[i];
			auto tex = getTexture(def.type);
			if (!tex)
			{
				tex = _staticConfig->defaultTexture;
			}
			hasTextures.x += 1 << i;
			encoder.setTexture(def.stage, def.handle, tex->getHandle());
		}
		encoder.setUniform(_staticConfig->hasTexturesUniform, glm::value_ptr(hasTextures));

		auto v = Colors::normalize(getBaseColor());
		encoder.setUniform(_staticConfig->baseColorUniform, glm::value_ptr(v));
		v = glm::vec4(getMetallicFactor(), getRoughnessFactor(), getNormalScale(), getOcclusionStrength());
		encoder.setUniform(_staticConfig->metallicRoughnessNormalOcclusionUniform, glm::value_ptr(v));
		v = glm::vec4(getEmissiveColor(), 0);
		encoder.setUniform(_staticConfig->emissiveColorUniform, glm::value_ptr(v));
		v = glm::vec4(getMultipleScattering() ? 1.F : 0.F, getWhiteFurnanceFactor(), 0, 0);
		encoder.setUniform(_staticConfig->multipleScatteringUniform, glm::value_ptr(v));

		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CCW
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_ALPHA
			;

		if (getPrimitiveType() == MaterialPrimitiveType::Line)
		{
			state |= BGFX_STATE_PT_LINES;
		}

		encoder.setState(state);
		encoder.submit(viewId, getProgramHandle());
	}
}
