#include <darmok/material.hpp>
#include <darmok/light.hpp>
#include <darmok/asset.hpp>
#include <darmok/texture.hpp>
#include <darmok/program.hpp>

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
		{ MaterialColorType::Diffuse, { "u_diffuseColor", Colors::white() }},
		{ MaterialColorType::Specular, { "u_specularColor", Colors::white() }}
	};

	Material::Material(const std::shared_ptr<Texture>& diffuseTexture) noexcept
		: Material(nullptr, diffuseTexture)
	{
	}

	Material::Material(const std::shared_ptr<Program>& program, const std::shared_ptr<Texture>& diffuseTexture) noexcept
		: _primitive(MaterialPrimitiveType::Triangle)
		, _mainData{
			32,  // shininess
			0.5, // specular strenth
			0, 0 // unused
		}
		, _mainHandle{ bgfx::kInvalidHandle }
		, _program(program)
	{
		if (diffuseTexture != nullptr)
		{
			setTexture(MaterialTextureType::Diffuse, diffuseTexture);
		}
		createHandles();
	}

	void Material::createHandles() noexcept
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
	}

	Material::Material(const Material& other) noexcept
		: _program(other._program)
		, _textures(other._textures)
		, _colors(other._colors)
		, _mainData(other._mainData)
		, _primitive(other._primitive)
	{
		createHandles();
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

	std::optional<Color> Material::getColor(MaterialColorType type) const noexcept
	{
		auto itr = _colors.find(type);
		if (itr == _colors.end())
		{
			return std::nullopt;
		}
		return itr->second;
	}

	Material& Material::setColor(MaterialColorType type, const std::optional<Color>& color) noexcept
	{
		if (color)
		{
			_colors[type] = color.value();
		}
		else
		{
			_colors.erase(type);
		}
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
		return uint8_t(_mainData.x);
	}

	Material& Material::setShininess(uint8_t v) noexcept
	{
		_mainData.x = v;
		return *this;
	}

	float Material::getSpecularStrength() const noexcept
	{
		return _mainData.y;
	}

	Material& Material::setSpecularStrength(float v) noexcept
	{
		_mainData.y = v;
		return *this;
	}

	void Material::renderSubmit(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
	{
		auto state = beforeRender(encoder, viewId);
		encoder.setState(state);
		encoder.submit(viewId, _program->getHandle(_programDefines));
	}

	uint64_t Material::beforeRender(bgfx::Encoder& encoder, bgfx::ViewId viewId) const noexcept
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
			Color c = Colors::magenta();
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

		uint64_t state = BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
			| BGFX_STATE_WRITE_Z
			| BGFX_STATE_DEPTH_TEST_LEQUAL
			| BGFX_STATE_CULL_CCW
			| BGFX_STATE_MSAA
			| BGFX_STATE_BLEND_ALPHA
			;

		if (_primitive == MaterialPrimitiveType::Line)
		{
			state |= BGFX_STATE_PT_LINES;
		}

		return state;
	}
}
