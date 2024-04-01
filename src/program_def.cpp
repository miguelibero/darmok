#include "program_def.hpp"
#include <darmok/light.hpp>
#include <charconv>
#include <cctype>
#include <glm/gtc/type_ptr.hpp>
#include <rapidjson/document.h>
#include "generated/shaders/unlit_progdef.h"
#include "generated/shaders/forward_phong_progdef.h"

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
		: _name(std::move(name))
		, _stage(stage)
	{
	}

	ProgramSamplerDefinition::ProgramSamplerDefinition(std::string name, std::string defaultTextureName, uint8_t stage) noexcept
		: _name(std::move(name))
		, _defaultTextureName(std::move(defaultTextureName))
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

	ProgramUniformDefinition::ProgramUniformDefinition(std::string name, bgfx::UniformType::Enum type, uint16_t num, Data&& defaultValue) noexcept
		: _name(std::move(name))
		, _type(type)
		, _num(num)
		, _default(std::move(defaultValue))
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

	static const std::unordered_map<StandardProgramType, const char*> _standardProgramDefinitionJsons
	{
		{StandardProgramType::ForwardPhong, forward_phong_progdef},
		{StandardProgramType::Unlit, unlit_progdef},
	};

	ProgramDefinition ProgramDefinition::getStandard(StandardProgramType type) noexcept
	{
		auto itr = _standardProgramDefinitionJsons.find(type);
		if (itr == _standardProgramDefinitionJsons.end())
		{
			return {};
		}
		ProgramDefinition progDef;
		rapidjson::Document json;
		json.Parse(itr->second);
		JsonDataProgramDefinitionLoader::read(progDef, json);
		return progDef;
	}

	JsonDataProgramDefinitionLoader::JsonDataProgramDefinitionLoader(IDataLoader& dataLoader)
		: _dataLoader(dataLoader)
	{
	}

	namespace json
	{
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

		std::string strToLower(std::string_view sv)
		{
			std::string s(sv);
			std::transform(s.begin(), s.end(), s.begin(),
				[](unsigned char c) { return std::tolower(c); } // correct
			);
			return s;
		}

		static bgfx::Attrib::Enum getBgfxAttrib(const std::string_view name) noexcept
		{
			auto sname = strToLower(name);
			if (sname == "position" || sname == "pos")
			{
				return bgfx::Attrib::Position;
			}
			if (sname == "normal" || sname == "norm" || sname == "n")
			{
				return bgfx::Attrib::Normal;
			}
			if (sname == "tangent" || name == "tang" || sname == "t")
			{
				return bgfx::Attrib::Normal;
			}
			if (sname == "bitangent" || sname == "bitang" || sname == "b")
			{
				return bgfx::Attrib::Normal;
			}
			if (sname == "bitangent" || sname == "bitang" || sname == "b")
			{
				return bgfx::Attrib::Normal;
			}
			if (sname == "indices" || sname == "index" || sname == "i")
			{
				return bgfx::Attrib::Indices;
			}
			if (sname == "weight" || sname == "w")
			{
				return bgfx::Attrib::Weight;
			}
			auto count = getNameSuffixCounter(sname, "color");
			if (count != std::nullopt)
			{
				return (bgfx::Attrib::Enum)((int)bgfx::Attrib::Color0 + count.value());
			}
			count = getNameSuffixCounter(sname, "texcoord");
			if (count == std::nullopt)
			{
				count = getNameSuffixCounter(sname, "tex_coord");
			}
			if (count != std::nullopt)
			{
				return (bgfx::Attrib::Enum)((int)bgfx::Attrib::TexCoord0 + count.value());
			}

			return bgfx::Attrib::Count;
		}

		static bgfx::AttribType::Enum getBgfxAttribType(const std::string_view name) noexcept
		{
			auto sname = strToLower(name);
			if (sname == "u8" || sname == "uint8")
			{
				return bgfx::AttribType::Uint8;
			}
			if (sname == "u10" || sname == "uint10")
			{
				return bgfx::AttribType::Uint10;
			}
			if (sname == "i" || sname == "int" || sname == "int16")
			{
				return bgfx::AttribType::Int16;
			}
			if (sname == "h" || sname == "half" || sname == "float8")
			{
				return bgfx::AttribType::Half;
			}
			if (sname == "f" || sname == "float" || sname == "float16")
			{
				return bgfx::AttribType::Float;
			}
			return bgfx::AttribType::Count;
		}


		static bgfx::UniformType::Enum getBgfxUniformType(const std::string_view name) noexcept
		{
			auto sname = strToLower(name);
			if (sname == "s" || sname == "sampler")
			{
				return bgfx::UniformType::Sampler;
			}
			if (sname == "v" || sname == "vec4")
			{
				return bgfx::UniformType::Vec4;
			}
			if (sname == "m3" || sname == "mat3")
			{
				return bgfx::UniformType::Mat3;
			}
			if (sname == "m4" || sname == "mat4")
			{
				return bgfx::UniformType::Mat4;
			}
			return bgfx::UniformType::Count;
		}

		static std::optional<ProgramUniform> getProgramUniform(std::string_view name)
		{
			auto sname = strToLower(name);
			if (sname == "t" || sname == "time")
			{
				return ProgramUniform::Time;
			}
			if (sname == "color" || sname == "diffuse" || sname == "diffuse_color" || sname == "diffusecolor")
			{
				return ProgramUniform::DiffuseColor;
			}
			if (sname == "light_count" || sname == "lightcount")
			{
				return ProgramUniform::LightCount;
			}
			if (sname == "ambient" || sname == "ambient_light" || sname == "ambientlight")
			{
				return ProgramUniform::AmbientLightColor;
			}
			return std::nullopt;
		}

		static std::optional<ProgramSampler> getProgramSampler(std::string_view name)
		{
			auto sname = strToLower(name);
			if (sname == "diffuse" || sname == "diffusetexture" || sname == "diffuse_texture")
			{
				return ProgramSampler::DiffuseTexture;
			}
			return std::nullopt;
		}

		static std::optional<ProgramBuffer> getProgramBuffer(std::string_view name)
		{
			auto sname = strToLower(name);
			if (sname == "pointlights" || sname == "point_lights")
			{
				return ProgramBuffer::PointLights;
			}
			return std::nullopt;
		}

		static std::optional<ProgramAttribDefinition> getProgramAttribDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			auto type = bgfx::AttribType::Float;
			if (value.HasMember("type"))
			{
				type = getBgfxAttribType(value["type"].GetString());
			}
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

		template<glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
		static void readVec(glm::vec<L, T, Q>& vec, const rapidjson::Document::ConstArray& json) noexcept
		{
			int i = 0;
			for (auto& elm : json)
			{
				vec[i++] = elm.GetFloat();
				if (i > vec.length())
				{
					break;
				}
			}
		}

		template<glm::length_t L1, glm::length_t L2, typename T, glm::qualifier Q = glm::defaultp>
		static void readMat(glm::mat<L1, L2, T, Q>& mat, const rapidjson::Document::ConstArray& json) noexcept
		{
			int i = 0;
			for (auto& elm : json)
			{
				readVec(mat[i++], elm.GetArray());
				if (i > mat.length())
				{
					break;
				}
			}
		}

		static bgfx::UniformType::Enum getUniformData(Data& data, const rapidjson::Value& v)
		{
			auto jsonType = v.GetType();
			if (jsonType == rapidjson::Type::kStringType)
			{
				data = std::move(Data(v.GetString(), v.GetStringLength()));
				return bgfx::UniformType::Sampler;
			}
			if (jsonType == rapidjson::Type::kArrayType)
			{
				auto jsonArray = v.GetArray();
				auto size = jsonArray.Size();
				if (size > 0)
				{
					auto elmType = jsonArray[0].GetType();
					if (elmType == rapidjson::Type::kNumberType && size >= 4)
					{
						glm::vec4 v;
						readVec(v, jsonArray);
						data = std::move(Data::copy(v));
						return bgfx::UniformType::Vec4;
					}
					if (elmType == rapidjson::Type::kArrayType)
					{
						if (size == 3)
						{
							glm::mat3 v;
							readMat(v, jsonArray);
							data = std::move(Data::copy(v));
							return bgfx::UniformType::Mat3;
						}
						if (size >= 4)
						{
							glm::mat4 v;
							readMat(v, jsonArray);
							data = std::move(Data::copy(v));
							return bgfx::UniformType::Mat4;
						}
					}
				}
			}
			return bgfx::UniformType::Count;
		}

		static std::optional<ProgramUniformDefinition> getProgramUniformDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			auto type = bgfx::UniformType::Vec4;
			if (value.HasMember("type"))
			{
				type = getBgfxUniformType(value["type"].GetString());
			}
			if (type == bgfx::UniformType::Count)
			{
				return std::nullopt;
			}
			if (!value.HasMember("name"))
			{
				return std::nullopt;
			}
			std::string name = value["name"].GetString();
			uint8_t num = 1;
			if (value.HasMember("num"))
			{
				num = value["num"].GetInt();
			}
			Data defaultData;
			if (value.HasMember("default"))
			{
				getUniformData(defaultData, value["default"]);
			}
			return ProgramUniformDefinition(name, type, num, std::move(defaultData));
		}

		static std::optional<ProgramSamplerDefinition> getProgramSamplerDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			if (!value.HasMember("name"))
			{
				return std::nullopt;
			}
			std::string name = value["name"].GetString();
			uint8_t stage = 0;
			if (value.HasMember("stage"))
			{
				stage = value["stage"].GetInt();
			}
			std::string defaultTextureName;
			if (value.HasMember("default"))
			{
				defaultTextureName = value["default"].GetString();
			}
			return ProgramSamplerDefinition(name, defaultTextureName, stage);
		}

		size_t readAttribMap(ProgramAttribMap& attribs, const rapidjson::Document::ConstObject& json)
		{
			size_t count = 0;
			for (auto& member : json)
			{
				auto attrib = json::getBgfxAttrib(member.name.GetString());
				if (attrib == bgfx::Attrib::Count)
				{
					continue;
				}
				auto definition = json::getProgramAttribDefinition(member.value);
				if (!definition)
				{
					continue;
				}
				attribs.emplace(attrib, definition.value());
				count++;
			}
			return count;
		}

		static std::optional<ProgramBufferDefinition> getProgramBufferDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			ProgramBufferDefinition def{ 0 };
			if (value.HasMember("stage"))
			{
				def.stage = value["stage"].GetInt();
			}
			if (value.HasMember("attributes"))
			{
				readAttribMap(def.attribs, value["attributes"].GetObject());
			}
			return def;
		}
	}

	void JsonDataProgramDefinitionLoader::read(ProgramDefinition& progDef, const rapidjson::Document& json)
	{
		if (json.HasMember("attributes"))
		{
			json::readAttribMap(progDef.attribs, json["attributes"].GetObject());
		}
		if (json.HasMember("uniforms"))
		{
			for (auto& member : json["uniforms"].GetObject())
			{
				auto uniform = json::getProgramUniform(member.name.GetString());
				if (!uniform)
				{
					continue;
				}
				auto definition = json::getProgramUniformDefinition(member.value);
				if (!definition)
				{
					continue;
				}
				progDef.uniforms.emplace(uniform.value(), definition.value());
			}
		}
		if (json.HasMember("samplers"))
		{
			for (auto& member : json["samplers"].GetObject())
			{
				auto sampler = json::getProgramSampler(member.name.GetString());
				if (!sampler)
				{
					continue;
				}
				auto definition = json::getProgramSamplerDefinition(member.value);
				if (!definition)
				{
					continue;
				}
				progDef.samplers.emplace(sampler.value(), definition.value());
			}
		}
	}

	ProgramDefinition JsonDataProgramDefinitionLoader::operator()(std::string_view name)
	{
		ProgramDefinition progDef;
		auto data = _dataLoader(name);
		rapidjson::Document json;
		json.Parse((const char*)data->ptr(), data->size()); // maybe ParseInsitu?
		read(progDef, json);
		return progDef;
	}
}