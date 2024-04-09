#include "program_def.hpp"
#include <darmok/light.hpp>
#include <darmok/uniform.hpp>
#include <charconv>
#include <cctype>
#include <glm/gtc/type_ptr.hpp>
#include <rapidjson/document.h>
#include "generated/shaders/unlit_progdef.h"
#include "generated/shaders/forward_phong_progdef.h"

namespace darmok
{
	void ProgramAttribDefinition::updateVertexLayout(bgfx::Attrib::Enum attrib, bgfx::VertexLayout& layout) const noexcept
	{
		layout.add(attrib, num, type, normalize);
	}

	bool ProgramAttribDefinition::operator==(const ProgramAttribDefinition& other) const noexcept
	{
		return type == other.type && num == other.num && normalize == other.normalize;
	}

	bool ProgramAttribDefinition::operator!=(const ProgramAttribDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	static bgfx::AttribType::Enum getBiggestAttribType(bgfx::AttribType::Enum a, bgfx::AttribType::Enum b) noexcept
	{
		return (bgfx::AttribType::Enum)std::max((int)a, (int)b);
	}

	ProgramAttribDefinition& ProgramAttribDefinition::operator+=(const ProgramAttribDefinition& other) noexcept
	{
		type = getBiggestAttribType(type, other.type);
		num = std::max(num, other.num);
		normalize = normalize || other.normalize;
		return *this;
	}

	ProgramAttribDefinition ProgramAttribDefinition::operator+(const ProgramAttribDefinition& other) const  noexcept
	{
		return {
			getBiggestAttribType(type, other.type),
			std::max(num, other.num),
			normalize || other.normalize,
		};
	}

	ProgramSamplerDefinition::ProgramSamplerDefinition(std::string name, uint8_t stage) noexcept
		: name(std::move(name))
		, stage(stage)
	{
	}

	ProgramSamplerDefinition::ProgramSamplerDefinition(std::string name, std::string defaultTexture, uint8_t stage) noexcept
		: name(std::move(name))
		, defaultTexture(std::move(defaultTexture))
		, stage(stage)
	{
	}

	bool ProgramSamplerDefinition::operator==(const ProgramSamplerDefinition& other) const noexcept
	{
		return name == other.name && defaultTexture == other.defaultTexture && stage == other.stage;
	}

	bool ProgramSamplerDefinition::operator!=(const ProgramSamplerDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	bgfx::UniformHandle ProgramSamplerDefinition::createHandle() const noexcept
	{
		return bgfx::createUniform(name.c_str(), bgfx::UniformType::Sampler);
	}

	ProgramPackDefinition::ProgramPackDefinition(ProgramPackType type, uint16_t num) noexcept
		: type(type)
		, num(num)
	{

	}

	ProgramPackDefinition::ProgramPackDefinition(ProgramPackType type, uint16_t num, Data&& defaultValue) noexcept
		: type(type)
		, num(num)
		, defaultValue(std::move(defaultValue))
	{
	}

	bool ProgramPackDefinition::operator==(const ProgramPackDefinition& other) const noexcept
	{
		return type == other.type && num == other.num && defaultValue == other.defaultValue;
	}

	bool ProgramPackDefinition::operator!=(const ProgramPackDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	bool ProgramUniformDefinition::operator==(const ProgramUniformDefinition& other) const noexcept
	{
		return type == other.type && num == other.num && packs == other.packs;
	}

	bool ProgramUniformDefinition::operator!=(const ProgramUniformDefinition& other) const noexcept
	{
		return !operator==(other);
	}

	bool ProgramUniformDefinition::contains(const ProgramUniformDefinition& other) const noexcept
	{
		for (auto& elm : other.packs)
		{
			auto itr = packs.find(elm.first);
			if(itr == packs.end())
			{
				return false;
			}
		}
		return true;
	}

	bgfx::UniformHandle ProgramUniformDefinition::createHandle() const noexcept
	{
		return bgfx::createUniform(name.c_str(), type, num);
	}

	static bgfx::VertexLayout createVertexLayout(const ProgramAttribMap& attribs) noexcept
	{
		bgfx::VertexLayout layout;
		layout.begin();
		for (const auto& pair : attribs)
		{
			pair.second.updateVertexLayout(pair.first, layout);
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
		return stage == other.stage && type == other.type && attribs == other.attribs;
	}

	bool ProgramBufferDefinition::operator!=(const ProgramBufferDefinition& other) const noexcept
	{
		return !operator==(other);
	}


	static ProgramBufferAttribType getBiggestProgramBufferAttribType(ProgramBufferAttribType a, ProgramBufferAttribType b) noexcept
	{
		return (ProgramBufferAttribType)std::max((int)a, (int)b);
	}

	ProgramBufferDefinition& ProgramBufferDefinition::operator+=(const ProgramBufferDefinition& other) noexcept
	{
		type = getBiggestProgramBufferAttribType(type, other.type);
		for (auto& attrib : other.attribs)
		{
			if (!hasAttrib(attrib))
			{
				attribs.push_back(attrib);
			}
		}
		return *this;
	}

	ProgramBufferDefinition ProgramBufferDefinition::operator+(const ProgramBufferDefinition& other) const  noexcept
	{
		std::vector<bgfx::Attrib::Enum> combinedAttribs(attribs);
		for (auto& attrib : other.attribs)
		{
			auto itr = std::find(combinedAttribs.begin(), combinedAttribs.end(), attrib);
			if (itr == combinedAttribs.end())
			{
				combinedAttribs.push_back(attrib);
			}
		}
		return { name, getBiggestProgramBufferAttribType(type, other.type), stage, combinedAttribs };
	}

	bool ProgramBufferDefinition::contains(const ProgramBufferDefinition& other) const noexcept
	{
		for (auto& elm : other.packs)
		{
			auto itr = packs.find(elm.first);
			if (itr == packs.end())
			{
				return false;
			}
		}
		return true;
	}

	bool ProgramBufferDefinition::hasAttrib(bgfx::Attrib::Enum attrib) const noexcept
	{
		auto itr = std::find(attribs.begin(), attribs.end(), attrib);
		return itr != attribs.end();
	}

	bgfx::DynamicVertexBufferHandle ProgramBufferDefinition::createHandle() const noexcept
	{
		return bgfx::createDynamicVertexBuffer(1, createVertexLayout(), BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_ALLOW_RESIZE);
	}

	bgfx::VertexLayout ProgramBufferDefinition::createVertexLayout() const noexcept
	{
		bgfx::VertexLayout layout;
		layout.begin();

		ProgramAttribDefinition attribDef{ bgfx::AttribType::Float, 1, false };
		switch (type)
		{
		case ProgramBufferAttribType::Vec4:
			attribDef.num = 4;
			break;
		case ProgramBufferAttribType::Uvec4:
			attribDef.num = 4;
			attribDef.type = bgfx::AttribType::Uint8;
			break;
		case ProgramBufferAttribType::Uint:
			attribDef.type = bgfx::AttribType::Uint8;
			break;
		case ProgramBufferAttribType::Int:
			attribDef.type = bgfx::AttribType::Int16;
			break;
		case ProgramBufferAttribType::Bool:
			attribDef.type = bgfx::AttribType::Uint8;
			break;
		}

		for (auto& attrib : attribs)
		{
			attribDef.updateVertexLayout(attrib, layout);
		}
		layout.end();
		return layout;
	}

	bool ProgramDefinition::empty() const noexcept
	{
		return attribs.empty() && uniforms.empty() && samplers.empty() && buffers.empty();
	}

	bgfx::VertexLayout ProgramDefinition::createVertexLayout() const noexcept
	{
		return darmok::createVertexLayout(attribs);
	}

	bool ProgramDefinition::operator==(const ProgramDefinition& other) const noexcept
	{
		return attribs == other.attribs && uniforms == other.uniforms && samplers == other.samplers && buffers == other.buffers;
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
			auto uniform = getUniform(elm.first);
			if (!uniform || uniform.value() != elm.second)
			{
				return false;
			}
		}
		for (auto& elm : other.samplers)
		{
			auto sampler = getSampler(elm.first);
			if (!sampler || sampler.value() != elm.second)
			{
				return false;
			}
		}
		for (auto& elm : other.buffers)
		{
			auto buffer = getBuffer(elm.first);
			if (!buffer || buffer.value() != elm.second)
			{
				return false;
			}
		}
		return true;
	}

	bool ProgramDefinition::hasAttrib(bgfx::Attrib::Enum attrib) const noexcept
	{
		auto itr = attribs.find(attrib);
		if (itr == attribs.end())
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

	bool ProgramDefinition::hasBuffer(std::string_view name, const ProgramDefinition& other) const noexcept
	{
		auto buffer = getBuffer(name);
		if (!buffer)
		{
			return false;
		}
		auto otherBuffer = other.getBuffer(name);
		return otherBuffer && buffer.value() == otherBuffer.value();
	}

	OptionalRef<const ProgramBufferDefinition> ProgramDefinition::getBuffer(std::string_view name) const noexcept
	{
		auto itr = std::find_if(buffers.begin(), buffers.end(),
			[&name](auto& elm) {
				return elm.first == name;
			});
		if (itr != buffers.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const ProgramBufferDefinition> ProgramDefinition::getBuffer(std::string_view name, ProgramBufferAttribType type) const noexcept
	{
		auto itr = std::find_if(buffers.begin(), buffers.end(),
			[&name, &type](auto& elm) {
				return elm.first == name && elm.second.type == type;
			});
		if (itr != buffers.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	bool ProgramDefinition::hasUniform(std::string_view name, const ProgramDefinition& other) const noexcept
	{
		auto uniform = getUniform(name);
		if (!uniform)
		{
			return false;
		}
		auto otherUniform = other.getUniform(name);
		return otherUniform && uniform.value() == otherUniform.value();
	}


	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(std::string_view name) const noexcept
	{
		auto itr = std::find_if(uniforms.begin(), uniforms.end(),
			[&name](auto& elm) {
				return elm.first == name;
			});
		if (itr != uniforms.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(std::string_view name, bgfx::UniformType::Enum type) const noexcept
	{
		auto itr = std::find_if(uniforms.begin(), uniforms.end(),
			[&name, &type](auto& elm) {
				return elm.first == name && elm.second.type == type;
			});
		if (itr != uniforms.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const ProgramUniformDefinition> ProgramDefinition::getUniform(std::string_view name, bgfx::UniformType::Enum type, uint16_t num) const noexcept
	{
		auto itr = std::find_if(uniforms.begin(), uniforms.end(),
			[&name, &type, &num](auto& elm) {
				return elm.first == name && elm.second.type == type && elm.second.num == num;
			});
		if (itr != uniforms.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	OptionalRef<const ProgramSamplerDefinition> ProgramDefinition::getSampler(std::string_view name) const noexcept
	{
		auto itr = std::find_if(samplers.begin(), samplers.end(),
			[name](auto& v){
				return v.first == name;
		});
		if (itr != samplers.end())
		{
			return itr->second;
		}
		return nullptr;
	}

	static const std::unordered_map<StandardProgramType, const char*> _standardProgramDefinitionJsons
	{
		{StandardProgramType::ForwardPhong, forward_phong_progdef},
		{StandardProgramType::Unlit, unlit_progdef},
	};

	static std::optional<ProgramDefinition> getStandardProgramDefinitionInclude(std::string_view name) noexcept
	{
		if (name == "phong_lighting")
		{
			return PhongLightRenderer::getProgramDefinition();
		}
		return std::nullopt;
	}

	ProgramDefinition ProgramDefinition::createFromJson(std::string_view json)
	{
		rapidjson::Document doc;
		doc.Parse(json.data(), json.size());
		ProgramDefinition progDef;
		JsonDataProgramDefinitionLoader::read(progDef, doc, getStandardProgramDefinitionInclude);
		return progDef;
	}

	ProgramDefinition ProgramDefinition::createStandard(StandardProgramType type)
	{
		auto itr = _standardProgramDefinitionJsons.find(type);
		if (itr == _standardProgramDefinitionJsons.end())
		{
			return {};
		}
		return createFromJson(itr->second);
	}

	JsonDataProgramDefinitionLoader::JsonDataProgramDefinitionLoader(IDataLoader& dataLoader)
		: _dataLoader(dataLoader)
	{
	}

	namespace json
	{
		static std::string_view getStringView(const rapidjson::Value& v) noexcept
		{
			return std::string_view(v.GetString(), v.GetStringLength());
		}

		static std::string getString(const rapidjson::Value& v) noexcept
		{
			return std::string(v.GetString(), v.GetStringLength());
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

		static std::optional<ProgramBufferAttribType> getProgramBufferAttribType(std::string_view name) noexcept
		{
			auto sname = strToLower(name);
			if (sname == "vec4" || sname == "v4")
			{
				return ProgramBufferAttribType::Vec4;
			}
			if (sname == "uvec4" || sname == "u4")
			{
				return ProgramBufferAttribType::Uvec4;
			}
			if (sname == "uint" || sname == "u")
			{
				return ProgramBufferAttribType::Uint;
			}
			if (sname == "int" || sname == "i")
			{
				return ProgramBufferAttribType::Int;
			}
			if (sname == "bool" || sname == "boolean" || sname == "b")
			{
				return ProgramBufferAttribType::Bool;
			}
			return std::nullopt;
		}

		static std::optional<ProgramAttribDefinition> getProgramAttribDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			ProgramAttribDefinition def{ bgfx::AttribType::Float, 1, false };
			if (value.HasMember("type"))
			{
				def.type = getBgfxAttribType(getStringView(value["type"]));
			}
			if (def.type == bgfx::AttribType::Count)
			{
				return std::nullopt;
			}
			if (value.HasMember("num"))
			{
				def.num = value["num"].GetInt();
			}
			if (value.HasMember("normalize"))
			{
				def.normalize = value["normalize"].GetBool();
			}
			return def;
		}

		static ProgramPackType getProgramPackType(const std::string_view name) noexcept
		{
			auto sname = strToLower(name);
			if (sname == "f" || sname == "float")
			{
				return ProgramPackType::Float;
			}
			if (sname == "i" || sname == "int")
			{
				return ProgramPackType::Int;
			}
			if (sname == "u" || sname == "uint")
			{
				return ProgramPackType::Uint8;
			}
			if (sname == "b" || sname == "bool")
			{
				return ProgramPackType::Bool;
			}
			return ProgramPackType::Float;
		}


		static void readVec(std::vector<float>& vec, const rapidjson::Document::ConstArray& json) noexcept
		{
			vec.reserve(json.Size());
			for (auto& elm : json)
			{
				vec.push_back(elm.GetFloat());
			}
		}

		static void readMat(std::vector<std::vector<float>>& mat, const rapidjson::Document::ConstArray& json) noexcept
		{
			mat.reserve(json.Size());
			for (auto& elm : json)
			{
				auto itr = mat.emplace({});
				readVec(*itr, elm.GetArray());
			}
		}

		static void getDefaultData(ProgramPackType type, const rapidjson::Value& v, Data& data) noexcept
		{
			auto jsonType = v.GetType();
			if (jsonType == rapidjson::Type::kStringType)
			{
				data = std::move(Data(v.GetString(), v.GetStringLength()));
				return;
			}
			if (jsonType == rapidjson::Type::kNumberType)
			{
				switch (type)
				{
				case ProgramPackType::Float:
					data = Data::copy(v.GetFloat());
					break;
				case ProgramPackType::Int:
					data = Data::copy(v.GetInt());
					break;
				case ProgramPackType::Uint8:
					data = Data::copy(v.GetUint());
					break;
				case ProgramPackType::Bool:
					data = Data::copy(v.GetBool());
					break;
				}
			}
			if (jsonType == rapidjson::Type::kArrayType)
			{
				auto jsonArray = v.GetArray();
				auto size = jsonArray.Size();
				if (size == 0)
				{
					return;
				}
				auto elmType = jsonArray[0].GetType();
				if (elmType == rapidjson::Type::kNumberType)
				{
					std::vector<float> v;
					readVec(v, jsonArray);
					data = std::move(Data::copy(v));
				}
				else if (elmType == rapidjson::Type::kArrayType)
				{
					std::vector<std::vector<float>> v;
					readMat(v, jsonArray);
					data = std::move(Data::copy(v));
				}
			}
		}

		static std::optional<ProgramPackDefinition> getProgramPackDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			ProgramPackDefinition def{ ProgramPackType::Float, 1 };
			if (!value.HasMember("type"))
			{
				def.type = getProgramPackType(getStringView(value["type"]));
			}
			if (value.HasMember("num"))
			{
				def.num = value["num"].GetInt();
			}
			if (value.HasMember("default"))
			{
				getDefaultData(def.type, value["default"], def.defaultValue);
			}
			return nullptr;
		}

		static std::optional<ProgramUniformDefinition> getProgramUniformDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			if (!value.HasMember("name"))
			{
				return std::nullopt;
			}
			ProgramUniformDefinition def{ getString(value["name"]), bgfx::UniformType::Vec4, 1 };
			if (value.HasMember("type"))
			{
				def.type = getBgfxUniformType(getStringView(value["type"]));
			}
			if (def.type == bgfx::UniformType::Count)
			{
				return std::nullopt;
			}
			if (value.HasMember("num"))
			{
				def.num = value["num"].GetInt();
			}
			if (value.HasMember("pack"))
			{
				for (auto& member : value["pack"].GetObject())
				{
					auto pack = getProgramPackDefinition(member.value);
					if (!pack)
					{
						continue;
					}
					def.packs.emplace(json::getString(member.name), pack.value());
				}
			}
			return def;
		}

		static std::optional<ProgramSamplerDefinition> getProgramSamplerDefinition(const rapidjson::Document::GenericValue& value) noexcept
		{
			if (!value.HasMember("name"))
			{
				return std::nullopt;
			}
			ProgramSamplerDefinition def{ std::string(getStringView(value["name"])), 0 };
			if (value.HasMember("stage"))
			{
				def.stage = value["stage"].GetInt();
			}
			if (value.HasMember("default"))
			{
				def.defaultTexture = value["default"].GetString();
			}
			return def;
		}

		size_t readAttribMap(ProgramAttribMap& attribs, const rapidjson::Document::ConstObject& json)
		{
			size_t count = 0;
			for (auto& member : json)
			{
				auto attrib = getBgfxAttrib(getStringView(member.name));
				if (attrib == bgfx::Attrib::Count)
				{
					continue;
				}
				auto definition = getProgramAttribDefinition(member.value);
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
			if (!value.HasMember("name"))
			{
				return std::nullopt;
			}
			ProgramBufferDefinition def{ std::string(getStringView(value["stage"])), ProgramBufferAttribType::Vec4, 0};
			if (value.HasMember("stage"))
			{
				def.stage = value["stage"].GetInt();
			}
			if (value.HasMember("type"))
			{
				auto type = getProgramBufferAttribType(getStringView(value["type"]));
				if (type)
				{
					def.type = type.value();
				}
			}
			if (value.HasMember("attributes"))
			{
				for (auto& elm : value["attributes"].GetArray())
				{
					def.attribs.push_back(getBgfxAttrib(getStringView(elm)));
				}
			}
			if (value.HasMember("pack"))
			{
				for (auto& member : value["pack"].GetObject())
				{
					auto pack = getProgramPackDefinition(member.value);
					if (!pack)
					{
						continue;
					}
					def.packs.emplace(json::getString(member.name), pack.value());
				}
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
				auto definition = json::getProgramUniformDefinition(member.value);
				if (!definition)
				{
					continue;
				}
				progDef.uniforms.emplace(json::getString(member.name), definition.value());
			}
		}
		if (json.HasMember("samplers"))
		{
			for (auto& member : json["samplers"].GetObject())
			{
				auto definition = json::getProgramSamplerDefinition(member.value);
				if (!definition)
				{
					continue;
				}
				progDef.samplers.emplace(json::getString(member.name), definition.value());
			}
		}
		if (json.HasMember("buffers"))
		{
			for (auto& member : json["buffers"].GetObject())
			{
				auto definition = json::getProgramBufferDefinition(member.value);
				if (!definition)
				{
					continue;
				}
				progDef.buffers.emplace(json::getString(member.name), definition.value());
			}
		}
	}

	ProgramDefinition JsonDataProgramDefinitionLoader::operator()(std::string_view name)
	{
		ProgramDefinition progDef;
		auto data = _dataLoader(name);
		rapidjson::Document json;
		json.Parse(static_cast<const char*>(data->ptr()), data->size()); // maybe ParseInsitu?
		read(progDef, json, [this](auto include) -> std::optional<ProgramDefinition>
		{
			auto v = getStandardProgramDefinitionInclude(include);
			if (v)
			{
				return v;
			}
			return (*this)(include);
		});
		return progDef;
	}
}