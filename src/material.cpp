#include <darmok/material.hpp>
#include <darmok/light.hpp>
#include <darmok/asset.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace darmok
{

	struct MaterialSamplerDefinition
	{
		std::string name;
		uint8_t stage;
	};

	struct MaterialColorUniformDefinition
	{
		std::string name;
		Color defaultValue;
	};

	static const std::unordered_map<MaterialTextureType, MaterialSamplerDefinition> _materialSamplerDefinitions = {
		{ MaterialTextureType::Diffuse, { "s_texColor", 0 } }
	};

	static const std::unordered_map<MaterialColorType, MaterialColorUniformDefinition> _materialColorDefinitions = {
		{ MaterialColorType::Diffuse, { "u_diffuseColor", Colors::white } }
	};


	Material::Material(const std::shared_ptr<Texture>& diffuseTexture) noexcept
		: _primitive(MaterialPrimitiveType::Triangle)
		, _mainData(32, 0, 0, 0)
		, _mainHandle{ bgfx::kInvalidHandle }
	{
		for (auto& pair : _materialSamplerDefinitions)
		{
			_textureHandles.emplace(pair.first, bgfx::createUniform(pair.second.name.c_str(), bgfx::UniformType::Sampler));
		}
		for (auto& pair : _materialColorDefinitions)
		{
			_colorHandles.emplace(pair.first, bgfx::createUniform(pair.second.name.c_str(), bgfx::UniformType::Vec4));
		}
		_mainHandle = bgfx::createUniform("u_material", bgfx::UniformType::Vec4);
		if (diffuseTexture != nullptr)
		{
			setTexture(MaterialTextureType::Diffuse, diffuseTexture);
		}
	}

	Material::~Material()
	{
		destroyHandles();
	}

	void Material::destroyHandles() noexcept
	{
		for (auto& pair : _textureHandles)
		{
			bgfx::destroy(pair.second);
		}
		_textureHandles.clear();
		for (auto& pair : _colorHandles)
		{
			bgfx::destroy(pair.second);
		}
		_colorHandles.clear();

		if (isValid(_mainHandle))
		{
			bgfx::destroy(_mainHandle);
			_mainHandle.idx = bgfx::kInvalidHandle;
		}
	}

	std::shared_ptr<Texture> Material::getTexture(MaterialTextureType type) const noexcept
	{
		auto itr = _textures.find(type);
		if (itr == _textures.end())
		{
			return nullptr;
		}
		return itr->second;
	}

	Material& Material::setTexture(MaterialTextureType type, const std::shared_ptr<Texture>& texture) noexcept
	{
		_textures[type] = texture;
		return *this;
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

	Material& Material::setColor(MaterialColorType type, const Color& color) noexcept
	{
		_colors[type] = color;
		return *this;
	}

	MaterialPrimitiveType Material::getPrimitiveType() const noexcept
	{
		return _primitive;
	}

	Material& Material::setPrimitiveType(MaterialPrimitiveType type) noexcept
	{
		_primitive = type;
		return *this;
	}

	uint8_t Material::getShininess() const noexcept
	{
		return _mainData.x;
	}

	Material& Material::setShininess(uint8_t v) noexcept
	{
		_mainData.x = v;
		return *this;
	}

	void Material::bgfxConfig(bgfx::Encoder& encoder) const noexcept
	{
		for (auto& pair : _textureHandles)
		{
			uint8_t stage = 0;
			{
				auto itr = _materialSamplerDefinitions.find(pair.first);
				if (itr != _materialSamplerDefinitions.end())
				{
					stage = itr->second.stage;
				}
			}
			std::shared_ptr<Texture> tex;
			auto itr = _textures.find(pair.first);
			if (itr != _textures.end())
			{
				tex = itr->second;
			}
			if (tex != nullptr)
			{
				encoder.setTexture(stage, pair.second, tex->getHandle());
			}
		}
		for (auto& pair : _colorHandles)
		{
			auto itr = _colors.find(pair.first);
			Color c = Colors::magenta;
			if (itr != _colors.end())
			{
				c = itr->second;
			}
			else
			{
				auto itr = _materialColorDefinitions.find(pair.first);
				if (itr != _materialColorDefinitions.end())
				{
					c = itr->second.defaultValue;
				}
			}
			encoder.setUniform(pair.second, glm::value_ptr(Colors::normalize(c)));
		}

		if (isValid(_mainHandle))
		{
			encoder.setUniform(_mainHandle, glm::value_ptr(_mainData));
		}

	}
}
