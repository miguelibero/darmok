#include <darmok/material.hpp>

#include <glm/gtc/type_ptr.hpp>
#include "embedded_shader.hpp"
#include "generated/shaders/debug_vertex.h"
#include "generated/shaders/debug_fragment.h"
#include "generated/shaders/sprite_vertex.h"
#include "generated/shaders/sprite_fragment.h"
#include "generated/shaders/basic_vertex.h"
#include "generated/shaders/basic_fragment.h"

namespace darmok
{
	Material::Material(const std::shared_ptr<Program>& program, const ProgramDefinition& progDef) noexcept
	{
		setProgram(program, progDef);
	}

	Material::Material(const Material& other) noexcept
	{
		setProgram(other._program, other._progDef);
	}

	Material& Material::operator=(const Material& other) noexcept
	{
		setProgram(other._program, other._progDef);
		return *this;
	}

	Material::Material(Material&& other) noexcept
		: _program(other._program)
		, _progDef(other._progDef)
		, _uniforms(std::move(other._uniforms))
		, _textures(std::move(other._textures))
		, _colors(std::move(other._colors))
	{
		other._program = nullptr;
	}

	Material& Material::operator=(Material&& other) noexcept
	{
		_program = other._program;
		_progDef = other._progDef;
		_uniforms = std::move(other._uniforms);
		_textures = std::move(other._textures);
		_colors = std::move(other._colors);
		other._program = nullptr;
		return *this;
	}

	Material::~Material() noexcept
	{
		clearUniforms();
	}

	void Material::clearUniforms() noexcept
	{
		for (auto& pair : _uniforms)
		{
			bgfx::destroy(pair.second);
		}
		_uniforms.clear();
	}

	const std::shared_ptr<Program>& Material::getProgram() const noexcept
	{
		return _program;
	}

	void Material::setProgram(const std::shared_ptr<Program>& program, const ProgramDefinition& progDef) noexcept
	{
		clearUniforms();
		_program = program;
		_progDef = progDef;
		_vertexLayout = _progDef.createVertexLayout();
		for (auto& pair : progDef.uniforms)
		{
			auto handle = pair.second.createHandle();
			if (isValid(handle))
			{
				_uniforms[pair.first] = handle;
			}
		}
	}

	const bgfx::VertexLayout& Material::getVertexLayout() const noexcept
	{
		return _vertexLayout;
	}

	std::shared_ptr<Texture> Material::getTexture(MaterialTextureType type, uint8_t textureUnit) const noexcept
	{
		auto itr = _textures.find(type);
		if (itr == _textures.end())
		{
			return nullptr;
		}
		auto itr2 = itr->second.find(textureUnit);
		if (itr2 == itr->second.end())
		{
			return nullptr;
		}
		return itr2->second;
	}

	void Material::setTexture(MaterialTextureType type, const std::shared_ptr<Texture>& texture, uint8_t textureUnit) noexcept
	{
		_textures[type][textureUnit] = texture;
	}

	OptionalRef<const Color> Material::getColor(MaterialColorType type) const noexcept
	{
		auto itr = _colors.find(type);
		if (itr == _colors.end())
		{
			return std::nullopt;
		}
		return itr->second;
	}

	void Material::setColor(MaterialColorType type, const Color& color) noexcept
	{
		_colors[type] = color;
	}

	bgfx::UniformHandle Material::getUniformHandle(ProgramUniform uniform) const noexcept
	{
		auto itr = _uniforms.find(uniform);
		if (itr == _uniforms.end())
		{
			return { bgfx::kInvalidHandle };
		}
		return itr->second;
	}

	void Material::submit(bgfx::Encoder& encoder, bgfx::ViewId viewId) const
	{
		if (_program == nullptr)
		{
			throw std::runtime_error("material without program");
		}

		submitTextures(encoder);
		submitColors(encoder);
		
		// TODO: configure state
		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CCW
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
			;

		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle());
	}

	void Material::submitTextures(bgfx::Encoder& encoder) const
	{
		auto uniform = getUniformHandle(ProgramUniform::DiffuseTexture);
		if (isValid(uniform))
		{
			auto itr = _textures.find(MaterialTextureType::Diffuse);
			if (itr != _textures.end())
			{
				for (auto& pair : itr->second)
				{
					encoder.setTexture(pair.first, uniform, pair.second->getHandle());
				}
			}
		}
	}

	static const std::unordered_map<MaterialColorType, ProgramUniform> _materialColorUniforms = {
		{ MaterialColorType::Diffuse, ProgramUniform::DiffuseColor },
	};


	void Material::submitColors(bgfx::Encoder& encoder) const
	{
		for (auto& pair : _materialColorUniforms)
		{
			auto uniform = getUniformHandle(pair.second);
			if (!isValid(uniform))
			{
				continue;
			}
			auto itr = _colors.find(pair.first);
			if (itr != _colors.end())
			{
				auto v = itr->second.getVector();
				encoder.setUniform(uniform, glm::value_ptr(v));
			}
			else
			{
				auto itr = _progDef.uniforms.find(pair.second);
				if (itr != _progDef.uniforms.end())
				{
					auto& defVal = itr->second.defaultValue;
					if (!defVal.empty())
					{
						encoder.setUniform(uniform, defVal.ptr());
					}
				}
			}
		}
	}

	static const bgfx::EmbeddedShader _embeddedShaders[] =
	{
		BGFX_EMBEDDED_SHADER(debug_vertex),
		BGFX_EMBEDDED_SHADER(debug_fragment),
		BGFX_EMBEDDED_SHADER(sprite_vertex),
		BGFX_EMBEDDED_SHADER(sprite_fragment),
		BGFX_EMBEDDED_SHADER(basic_vertex),
		BGFX_EMBEDDED_SHADER(basic_fragment),
		BGFX_EMBEDDED_SHADER_END()
	};

	static const std::unordered_map<StandardMaterialType, std::string> _embeddedShaderNames
	{
		{StandardMaterialType::Basic, "basic"},
		{StandardMaterialType::Sprite, "sprite"},
		{StandardMaterialType::Debug, "debug"},
	};

	std::shared_ptr<Program> getStandardProgram(StandardMaterialType type) noexcept
	{
		auto itr = _embeddedShaderNames.find(type);
		if (itr == _embeddedShaderNames.end())
		{
			return nullptr;
		}
		auto renderer = bgfx::getRendererType();
		auto handle = bgfx::createProgram(
			bgfx::createEmbeddedShader(_embeddedShaders, renderer, (itr->second + "_vertex").c_str()),
			bgfx::createEmbeddedShader(_embeddedShaders, renderer, (itr->second + "_fragment").c_str()),
			true
		);
		return std::make_shared<Program>(handle);
	}

	static const std::unordered_map<StandardMaterialType, ProgramDefinition> _standardProgramDefinitions
	{
		{StandardMaterialType::Basic, {
			{
				{bgfx::Attrib::Position, { bgfx::AttribType::Float, 3}},
				{bgfx::Attrib::Normal, { bgfx::AttribType::Float, 3}},
				{bgfx::Attrib::Color0, { bgfx::AttribType::Uint8, 4, true}},
				{bgfx::Attrib::TexCoord0, { bgfx::AttribType::Float, 2}},
			},
			{
				{ProgramUniform::DiffuseTexture, {"s_texColor", bgfx::UniformType::Sampler}},
				{ProgramUniform::DiffuseColor, {"u_diffuseColor", bgfx::UniformType::Vec4, Data::copy(glm::vec4(1, 1, 1, 1))}},
			}
		}},
		{StandardMaterialType::Sprite, {
			{
				{bgfx::Attrib::Position, { bgfx::AttribType::Float, 2}},
				{bgfx::Attrib::Color0, { bgfx::AttribType::Uint8, 4, true}},
				{bgfx::Attrib::TexCoord0, { bgfx::AttribType::Float, 2}},
			},
			{
				{ProgramUniform::DiffuseTexture, {"s_texColor", bgfx::UniformType::Sampler}},
			}
		}},
		{StandardMaterialType::Debug, {
			{
				{bgfx::Attrib::Position, { bgfx::AttribType::Float, 2}},
			},
			{
				{ProgramUniform::DiffuseColor, {"u_color", bgfx::UniformType::Vec4}},
			}
		}},
	};

	ProgramDefinition getStandardProgramDefinition(StandardMaterialType type) noexcept
	{
		auto itr = _standardProgramDefinitions.find(type);
		if (itr == _standardProgramDefinitions.end())
		{
			return {};
		}
		return itr->second;
	}

	std::shared_ptr<Material> Material::createStandard(StandardMaterialType type) noexcept
	{
		return std::make_shared<Material>(getStandardProgram(type), getStandardProgramDefinition(type));
	}
}
