#include <darmok/material.hpp>
#include <darmok/asset.hpp>

namespace darmok
{
	bool MaterialPropertyCollection::has(const std::string& name) const
	{
		auto itr = _properties.find(name);
		return itr != _properties.end();
	}

	const Data& MaterialPropertyCollection::get(const std::string& name) const
	{
		static Data empty;
		auto itr = _properties.find(name);
		if (itr == _properties.end())
		{
			return empty;
		}
		return itr->second;
	}

	void MaterialPropertyCollection::set(const std::string& name, Data&& data)
	{
		_properties[name] = std::move(data);
	}

	MaterialTexture::MaterialTexture(const std::shared_ptr<Texture>& texture, MaterialTextureType type, MaterialPropertyCollection&& props)
		: _texture(texture)
		, _type(type)
		, _properties(std::move(props))
	{
	}

	const std::shared_ptr<Texture>& MaterialTexture::getTexture() const
	{
		return _texture;
	}

	MaterialTextureType MaterialTexture::getType() const
	{
		return _type;
	}

	void MaterialTexture::setTexture(const std::shared_ptr<Texture>& v)
	{
		_texture = v;
	}

	void MaterialTexture::setType(MaterialTextureType v)
	{
		_type = v;
	}

	const MaterialPropertyCollection& MaterialTexture::getProperties() const
	{
		return _properties;
	}

	MaterialPropertyCollection& MaterialTexture::getProperties()
	{
		return _properties;
	}

    Material::Material(std::vector<MaterialTexture>&& textures, MaterialPropertyCollection&& props)
		: _properties(std::move(props))
	{
		for (auto& tex : textures)
		{
			addTexture(std::move(tex));
		}
	}

	const std::vector<MaterialTexture>& Material::getTextures(MaterialTextureType type) const
	{
		static std::vector<MaterialTexture> empty;
		auto itr = _textures.find(type);
		if (itr == _textures.end())
		{
			return empty;
		}
		return itr->second;
	}

	const MaterialPropertyCollection& Material::getProperties() const
	{
		return _properties;
	}

	MaterialTexture& Material::addTexture(MaterialTexture&& texture)
	{
		auto& textures = _textures[texture.getType()];
		textures.push_back(std::move(texture));
		return textures.back();
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

	void Material::configure(bgfx::Encoder& encoder, const MaterialUniforms& uniforms)
	{
		auto textureUnit = 0;
		for (auto& texture : getTextures(MaterialTextureType::Diffuse))
		{
			encoder.setTexture(textureUnit, uniforms.TextureColor, texture.getTexture()->getHandle());
			textureUnit++;
		}
		if (isValid(uniforms.DiffuseColor))
		{
			if (auto c = getColor(MaterialColorType::Diffuse))
			{
				encoder.setUniform(uniforms.DiffuseColor, &c);
			}
		}
	}
}
