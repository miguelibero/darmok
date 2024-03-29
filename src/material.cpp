#include <darmok/material.hpp>
#include <darmok/light.hpp>
#include <darmok/program_def.hpp>
#include <darmok/asset.hpp>

#include <glm/gtc/type_ptr.hpp>

namespace darmok
{
	Material::Material(const ProgramDefinition& progDef) noexcept
		: _progDef(progDef)
		, _vertexLayout(progDef.createVertexLayout())
	{
		for (auto& pair : _progDef.samplers)
		{
			_samplerHandles.emplace(pair.first, pair.second.createHandle());
		}
		for (auto& pair : _progDef.uniforms)
		{
			_uniformHandles.emplace(pair.first, pair.second.createHandle());
		}
	}

	Material::~Material()
	{
		for (auto& pair : _samplerHandles)
		{
			bgfx::destroy(pair.second);
		}
		for (auto& pair : _uniformHandles)
		{
			bgfx::destroy(pair.second);
		}
	}

	void Material::load(AssetContext& assets)
	{
		_defaultTextures.clear();
		for (auto& pair : _progDef.samplers)
		{
			auto& texName = pair.second.getDefaultTextureName();
			if (!texName.empty())
			{
				auto tex = assets.getTextureLoader()(texName);
				_defaultTextures.emplace(pair.first, tex);
			}
		}
	}

	const ProgramDefinition& Material::getProgramDefinition() const noexcept
	{
		return _progDef;
	}

	const bgfx::VertexLayout& Material::getVertexLayout() const noexcept
	{
		return _vertexLayout;
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
		return _shininess;
	}

	Material& Material::setShininess(uint8_t v) noexcept
	{
		_shininess = v;
		return *this;
	}

	static const std::unordered_map<ProgramSampler, MaterialTextureType> _materialTextureSamplers
	{
		{ ProgramSampler::DiffuseTexture, MaterialTextureType::Diffuse },
	};

	static const std::unordered_map<ProgramUniform, MaterialColorType> _materialColorUniforms
	{
		{ ProgramUniform::DiffuseColor, MaterialColorType::Diffuse },
	};

	void Material::bgfxConfig(bgfx::Encoder& encoder) const noexcept
	{
		for (auto& pair : _progDef.samplers)
		{
			auto itr1 = _samplerHandles.find(pair.first);
			if (itr1 == _samplerHandles.end())
			{
				continue;
			}
			auto itr2 = _materialTextureSamplers.find(pair.first);
			if (itr2 != _materialTextureSamplers.end())
			{
				auto itr3 = _textures.find(itr2->second);
				if (itr3 != _textures.end())
				{
					encoder.setTexture(pair.second.getStage(), itr1->second, itr3->second->getHandle());
					continue;
				}
			}
			auto itr3 = _defaultTextures.find(pair.first);
			if (itr3 != _defaultTextures.end())
			{
				encoder.setTexture(pair.second.getStage(), itr1->second, itr3->second->getHandle());
			}
		}
		for (auto& pair : _progDef.uniforms)
		{
			auto itr1 = _uniformHandles.find(pair.first);
			if (itr1 == _uniformHandles.end())
			{
				continue;
			}
			auto itr2 = _materialColorUniforms.find(pair.first);
			if (itr2 != _materialColorUniforms.end())
			{
				auto itr3 = _colors.find(itr2->second);
				if (itr3 != _colors.end())
				{
					auto colorVec = Colors::normalize(itr3->second);
					encoder.setUniform(itr1->second, glm::value_ptr(colorVec));
					continue;
				}
			}
			auto& def = pair.second.getDefault();
			if (!def.empty())
			{
				encoder.setUniform(itr1->second, def.ptr());
			}
		}
	}
}
