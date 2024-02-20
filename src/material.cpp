#include <darmok/material.hpp>

namespace darmok
{
	MaterialUniforms MaterialUniforms::getDefault()
	{
		static MaterialUniforms uniforms
		{
			bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler),
			bgfx::createUniform("a_color0", bgfx::UniformType::Vec4),
		};
		return uniforms;
	}

    Material::Material(const std::shared_ptr<Program>& program, const MaterialUniforms& uniforms)
		: _program(program)
		, _uniforms(uniforms)
	{
	}

	const std::shared_ptr<Program>& Material::getProgram() const
	{
		return _program;
	}

	void Material::setProgram(const std::shared_ptr<Program>& program)
	{
		_program = program;
	}

	const std::vector<std::shared_ptr<Texture>>& Material::getTextures(MaterialTextureType type) const
	{
		static std::vector<std::shared_ptr<Texture>> empty;
		auto itr = _textures.find(type);
		if (itr == _textures.end())
		{
			return empty;
		}
		return itr->second;
	}

	void Material::addTexture(const std::shared_ptr<Texture>& texture, MaterialTextureType type)
	{
		_textures[type].push_back(texture);
	}

	std::optional<Color> Material::getColor(MaterialColorType type)
	{
		auto itr = _colors.find(type);
		if (itr == _colors.end())
		{
			return std::nullopt;
		}
		return itr->second;
	}

	void Material::setColor(MaterialColorType type, const Color& color)
	{
		_colors[type] = color;
	}

	void Material::submit(bgfx::Encoder& encoder, bgfx::ViewId viewId)
	{
		if (_program == nullptr)
		{
			return;
		}
		auto textureUnit = 0;
		for (auto& texture : getTextures(MaterialTextureType::Diffuse))
		{
			encoder.setTexture(textureUnit, _uniforms.TextureColor, texture->getHandle());
			textureUnit++;
		}
		if (auto c = getColor(MaterialColorType::Diffuse))
		{
			encoder.setUniform(_uniforms.DiffuseColor, &c);
		}

		// TODO: configure state
		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
			;

		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle());
	}
}
