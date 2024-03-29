#include "program_def.hpp"
#include <darmok/light.hpp>
#include <charconv>
#include <glm/gtc/type_ptr.hpp>
#include <rapidjson/document.h>

namespace darmok
{
	bool ProgramAttribDefinition::operator==(const ProgramAttribDefinition& other) const noexcept
	{
		return type == other.type && num == other.num && normalize == other.normalize;
	}

	bool ProgramAttribDefinition::operator!=(const ProgramAttribDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	ProgramSamplerDefinition::ProgramSamplerDefinition(std::string name, uint8_t stage) noexcept
		: _name(name)
		, _stage(stage)
	{
	}

	ProgramSamplerDefinition::ProgramSamplerDefinition(std::string name, std::string defaultTextureName, uint8_t stage) noexcept
		: _name(name)
		, _defaultTextureName(defaultTextureName)
		, _stage(stage)
	{
	}

	bool ProgramSamplerDefinition::operator==(const ProgramSamplerDefinition& other) const noexcept
	{
		return _name == other._name && _defaultTextureName == other._defaultTextureName && _stage == other._stage;
	}

	bool ProgramSamplerDefinition::operator!=(const ProgramSamplerDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	const std::string& ProgramSamplerDefinition::getName() const
	{
		return _name;
	}

	const std::string& ProgramSamplerDefinition::getDefaultTextureName() const
	{
		return _defaultTextureName;
	}
	uint8_t ProgramSamplerDefinition::getStage() const
	{
		return _stage;
	}

	bgfx::UniformHandle ProgramSamplerDefinition::createHandle() const noexcept
	{
		return bgfx::createUniform(_name.c_str(), bgfx::UniformType::Sampler);
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num) noexcept
		: _name(std::move(name))
		, _type(type)
		, _num(num)
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::vec4& defaultValue) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Vec4)
		, _num(1)
		, _default(Data::copy(glm::value_ptr(defaultValue), 4))
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::mat3& defaultValue) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat3)
		, _num(1)
		, _default(Data::copy(glm::value_ptr(defaultValue), 9))
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const glm::mat4& defaultValue) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat4)
		, _num(1)
		, _default(Data::copy(glm::value_ptr(defaultValue), 16))
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const std::vector<glm::vec4>& defaultValue) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Vec4)
		, _num(defaultValue.size())
		, _default(Data::copy(&defaultValue.front(), 4))
	{
	}

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const std::vector<glm::mat4>& defaultValue) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat3)
		, _num(defaultValue.size())
		, _default(Data::copy(&defaultValue.front(), 9))
	{
	}
	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, const std::vector<glm::mat3>& defaultValue) noexcept
		: _name(std::move(name))
		, _type(bgfx::UniformType::Mat4)
		, _num(defaultValue.size())
		, _default(Data::copy(&defaultValue.front(), 16))
	{
	}

	bool ProgramUniformDefinition::operator==(const ProgramUniformDefinition& other) const noexcept
	{
		return _name == other._name && _type == other._type && _num == other._num && _default == other._default;
	}

	bool ProgramUniformDefinition::operator!=(const ProgramUniformDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	const std::string& ProgramUniformDefinition::getName() const
	{
		return _name;
	}

	bgfx::UniformType::Enum ProgramUniformDefinition::getType() const
	{
		return _type;
	}

	const Data& ProgramUniformDefinition::getDefault() const
	{
		return _default;
	}

	uint16_t ProgramUniformDefinition::getNum() const
	{
		return _num;
	}

	bgfx::UniformHandle ProgramUniformDefinition::createHandle() const noexcept
	{
		return bgfx::createUniform(_name.c_str(), _type, _num);
	}

	static bgfx::VertexLayout createVertexLayout(const ProgramAttribMap& attribs) noexcept
	{
		bgfx::VertexLayout layout;
		layout.begin();
		for (const auto& pair : attribs)
		{
			layout.add(pair.first, pair.second.num, pair.second.type, pair.second.normalize);
		}
		layout.end();
		return layout;
	}

	template<typename K, typename V>
	static std::unordered_map<K, V> combineMaps(const std::unordered_map<K, V>& a, const std::unordered_map<K, V>& b) noexcept
	{
		auto combined = a;
		combined.insert(b.begin(), b.end());
		return combined;
	}

	bool ProgramBufferDefinition::operator==(const ProgramBufferDefinition& other) const noexcept
	{
		return stage == other.stage && attribs == other.attribs;
	}

	bool ProgramBufferDefinition::operator!=(const ProgramBufferDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	ProgramBufferDefinition& ProgramBufferDefinition::operator+=(const ProgramBufferDefinition& other) noexcept
	{
		attribs.insert(other.attribs.begin(), other.attribs.end());
		return *this;
	}

	ProgramBufferDefinition ProgramBufferDefinition::operator+(const ProgramBufferDefinition& other) const  noexcept
	{
		return { stage, combineMaps(attribs, other.attribs) };
	}

	bool ProgramBufferDefinition::contains(const ProgramBufferDefinition& other) const noexcept
	{
		for (auto& elm : other.attribs)
		{
			if (!hasAttrib(elm.first, elm.second))
			{
				return false;
			}
		}
		return true;
	}

	bool ProgramBufferDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribMap& defs) const noexcept
	{
		auto itr = defs.find(attrib);
		if (itr == defs.end())
		{
			return false;
		}
		return hasAttrib(attrib, itr->second);
	}

	bool ProgramBufferDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribDefinition& def) const noexcept
	{
		auto itr = attribs.find(attrib);
		if (itr == attribs.end())
		{
			return false;
		}
		return itr->second == def;
	}

	bgfx::VertexLayout ProgramBufferDefinition::createVertexLayout() const noexcept
	{
		return darmok::createVertexLayout(attribs);
	}

	bgfx::VertexLayout ProgramDefinition::createVertexLayout() const noexcept
	{
		return darmok::createVertexLayout(attribs);
	}

	bool ProgramDefinition::operator==(const ProgramDefinition& other) const noexcept
	{
		return attribs == other.attribs && uniforms == other.uniforms && buffers == other.buffers;
	}

	bool ProgramDefinition::operator!=(const ProgramDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	ProgramDefinition& ProgramDefinition::operator+=(const ProgramDefinition& other) noexcept
	{
		attribs.insert(other.attribs.begin(), other.attribs.end());
		uniforms.insert(other.uniforms.begin(), other.uniforms.end());
		samplers.insert(other.samplers.begin(), other.samplers.end());
		buffers.insert(other.buffers.begin(), other.buffers.end());
		return *this;
	}

	ProgramDefinition ProgramDefinition::operator+(const ProgramDefinition& other) const noexcept
	{
		return {
			combineMaps(attribs, other.attribs),
			combineMaps(uniforms, other.uniforms),
			combineMaps(samplers, other.samplers),
			combineMaps(buffers, other.buffers),
		};
	}

	bool ProgramDefinition::contains(const ProgramDefinition& other) const noexcept
	{
		for (auto& elm : other.attribs)
		{
			if (!hasAttrib(elm.first, elm.second))
			{
				return false;
			}
		}
		for (auto& elm : other.uniforms)
		{
			if (!hasUniform(elm.first, elm.second))
			{
				return false;
			}
		}
		for (auto& elm : other.buffers)
		{
			if (!hasBuffer(elm.first, elm.second))
			{
				return false;
			}
		}
		return true;
	}

	bool ProgramDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribMap& defs) const noexcept
	{
		auto itr = defs.find(attrib);
		if (itr == defs.end())
		{
			return false;
		}
		return hasAttrib(attrib, itr->second);
	}

	bool ProgramDefinition::hasAttrib(bgfx::Attrib::Enum attrib, const ProgramAttribDefinition& def) const noexcept
	{
		auto itr = attribs.find(attrib);
		if (itr == attribs.end())
		{
			return false;
		}
		return itr->second == def;
	}

	bool ProgramDefinition::hasBuffer(ProgramBuffer buffer, const ProgramBufferMap& defs) const noexcept
	{
		auto itr = defs.find(buffer);
		if (itr == defs.end())
		{
			return false;
		}
		return hasBuffer(buffer, itr->second);
	}

	bool ProgramDefinition::hasBuffer(ProgramBuffer buffer, const ProgramBufferDefinition& def) const noexcept
	{
		auto itr = buffers.find(buffer);
		if (itr != buffers.end())
		{
			return itr->second == def;
		}
		return false;
	}

	bool ProgramDefinition::hasUniform(ProgramUniform uniform, const ProgramUniformMap& defs) const noexcept
	{
		auto itr = defs.find(uniform);
		if (itr == defs.end())
		{
			return false;
		}
		return hasUniform(uniform, itr->second);
	}

	bool ProgramDefinition::hasUniform(ProgramUniform uniform, const ProgramUniformDefinition& def) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			return itr->second == def;
		}
		return false;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(ProgramUniform uniform) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(ProgramUniform uniform, std::string_view name) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			auto& u = itr->second;
			if (u.getName() == name)
			{
				return u;
			}
		}
		return nullptr;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(ProgramUniform uniform, std::string_view name, bgfx::UniformType::Enum type, uint16_t num) const noexcept
	{
		auto itr = uniforms.find(uniform);
		if (itr != uniforms.end())
		{
			auto& u = itr->second;
			if (u.getName() == name && u.getType() == type && u.getNum() == num)
			{
				return u;
			}
		}
		return nullptr;
	}


	bool ProgramDefinition::hasSampler(ProgramSampler sampler, const ProgramSamplerMap& defs) const noexcept
	{
		auto itr = defs.find(sampler);
		if (itr == defs.end())
		{
			return false;
		}
		return hasSampler(sampler, itr->second);
	}

	bool ProgramDefinition::hasSampler(ProgramSampler sampler, const ProgramSamplerDefinition& def) const noexcept
	{
		auto itr = samplers.find(sampler);
		if (itr != samplers.end())
		{
			return itr->second == def;
		}
		return false;
	}

	OptionalRef<const ProgramSamplerDefinition> ProgramDefinition::getSampler(ProgramSampler sampler) const noexcept
	{
		auto itr = samplers.find(sampler);
		if (itr != samplers.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const ProgramSamplerDefinition> ProgramDefinition::getSampler(ProgramSampler sampler, std::string_view name, uint8_t stage) const noexcept
	{
		auto itr = samplers.find(sampler);
		if (itr != samplers.end())
		{
			auto& u = itr->second;
			if (u.getName() == name && u.getStage() == stage)
			{
				return u;
			}
		}
		return nullptr;
	}

	static ProgramDefinition createForwardPhongProgramDefinition() noexcept
	{
		return ProgramDefinition{
			{
				{ bgfx::Attrib::Position, { bgfx::AttribType::Float, 3 } },
				{ bgfx::Attrib::Normal, { bgfx::AttribType::Float, 3} },
				{ bgfx::Attrib::Tangent, { bgfx::AttribType::Float, 3} },
				{ bgfx::Attrib::TexCoord0, { bgfx::AttribType::Float, 2} },
				{ bgfx::Attrib::Color0, { bgfx::AttribType::Uint8, 4, true} },
			},
			{
				{ProgramUniform::DiffuseColor, {"u_diffuseColor", glm::vec4(1)}},
			},
			{
				{ProgramSampler::DiffuseTexture, {"s_texColor"}},
			}
		} + LightRenderUpdater::getPhongProgramDefinition();
	}

	static const std::unordered_map<StandardProgramType, ProgramDefinition> _standardProgramDefinitions
	{
		{StandardProgramType::ForwardPhong, createForwardPhongProgramDefinition() },
		{StandardProgramType::Unlit, {
			{
				{bgfx::Attrib::Position, { bgfx::AttribType::Float, 3}},
				{bgfx::Attrib::Color0, { bgfx::AttribType::Uint8, 4, true}},
				{bgfx::Attrib::TexCoord0, { bgfx::AttribType::Float, 2}},
			},
			{
				{ProgramUniform::DiffuseColor, {"u_diffuseColor", glm::vec4(1)}},
			},
			{
				{ProgramSampler::DiffuseTexture, {"s_texColor"}},
			}
		}}
	};

	ProgramDefinition ProgramDefinition::getStandard(StandardProgramType type) noexcept
	{
		auto itr = _standardProgramDefinitions.find(type);
		if (itr == _standardProgramDefinitions.end())
		{
			return {};
		}
		return itr->second;
	}

	JsonDataProgramDefinitionLoader::JsonDataProgramDefinitionLoader(IDataLoader& dataLoader)
		: _dataLoader(dataLoader)
	{
	}

	static std::optional<int> getNameSuffixCounter(const std::string_view name, const std::string_view prefix) noexcept
	{
		if (!name.starts_with(prefix))
		{
			return std::nullopt;
		}
		int v;
		auto r = std::from_chars(name.data() + prefix.size(), name.data() + name.size(), v);
		if (r.ptr == nullptr)
		{
			return std::nullopt;
		}
		return v;
	}

	static bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept
	{
		if (name == "position" || name == "pos")
		{
			return bgfx::Attrib::Position;
		}
		if (name == "normal" || name == "norm" || name == "n")
		{
			return bgfx::Attrib::Normal;
		}
		if (name == "tangent" || name == "tang" || name == "t")
		{
			return bgfx::Attrib::Normal;
		}
		if (name == "bitangent" || name == "bitang" || name == "b")
		{
			return bgfx::Attrib::Normal;
		}
		if (name == "bitangent" || name == "bitang" || name == "b")
		{
			return bgfx::Attrib::Normal;
		}
		if (name == "indices" || name == "index" || name == "i")
		{
			return bgfx::Attrib::Indices;
		}
		if (name == "weight" || name == "w")
		{
			return bgfx::Attrib::Weight;
		}
		auto count = getNameSuffixCounter(name, "color");
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::Color0 + count.value());
		}
		count = getNameSuffixCounter(name, "texcoord");
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::TexCoord0 + count.value());
		}

		return bgfx::Attrib::Count;
	}

	static bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept
	{
		return bgfx::AttribType::Count;
	}

	static std::optional<ProgramAttribDefinition> getJsonProgramAttribDefinition(const rapidjson::Document::GenericValue& value) noexcept
	{
		auto type = getBgfxAttribType(value["type"].GetString());
		if (type == bgfx::AttribType::Count)
		{
			return std::nullopt;
		}
		uint8_t num = 1;
		if (value.HasMember("num"))
		{
			num = value["num"].GetInt();
		}
		bool normalize = false;
		if (value.HasMember("normalize"))
		{
			normalize = value["normalize"].GetBool();
		}
		return ProgramAttribDefinition{ type, num, normalize };
	}

	std::shared_ptr<ProgramDefinition> JsonDataProgramDefinitionLoader::operator()(std::string_view name)
	{
		auto data = _dataLoader(name);
		rapidjson::Document json;
		// maybe ParseInsitu?
		json.Parse((const char*)data->ptr(), data->size());

		auto def = std::make_shared<ProgramDefinition>();

		for (auto& member : json["attributes"].GetObject())
		{
			auto attrib = getBgfxAttrib(member.name.GetString());
			if (attrib == bgfx::Attrib::Count)
			{
				continue;
			}
			auto attrDef = getJsonProgramAttribDefinition(member.value);
			if (!attrDef)
			{
				continue;
			}
			def->attribs.emplace(attrib, attrDef.value());
		}

		return def;
	}
}