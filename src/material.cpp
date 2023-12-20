#include <darmok/material.hpp>
#include <darmok/model.hpp>
#include <darmok/asset.hpp>
#include <filesystem>

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

	MaterialPropertyCollection MaterialPropertyCollection::fromModel(const ModelMaterialPropertyCollection& model, int textureIndex)
	{
		MaterialPropertyCollection collection;
		for (auto& prop : model)
		{
			if (textureIndex < 0 && prop.getTextureType() != ModelMaterialTextureType::None)
			{
				continue;
			}
			else if (prop.getTextureIndex() != textureIndex)
			{
				continue;
			}
			collection.set(std::string(prop.getKey()), Data::copy(prop.getData()));
		}
		return collection;
	}


	MaterialTexture::MaterialTexture(bgfx::TextureHandle handle, MaterialTextureType type, MaterialPropertyCollection&& props)
		: _handle(handle)
		, _type(type)
		, _properties(std::move(props))
	{
	}

	const bgfx::TextureHandle& MaterialTexture::getHandle() const
	{
		return _handle;
	}

	MaterialTextureType MaterialTexture::getType() const
	{
		return _type;
	}

	void MaterialTexture::setHandle(const bgfx::TextureHandle& v)
	{
		_handle = v;
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

	MaterialTexture MaterialTexture::fromModel(const ModelMaterialTexture& texture, const ModelMaterial& material, const std::string& basePath)
	{
		std::filesystem::path texPath(texture.getPath());
		if (!basePath.empty() && texPath.is_relative())
		{
			texPath = std::filesystem::path(basePath) / texPath;
		}
		auto path = texture.getPath();
		auto handle = AssetContext::get().loadTexture(texPath.string());
		auto type = MaterialTextureType::Unknown;
		switch (texture.getType())
		{
		case ModelMaterialTextureType::Diffuse:
			type = MaterialTextureType::Diffuse;
			break;
		case ModelMaterialTextureType::Specular:
			type = MaterialTextureType::Specular;
			break;
		case ModelMaterialTextureType::Normals:
			type = MaterialTextureType::Normal;
			break;
		default:
			throw std::runtime_error("unsupported model texture type");
		}
		auto props = MaterialPropertyCollection::fromModel(material.getProperties(), texture.getIndex());
		return MaterialTexture(handle, type, std::move(props));
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

	std::shared_ptr<Material> Material::fromModel(const ModelMaterial& material, const std::string& basePath)
	{
		std::vector<MaterialTexture> textures;
		for (auto& modelTex : material.getTextures(ModelMaterialTextureType::Diffuse))
		{
			textures.push_back(MaterialTexture::fromModel(modelTex, material, basePath));
		}
		auto props = MaterialPropertyCollection::fromModel(material.getProperties(), -1);
		return std::make_shared<Material>(std::move(textures), std::move(props));
	}
}
