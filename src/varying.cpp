#include <darmok/varying.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/utils.hpp>
#include <sstream>
#include <fstream>
#include <map>
#include <cereal/types/vector.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/archives/portable_binary.hpp>

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
			auto i = toUnderlying(val) - toUnderlying(bgfx::Attrib::Color0);
			return std::string("color") + std::to_string(i);
		}
		if (val >= bgfx::Attrib::TexCoord0 && val <= bgfx::Attrib::TexCoord7)
		{
			auto i = toUnderlying(val) - toUnderlying(bgfx::Attrib::TexCoord0);
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

	VaryingDefinitionFormat AttribUtils::getPathFormat(const std::filesystem::path& path) noexcept
	{
		auto ext = path.extension();
		if (ext == ".json")
		{
			return VaryingDefinitionFormat::Json;
		}
		if (ext == ".xml")
		{
			return VaryingDefinitionFormat::Xml;
		}
		return VaryingDefinitionFormat::Binary;
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
			name = key;
		}
		else
		{
			name = AttribUtils::getBgfxName(attrib);
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
		else if (attrib == bgfx::Attrib::Normal)
		{
			num = 3;
			defaultValue = { 0.F, 0.F, 1.F };
		}
		else if (attrib == bgfx::Attrib::Tangent)
		{
			num = 3;
			defaultValue = { 1.F, 0.F, 0.F };
		}
		else if (attrib == bgfx::Attrib::Bitangent)
		{
			num = 3;
			defaultValue = { 0.F, 1.F, 0.F };
		}
		else
		{
			num = 3;
			defaultValue = { 0.F, 0.F, 0.F };
		}
	}

	void FragmentAttribute::read(const std::string& key, const nlohmann::json& json)
	{
		read(key);
		auto type = json.type();
		if(type == nlohmann::json::value_t::array)
		{
			defaultValue = json.get<std::vector<float>>();
			num = defaultValue.size();
		}
		if (type != nlohmann::json::value_t::object)
		{
			num = json;
			defaultValue.resize(num);
			return;
		}
		if (json.contains("default"))
		{
			defaultValue = json["default"].get<std::vector<float>>();
		}
		if (json.contains("num"))
		{
			num = json["num"].get<uint8_t>();
			defaultValue.resize(num);
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

	const std::string VaryingDefinition::_vertexJsonKey = "vertex";
	const std::string VaryingDefinition::_fragmentJsonKey = "fragment";

	void VaryingDefinition::read(const nlohmann::ordered_json& json)
	{
		for (auto& elm : json.items())
		{
			if (elm.key().starts_with(_vertexJsonKey))
			{
				vertex.read(elm.value());
			}
			else if (elm.key().starts_with(_fragmentJsonKey))
			{
				fragment.read(elm.value());
			}
		}
	}

	void VaryingDefinition::write(nlohmann::ordered_json& json) const noexcept
	{
		vertex.write(json[_vertexJsonKey]);
		fragment.write(json[_fragmentJsonKey]);
	}

	void VaryingDefinition::read(const std::filesystem::path& path)
	{
		std::ifstream in(path);
		read(in, AttribUtils::getPathFormat(path));
	}

	void VaryingDefinition::write(const std::filesystem::path& path) const noexcept
	{
		std::ofstream out(path);
		write(out, AttribUtils::getPathFormat(path));
	}

	void VaryingDefinition::read(std::istream& in, Format format)
	{
		if (format == Format::Json)
		{
			auto json = nlohmann::ordered_json::parse(in);
			read(json);
		}
		else if (format == Format::Xml)
		{
			cereal::XMLInputArchive archive(in);
			archive(*this);
		}
		else
		{
			cereal::PortableBinaryInputArchive archive(in);
			archive(*this);
		}
	}

	void VaryingDefinition::write(std::ostream& out, Format format) const noexcept
	{
		if (format == Format::Json)
		{
			auto json = nlohmann::ordered_json::object();
			write(json);
			out << json.dump(2);
		}
		else if (format == Format::Xml)
		{
			cereal::XMLOutputArchive archive(out);
			archive(*this);
		}
		else
		{
			cereal::PortableBinaryOutputArchive archive(out);
			archive(*this);
		}
	}

	std::string VaryingDefinition::getBgfxTypeName(bgfx::Attrib::Enum val) noexcept
	{
		if (val == bgfx::Attrib::Indices)
		{
			return "BLENDINDICES";
		}
		if (val == bgfx::Attrib::Weight)
		{
			return "BLENDWEIGHT";
		}
		if (val == bgfx::Attrib::Bitangent)
		{
			return "BINORMAL";
		}
		return StringUtils::toUpper(AttribUtils::getBgfxName(val));
	}

	std::string VaryingDefinition::getBgfxVarTypeName(uint8_t num) noexcept
	{
		if (num <= 1)
		{
			return "float";
		}
		return "vec" + std::to_string(num);
	}

	void VaryingDefinition::writeBgfx(std::ostream& out, const AttribGroups& disabledGroups) const noexcept
	{
		static const std::string instrEnd = ";";
		std::vector<bgfx::Attrib::Enum> usedTypes;
		for (auto& attr : fragment)
		{
			if (attr.inGroups(disabledGroups))
			{
				continue;
			}
			auto type = attr.attrib;
			if (type == bgfx::Attrib::Position || type == bgfx::Attrib::Count)
			{
				type = fragment.getUnusedAttrib(usedTypes);
			}
			usedTypes.push_back(type);
			out << getBgfxVarTypeName(attr.num) << " v_" << attr.name;
			out << " : " << getBgfxTypeName(type);
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
			out << instrEnd << std::endl;
		}
		out << std::endl;
		for (auto& attr : vertex)
		{
			if (attr.inGroups(disabledGroups))
			{
				continue;
			}
			out << getBgfxVarTypeName(attr.num) << " a_" << AttribUtils::getBgfxName(attr.attrib);
			out << " : " << getBgfxTypeName(attr.attrib) << instrEnd << std::endl;
		}

		// TODO: support instance attributes
		// vec4 i_data0 : TEXCOORD7;
		// https://bkaradzic.github.io/bgfx/tools.html#vertex-shader-attributes
	}

	void VaryingDefinition::writeBgfx(std::ostream& out, const AttribDefines& defines) const noexcept
	{
		AttribGroups disabledGroups;
		AttribUtils::getDisabledGroups(defines, disabledGroups);
		writeBgfx(out, disabledGroups);
	}

	VertexLayout::VertexLayout(const std::vector<VertexAttribute>& attribs) noexcept
		: _attributes(attribs)
	{
	}

	VertexLayout::VertexLayout(const bgfx::VertexLayout& layout) noexcept
	{
		setBgfx(layout);
	}

	bool VertexLayout::empty() const noexcept
	{
		return _attributes.empty();
	}

	bool VertexLayout::has(bgfx::Attrib::Enum attrib) const noexcept
	{
		auto itr = std::find_if(_attributes.begin(), _attributes.end(), [attrib](auto& elm) { return elm.attrib == attrib; });
		return itr != _attributes.end();
	}

	VertexLayout::ConstIterator VertexLayout::begin() const noexcept
	{
		return _attributes.begin();
	}

	VertexLayout::ConstIterator VertexLayout::end() const noexcept
	{
		return _attributes.end();
	}

	bgfx::VertexLayout VertexLayout::getBgfx(const AttribGroups& disabledGroups) const noexcept
	{
		bgfx::VertexLayout layout;
		layout.begin();
		for (auto& attr : _attributes)
		{
			if (attr.inGroups(disabledGroups))
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
		return getBgfx();
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
		_attributes.clear();
		for (auto& [offset, val] : items)
		{
			_attributes.push_back(val);
		}
	}

	void VertexLayout::read(const std::filesystem::path& path)
	{
		std::ifstream in(path);
		read(in, AttribUtils::getPathFormat(path));
	}

	void VertexLayout::write(const std::filesystem::path& path) const noexcept
	{
		std::ofstream out(path);
		write(out, AttribUtils::getPathFormat(path));
	}

	void VertexLayout::read(std::istream& in, Format format)
	{
		if (format == Format::Json)
		{
			auto json = nlohmann::ordered_json::parse(in);
			read(json);
		}
		else if (format == Format::Xml)
		{
			cereal::XMLInputArchive archive(in);
			archive(*this);
		}
		else
		{
			cereal::PortableBinaryInputArchive archive(in);
			archive(*this);
		}
	}

	void VertexLayout::write(std::ostream& out, Format format) const noexcept
	{
		if (format == Format::Json)
		{
			auto json = nlohmann::ordered_json::object();
			write(json);
			out << json.dump(2);
		}
		else if (format == Format::Xml)
		{
			cereal::XMLOutputArchive archive(out);
			archive(*this);
		}
		else
		{
			cereal::PortableBinaryOutputArchive archive(out);
			archive(*this);
		}
	}

	void VertexLayout::read(const nlohmann::ordered_json& json)
	{
		auto type = json.type();
		if (type == nlohmann::json::value_t::object)
		{
			for (auto& elm : json.items())
			{
				_attributes.emplace_back().read(elm.key(), elm.value());
			}
			return;
		}
		if (type != nlohmann::json::value_t::array)
		{
			return;
		}
		for (auto& elm : json)
		{
			auto& attrib = _attributes.emplace_back();
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
		for (auto& attr : _attributes)
		{
			nlohmann::json attrJson;
			auto key = attr.write(attrJson);
			json.emplace(key, attrJson);
		}
	}

	FragmentLayout::FragmentLayout(const std::vector<Attribute>& attribs) noexcept
		: _attributes(attribs)
	{
	}

	bool FragmentLayout::empty() const noexcept
	{
		return _attributes.empty();
	}

	bool FragmentLayout::has(bgfx::Attrib::Enum attrib) const noexcept
	{
		auto itr = std::find_if(_attributes.begin(), _attributes.end(), [attrib](auto& elm) { return elm.attrib == attrib; });
		return itr != _attributes.end();
	}

	FragmentLayout::ConstIterator FragmentLayout::begin() const noexcept
	{
		return _attributes.begin();
	}

	FragmentLayout::ConstIterator FragmentLayout::end() const noexcept
	{
		return _attributes.end();
	}

	bgfx::Attrib::Enum FragmentLayout::getUnusedAttrib(const std::vector<bgfx::Attrib::Enum>& used) const noexcept
	{
		for (auto i = bgfx::Attrib::TexCoord0; i < bgfx::Attrib::TexCoord7; i = (bgfx::Attrib::Enum)(i + 1))
		{
			auto itr1 = std::find(used.begin(), used.end(), i);
			if (itr1 != used.end())
			{
				continue;
			}
			auto itr2 = std::find_if(_attributes.begin(), _attributes.end(), [i](auto& elm) { return elm.attrib == i; });
			if (itr2 == _attributes.end())
			{
				return i;
			}
		}
		return bgfx::Attrib::Count;
	}

	void FragmentLayout::read(const std::filesystem::path& path)
	{
		std::ifstream in(path);
		read(in, AttribUtils::getPathFormat(path));
	}

	void FragmentLayout::write(const std::filesystem::path& path) const noexcept
	{
		std::ofstream out(path);
		write(out, AttribUtils::getPathFormat(path));
	}

	void FragmentLayout::read(std::istream& in, Format format)
	{
		if (format == Format::Json)
		{
			auto json = nlohmann::ordered_json::parse(in);
			read(json);
		}
		else if (format == Format::Xml)
		{
			cereal::XMLInputArchive archive(in);
			archive(*this);
		}
		else
		{
			cereal::PortableBinaryInputArchive archive(in);
			archive(*this);
		}
	}

	void FragmentLayout::write(std::ostream& out, Format format) const noexcept
	{
		if (format == Format::Json)
		{
			auto json = nlohmann::ordered_json::object();
			write(json);
			out << json.dump(2);
		}
		else if (format == Format::Xml)
		{
			cereal::XMLOutputArchive archive(out);
			archive(*this);
		}
		else
		{
			cereal::PortableBinaryOutputArchive archive(out);
			archive(*this);
		}
	}

	void FragmentLayout::read(const nlohmann::ordered_json& json)
	{
		auto type = json.type();
		if (type == nlohmann::json::value_t::object)
		{
			for (auto& elm : json.items())
			{
				_attributes.emplace_back().read(elm.key(), elm.value());
			}
			return;
		}
		if (type != nlohmann::json::value_t::array)
		{
			return;
		}
		for (auto& elm : json)
		{
			auto& attrib = _attributes.emplace_back();
			auto type = elm.type();
			if (type == nlohmann::json::value_t::object)
			{
				attrib.read(elm["name"], elm);
			}
			else if (type == nlohmann::json::value_t::array)
			{
				attrib.read(elm[0], elm[1]);
			}
			else
			{
				attrib.read(elm);
			}
		}
	}

	void FragmentLayout::write(nlohmann::ordered_json& json) const noexcept
	{
		for (auto& attr : _attributes)
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