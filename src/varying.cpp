#include <darmok/varying.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/utils.hpp>
#include <sstream>
#include <fstream>
#include <map>

namespace darmok
{
	size_t AttribUtils::getDisabledGroups(const AttribDefines& defines, AttribGroups& disabledGroups) noexcept
	{
		size_t count = 0;
		return count;
	}

	[[nodiscard]] AttribGroup AttribUtils::getGroup(bgfx::Attrib::Enum attrib) noexcept
	{
		if (attrib == bgfx::Attrib::Position)
		{
			return AttribGroup::Position;
		}
		if (attrib == bgfx::Attrib::Normal)
		{
			return AttribGroup::Normal;
		}
		if (attrib == bgfx::Attrib::Normal)
		{
			return AttribGroup::Normal;
		}
		if (attrib == bgfx::Attrib::Tangent)
		{
			return AttribGroup::Tangent;
		}
		if (attrib == bgfx::Attrib::Bitangent)
		{
			return AttribGroup::Bitangent;
		}
		if (attrib == bgfx::Attrib::Indices || attrib == bgfx::Attrib::Weight)
		{
			return AttribGroup::Skinning;
		}
		if (attrib >= bgfx::Attrib::Color0 && attrib <= bgfx::Attrib::Color3)
		{
			return AttribGroup::Color;
		}
		if (attrib >= bgfx::Attrib::TexCoord0 && attrib <= bgfx::Attrib::TexCoord7)
		{
			return AttribGroup::Texture;
		}
		return AttribGroup::Count;
	}
	
    bgfx::Attrib::Enum AttribUtils::getBgfx(const std::string_view name) noexcept
	{
		auto sname = StringUtils::toLower(name);
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
			return bgfx::Attrib::Tangent;
		}
		if (sname == "bitangent" || sname == "bitang" || sname == "b")
		{
			return bgfx::Attrib::Bitangent;
		}
		if (sname == "indices" || sname == "index" || sname == "i")
		{
			return bgfx::Attrib::Indices;
		}
		if (sname == "joints" || sname == "joint" || sname == "j")
		{
			return bgfx::Attrib::Indices;
		}
		if (sname == "weight" || sname == "weights" || sname == "w")
		{
			return bgfx::Attrib::Weight;
		}
		auto count = StringUtils::getIntSuffix(sname, "color");
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::Color0 + count.value());
		}
		count = StringUtils::getIntSuffix(sname, "texcoord");
		if (count == std::nullopt)
		{
			count = StringUtils::getIntSuffix(sname, "tex_coord");
		}
		if (count != std::nullopt)
		{
			return (bgfx::Attrib::Enum)((int)bgfx::Attrib::TexCoord0 + count.value());
		}

		return bgfx::Attrib::Count;
	}

	bgfx::AttribType::Enum AttribUtils::getBgfxType(const std::string_view name) noexcept
	{
		auto sname = StringUtils::toLower(name);
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

	std::string AttribUtils::getBgfxName(bgfx::Attrib::Enum val) noexcept
	{
		switch (val)
		{
		case bgfx::Attrib::Position:
			return "position";
		case bgfx::Attrib::Normal:
			return "normal";
		case bgfx::Attrib::Tangent:
			return "tangent";
		case bgfx::Attrib::Bitangent:
			return "bitangent";
		case bgfx::Attrib::Indices:
			return "indices";
		case bgfx::Attrib::Weight:
			return "weight";
		}
		if (val >= bgfx::Attrib::Color0 && val <= bgfx::Attrib::Color3)
		{
			auto i = to_underlying(val) - to_underlying(bgfx::Attrib::Color0);
			return std::string("color") + std::to_string(i);
		}
		if (val >= bgfx::Attrib::TexCoord0 && val <= bgfx::Attrib::TexCoord7)
		{
			auto i = to_underlying(val) - to_underlying(bgfx::Attrib::TexCoord0);
			return std::string("texcoord") + std::to_string(i);
		}
		return "";
	}

	std::string AttribUtils::getBgfxTypeName(bgfx::AttribType::Enum val) noexcept
	{
		switch (val)
		{
			case bgfx::AttribType::Uint8:
				return "uint8";
			case bgfx::AttribType::Uint10:
				return "uint10";
			case bgfx::AttribType::Int16:
				return "int";
			case bgfx::AttribType::Half:
				return "half";
			case bgfx::AttribType::Float:
				return "float";
		}
		return "";
	}


	void VertexAttribute::read(const std::string& key)
	{
		attrib = AttribUtils::getBgfx(key);
		if (attrib == bgfx::Attrib::Count)
		{
			throw std::invalid_argument("invalid key: " + key);
		}
		num = 3;
		type = bgfx::AttribType::Float;

		auto group = AttribUtils::getGroup(attrib);
		if (group == AttribGroup::Color)
		{
			num = 4;
			type = bgfx::AttribType::Uint8;
			normalize = true;
		}
		else if (group == AttribGroup::Texture)
		{
			num = 2;
		}
		else if (group == AttribGroup::Skinning)
		{
			num = 4;
		}
	}

	void VertexAttribute::read(const std::string& key, const nlohmann::json& json)
	{
		attrib = AttribUtils::getBgfx(key);
		if (attrib == bgfx::Attrib::Count)
		{
			throw std::invalid_argument("invalid key: " + key);
		}
		if (json.contains("type"))
		{
			auto typeStr = json["type"].get<std::string_view>();
			type = AttribUtils::getBgfxType(typeStr);
			if (type == bgfx::AttribType::Count)
			{
				throw std::invalid_argument("invalid type: " + key);
			}
		}
		if (json.contains("num"))
		{
			num = json["num"].get<uint8_t>();
		}
		if (json.contains("normalize"))
		{
			normalize = json["normalize"].get<bool>();
		}
		if (json.contains("int"))
		{
			asInt = json["int"].get<bool>();
		}
	}

	std::string VertexAttribute::write(nlohmann::json& json) const noexcept
	{
		json["type"] = AttribUtils::getBgfxTypeName(type);
		if (asInt)
		{
			json["int"] = true;
		}
		if (normalize)
		{
			json["normalize"] = true;
		}
		json["num"] = num;
		return AttribUtils::getBgfxName(attrib);
	}

	void VertexAttribute::addTo(bgfx::VertexLayout& layout) const noexcept
	{
		layout.add(attrib, num, type, normalize, asInt);
	}

	bool VertexAttribute::inGroup(AttribGroup group) const noexcept
	{
		return AttribUtils::getGroup(attrib) == group;
	}

	bool VertexAttribute::inGroups(const AttribGroups& groups) const noexcept
	{
		for (auto& group : groups)
		{
			if (inGroup(group))
			{
				return true;
			}
		}
		return false;
	}

	bool FragmentAttribute::inGroup(AttribGroup group) const noexcept
	{
		return AttribUtils::getGroup(attrib) == group;
	}

	bool FragmentAttribute::inGroups(const AttribGroups& groups) const noexcept
	{
		for (auto& group : groups)
		{
			if (inGroup(group))
			{
				return true;
			}
		}
		return false;
	}

	void FragmentAttribute::read(const std::string& key)
	{
		attrib = AttribUtils::getBgfx(key);
		if (attrib == bgfx::Attrib::Count)
		{
			throw std::invalid_argument("invalid key: " + key);
		}
		auto group = AttribUtils::getGroup(attrib);
		if (group == AttribGroup::Color)
		{
			num = 4;
			defaultValue = { 1, 1, 1, 1 };
		}
		else if (group == AttribGroup::Texture)
		{
			num = 2;
			defaultValue = { 0.F, 0.F };
		}
		else if (attrib == bgfx::Attrib::Indices)
		{
			num = 4;
			defaultValue = { -1.F, -1.F, -1.F, -1.F };
		}
		else if (attrib == bgfx::Attrib::Weight)
		{
			num = 4;
			defaultValue = { 1.F, 0.F, 0.F, 0.F };
		}
		else
		{
			num = 3;
			defaultValue = { 0.F, 0.F, 0.F };
		}
	}

	void FragmentAttribute::read(const std::string& key, const nlohmann::json& json)
	{
		attrib = AttribUtils::getBgfx(key);
		if (attrib == bgfx::Attrib::Count)
		{
			throw std::invalid_argument("invalid key: " + key);
		}
		if (json.is_array())
		{
			defaultValue = json.get<std::vector<float>>();
			num = defaultValue.size();
			return;
		}
		if (json.contains("default"))
		{
			defaultValue = json["default"].get<std::vector<float>>();
		}
		if (json.contains("num"))
		{
			num = json["num"].get<uint8_t>();
		}
		else
		{
			num = defaultValue.size();
		}
	}

	std::string FragmentAttribute::write(nlohmann::json& json) const noexcept
	{
		json["num"] = num;
		if (!defaultValue.empty())
		{
			json["default"] = defaultValue;
		}
		return AttribUtils::getBgfxName(attrib);
	}

	const std::unordered_map<std::string, bgfx::Attrib::Enum>& VaryingDefinition::getAttrs() noexcept
	{
		static const std::unordered_map<std::string, bgfx::Attrib::Enum> map =
		{
			{ "POSITION",		bgfx::Attrib::Position },
			{ "NORMAL",			bgfx::Attrib::Normal },
			{ "TANGENT",		bgfx::Attrib::Tangent },
			{ "BITANGENT",		bgfx::Attrib::Bitangent },
			{ "COLOR0",			bgfx::Attrib::Color0 },
			{ "COLOR1",			bgfx::Attrib::Color1 },
			{ "COLOR2",			bgfx::Attrib::Color2 },
			{ "COLOR3",			bgfx::Attrib::Color3 },
			{ "BLENDINDICES",	bgfx::Attrib::Indices },
			{ "BLENDWEIGHT",	bgfx::Attrib::Weight },
			{ "TEXCOORD0",		bgfx::Attrib::TexCoord0 },
			{ "TEXCOORD1",		bgfx::Attrib::TexCoord1 },
			{ "TEXCOORD2",		bgfx::Attrib::TexCoord2 },
			{ "TEXCOORD3",		bgfx::Attrib::TexCoord3 },
			{ "TEXCOORD4",		bgfx::Attrib::TexCoord4 },
			{ "TEXCOORD5",		bgfx::Attrib::TexCoord5 },
			{ "TEXCOORD6",		bgfx::Attrib::TexCoord6 },
			{ "TEXCOORD7",		bgfx::Attrib::TexCoord7 },
		};
		return map;
	}

	const std::string VaryingDefinition::_vertexJsonKey = "vertex";
	const std::string VaryingDefinition::_fragmentJsonKey = "fragment";

	void VaryingDefinition::read(const nlohmann::ordered_json& json)
	{
		for (auto& elm : json.items())
		{
			if (elm.key().starts_with(_vertexJsonKey))
			{
				vertex.read(elm.value());
				continue;
			}
			if (elm.key().starts_with(_fragmentJsonKey))
			{
				readFragments(elm.value());
			}
		}
	}

	void VaryingDefinition::readFragments(const nlohmann::ordered_json& json)
	{
		if (json.is_object())
		{
			for (auto& elm : json.items())
			{
				fragment.emplace_back().read(elm.key(), elm.value());
			}
			return;
		}
		if (!json.is_array())
		{
			return;
		}
		for (auto& elm : json)
		{
			auto& frag = fragment.emplace_back();
			if (elm.is_object())
			{
				frag.read(elm["name"], elm);
			}
			else
			{
				frag.read(elm);
			}
		}
	}

	void VaryingDefinition::write(nlohmann::ordered_json& json) const noexcept
	{
		vertex.write(json[_vertexJsonKey]);
		auto& fragmentJson = json[_fragmentJsonKey];
		for (auto& attr : fragment)
		{
			nlohmann::json attrJson;
			auto key = attr.write(attrJson);
			fragmentJson.emplace(key, attrJson);
		}
	}

	const std::string VaryingDefinition::_bgfxVertMarker = "a_";
	const std::string VaryingDefinition::_bgfxFragMarker = "v_";
	const std::string VaryingDefinition::_bgfxInstMarker = "i_";
	const std::string VaryingDefinition::_bgfxInstrEnd = ";";
	const std::string VaryingDefinition::_bgfxComment = "//";

	void VaryingDefinition::readBgfx(std::istream& is)
	{
		std::string line;
		auto& varyingDefAttrs = getAttrs();
		while (std::getline(is, line))
		{
			StringUtils::trim(line);
			if (line.starts_with(_bgfxComment))
			{
				continue;
			}
			if (line.ends_with(_bgfxInstrEnd))
			{
				line = line.substr(0, line.size() - _bgfxInstrEnd.size());
			}
			auto words = StringUtils::splitWords(line);
			if (words.size() < 4)
			{
				continue;
			}
			std::optional<int> optNum;
			if (words[0] == "float")
			{
				optNum = 1;
			}
			else
			{
				optNum = StringUtils::getIntSuffix(words[0], "vec");
			}
			if (!optNum)
			{
				continue;
			}
			uint8_t num = optNum.value();
			auto itr = varyingDefAttrs.find(words[3]);
			if (itr == varyingDefAttrs.end())
			{
				continue;
			}
			auto attr = itr->second;
			auto& name = words[1];
			if (name.starts_with(_bgfxVertMarker))
			{
				auto& elm = vertex.attributes.emplace_back(attr);
				elm.num = num;
				if (attr >= bgfx::Attrib::Color0 && attr <= bgfx::Attrib::Color3)
				{
					elm.type = bgfx::AttribType::Uint8;
					elm.normalize = true;
				}
			}
			if (name.starts_with(_bgfxFragMarker))
			{
				auto& elm = fragment.emplace_back(attr);
				elm.num = num;
				// TODO: read default value
			}

			// TODO: support instance attributes
			// vec4 i_data0 : TEXCOORD7;
			// https://bkaradzic.github.io/bgfx/tools.html#vertex-shader-attributes
		}
	}

	void VaryingDefinition::writeBgfx(std::ostream& out, const AttribGroups& disabledGroups) const noexcept
	{
		auto writeLine = [&out](unsigned int num, const std::string& marker, bgfx::Attrib::Enum attrib, bgfx::Attrib::Enum type = bgfx::Attrib::Count)
		{
			if (num > 1)
			{
				out << "vec" << num;
			}
			else
			{
				out << "float";
			}
			auto name = AttribUtils::getBgfxName(attrib);
			auto typeStr = type == bgfx::Attrib::Count ? name : AttribUtils::getBgfxName(type);
			out << " " << marker << StringUtils::toLower(name) << " : " << StringUtils::toUpper(typeStr);
		};

		auto firstEmptyTexture = bgfx::Attrib::Count;
		for (auto i = bgfx::Attrib::TexCoord0; i < bgfx::Attrib::TexCoord7; i = (bgfx::Attrib::Enum)(i + 1))
		{
			auto itr = std::find_if(fragment.begin(), fragment.end(), [i](auto& elm) { return elm.attrib == i; });
			if (itr == fragment.end())
			{
				firstEmptyTexture = i;
				break;
			}
		}

		for (auto& attr : fragment)
		{
			if (attr.inGroups(disabledGroups))
			{
				continue;
			}
			auto type = attr.attrib;
			if (type == bgfx::Attrib::Position)
			{
				// using position type in fragment attributes fails to compile
				type = firstEmptyTexture;
			}
			writeLine(attr.num, _bgfxFragMarker, attr.attrib, type);
			if (!attr.defaultValue.empty())
			{
				auto size = attr.defaultValue.size();
				if (size == 1)
				{
					out << attr.defaultValue[0];
				}
				else
				{
					out << " = vec" << size << "(";
					out << StringUtils::join(", ", attr.defaultValue) << ")";
				}
			}
			out << _bgfxInstrEnd << std::endl;
		}
		out << std::endl;
		for (auto& attr : vertex.attributes)
		{
			if (attr.inGroups(disabledGroups))
			{
				continue;
			}
			writeLine(attr.num, _bgfxVertMarker, attr.attrib);
			out << _bgfxInstrEnd << std::endl;
		}
	}

	void VaryingDefinition::writeBgfx(std::ostream& out, const AttribDefines& defines) const noexcept
	{
		AttribGroups disabledGroups;
		AttribUtils::getDisabledGroups(defines, disabledGroups);
		writeBgfx(out, disabledGroups);
	}

	VertexLayout::VertexLayout(const std::vector<VertexAttribute>& attribs) noexcept
		: attributes(attribs)
	{
	}

	VertexLayout::VertexLayout(const bgfx::VertexLayout& layout) noexcept
	{
		setBgfx(layout);
	}

	bgfx::VertexLayout VertexLayout::getBgfx(const AttribGroups& disabledGroups) const noexcept
	{
		bgfx::VertexLayout layout;
		layout.begin();
		for (auto& attr : attributes)
		{
			auto enabled = true;
			for (auto& group : disabledGroups)
			{
				if (attr.inGroup(group))
				{
					enabled = false;
				}
			}
			if (!enabled)
			{
				continue;
			}
			attr.addTo(layout);
		}
		layout.end();
		return layout;
	}

	bgfx::VertexLayout VertexLayout::getBgfx(const AttribDefines& defines) const noexcept
	{
		AttribGroups disabledGroups;
		AttribUtils::getDisabledGroups(defines, disabledGroups);
		return getBgfx(disabledGroups);
	}

	VertexLayout& VertexLayout::operator=(const bgfx::VertexLayout& layout) noexcept
	{
		setBgfx(layout);
		return *this;
	}

	VertexLayout::operator bgfx::VertexLayout() const noexcept
	{
		return getBgfx(AttribGroups{});
	}

	void VertexLayout::setBgfx(const bgfx::VertexLayout& layout) noexcept
	{
		std::map<uint16_t, VertexAttribute> items;
		for (auto i = 0; i < bgfx::Attrib::Count; i++)
		{
			const auto attr = static_cast<bgfx::Attrib::Enum>(i);
			if (!layout.has(attr))
			{
				continue;
			}
			auto offset = layout.getOffset(attr);
			VertexAttribute val;
			layout.decode(val.attrib, val.num, val.type, val.normalize, val.asInt);
			items.emplace(offset, val);
		}
		attributes.clear();
		for (auto& [offset, val] : items)
		{
			attributes.push_back(val);
		}
	}

	void VertexLayout::read(const nlohmann::ordered_json& json)
	{
		if (json.is_object())
		{
			for (auto& elm : json.items())
			{
				attributes.emplace_back().read(elm.key(), elm.value());
			}
			return;
		}
		if (!json.is_array())
		{
			return;
		}
		for (auto& elm : json)
		{
			auto& attrib = attributes.emplace_back();
			if (elm.is_object())
			{
				attrib.read(elm["name"], elm);
			}
			else
			{
				attrib.read(elm);
			}
		}
	}

	void VertexLayout::write(nlohmann::ordered_json& json) const noexcept
	{
		for (auto& attr : attributes)
		{
			nlohmann::json attrJson;
			auto key = attr.write(attrJson);
			json.emplace(key, attrJson);
		}
	}
}

std::string to_string(const bgfx::VertexLayout& layout) noexcept
{
	nlohmann::ordered_json json;
	darmok::VertexLayout(layout).write(json);
	std::stringstream ss;
	ss << "VertexLayout:" << std::endl;
	ss << json.dump(2) << std::endl;
	return ss.str();
}

std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout) noexcept
{
	return out << to_string(layout);
}