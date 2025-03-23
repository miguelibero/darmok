#include <darmok/varying.hpp>
#include <darmok/string.hpp>
#include <darmok/data.hpp>
#include <darmok/data_stream.hpp>
#include <darmok/utils.hpp>
#include <darmok/protobuf.hpp>

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

	bool AttribUtils::inGroup(bgfx::Attrib::Enum attrib, AttribGroup group) noexcept
	{
		return AttribUtils::getGroup(attrib) == group;
	}

	bool AttribUtils::inGroups(bgfx::Attrib::Enum attrib, const AttribGroups& groups) noexcept
	{
		for (auto& group : groups)
		{
			if (inGroup(attrib, group))
			{
				return true;
			}
		}
		return false;
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
		default:
			break;
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
		default:
			break;
		}
		return "";
	}

	expected<void, std::string> VaryingUtils::read(VertexAttribute& attrib, const std::string& key) noexcept
	{
		auto bgfx = AttribUtils::getBgfx(key);
		if (bgfx == bgfx::Attrib::Count)
		{
			return unexpected<std::string>("invalid key: " + key);
		}
		attrib.set_bgfx(protobuf::BgfxAttrib::Enum(bgfx));
		attrib.set_num(3);
		attrib.set_bgfx_type(protobuf::BgfxAttribType::Float);

		auto group = AttribUtils::getGroup(bgfx);
		if (group == AttribGroup::Color)
		{
			attrib.set_num(4);
			attrib.set_bgfx_type(protobuf::BgfxAttribType::Uint8);
			attrib.set_normalize(true);
		}
		else if (group == AttribGroup::Texture)
		{
			attrib.set_num(2);
		}
		else if (group == AttribGroup::Skinning)
		{
			attrib.set_num(4);
		}
		return {};
	}

	expected<void, std::string> VaryingUtils::read(VertexAttribute& attrib, const std::string& key, const nlohmann::json& json) noexcept
	{
		auto bgfx = AttribUtils::getBgfx(key);
		if (bgfx == bgfx::Attrib::Count)
		{
			return unexpected<std::string>("invalid key: " + key);
		}
		attrib.set_bgfx(protobuf::BgfxAttrib::Enum(bgfx));
		if (json.contains("type"))
		{
			auto typeStr = json["type"].get<std::string_view>();
			auto bgfxType = AttribUtils::getBgfxType(typeStr);
			if (bgfxType == bgfx::AttribType::Count)
			{
				return unexpected<std::string>("invalid type: " + key);
			}
			attrib.set_bgfx_type(protobuf::BgfxAttribType::Enum(bgfxType));
		}
		if (json.contains("num"))
		{
			attrib.set_num(json["num"]);
		}
		if (json.contains("normalize"))
		{
			attrib.set_normalize(json["normalize"]);
		}
		if (json.contains("int"))
		{
			attrib.set_as_int(json["int"]);
		}
		return {};
	}

	expected<void, std::string> VaryingUtils::read(FragmentAttribute& attrib, const std::string& key) noexcept
	{
		auto bgfx = AttribUtils::getBgfx(key);
		if (bgfx == bgfx::Attrib::Count)
		{
			attrib.set_name(key);
		}
		else
		{
			attrib.set_name(AttribUtils::getBgfxName(bgfx));
		}
		attrib.set_bgfx(protobuf::BgfxAttrib::Enum(bgfx));
		auto group = AttribUtils::getGroup(bgfx);
		if (group == AttribGroup::Color)
		{
			attrib.set_num(4);
			auto def = attrib.mutable_default_value();
			def->set_x(1);
			def->set_y(1);
			def->set_z(1);
			def->set_w(1);
		}
		else if (group == AttribGroup::Texture)
		{
			attrib.set_num(2);
			auto def = attrib.mutable_default_value();
			def->set_x(0);
			def->set_y(0);
		}
		else if (bgfx == bgfx::Attrib::Indices)
		{
			attrib.set_num(4);
			auto def = attrib.mutable_default_value();
			def->set_x(-1);
			def->set_y(-1);
			def->set_z(-1);
			def->set_w(-1);
		}
		else if (bgfx == bgfx::Attrib::Weight)
		{
			attrib.set_num(4);
			auto def = attrib.mutable_default_value();
			def->set_x(1);
			def->set_y(0);
			def->set_z(0);
			def->set_w(0);
		}
		else if (bgfx == bgfx::Attrib::Normal)
		{
			attrib.set_num(3);
			auto def = attrib.mutable_default_value();
			def->set_x(0);
			def->set_y(0);
			def->set_z(1);
		}
		else if (bgfx == bgfx::Attrib::Tangent)
		{
			attrib.set_num(3);
			auto def = attrib.mutable_default_value();
			def->set_x(1);
			def->set_y(0);
			def->set_z(0);
		}
		else if (bgfx == bgfx::Attrib::Bitangent)
		{
			attrib.set_num(3);
			auto def = attrib.mutable_default_value();
			def->set_x(0);
			def->set_y(1);
			def->set_z(0);
		}
		else
		{
			attrib.set_num(3);
			auto def = attrib.mutable_default_value();
			def->set_x(0);
			def->set_y(0);
			def->set_z(0);
		}
		return {};
	}

	expected<void, std::string> VaryingUtils::read(FragmentAttribute& attrib, const std::string& key, const nlohmann::json& json) noexcept
	{
		auto result = read(attrib, key);
		if (!result)
		{
			return result;
		}
		auto type = json.type();

		auto setDefaultArray = [&attrib](const nlohmann::json& json)
		{
			auto def = attrib.mutable_default_value();
			auto size = json.size();
			if (0 < size)
			{
				def->set_x(json[0]);
			}
			if (1 < size)
			{
				def->set_y(json[1]);
			}
			if (2 < size)
			{
				def->set_z(json[2]);
			}
			if (3 < size)
			{
				def->set_w(json[3]);
			}
			attrib.set_num(size);
			return size;
		};

		if(type == nlohmann::json::value_t::array)
		{
			auto defaultValue = json.get<std::vector<float>>();
			setDefaultArray(defaultValue);
		}
		if (type != nlohmann::json::value_t::object)
		{
			attrib.set_num(json);
			return {};
		}
		if (json.contains("default"))
		{
			auto defaultValue = json["default"].get<std::vector<float>>();
			setDefaultArray(defaultValue);
		}
		if (json.contains("num"))
		{
			attrib.set_num(json["num"]);
		}
		return {};
	}

	void VaryingUtils::addVertexAttribute(bgfx::VertexLayout& layout, const VertexAttribute& attrib) noexcept
	{
		auto bgfx = bgfx::Attrib::Enum(attrib.bgfx());
		auto bgfxType = bgfx::AttribType::Enum(attrib.bgfx_type());
		layout.add(bgfx, attrib.num(), bgfxType, attrib.normalize(), attrib.as_int());
	}

	bgfx::VertexLayout VaryingUtils::getBgfx(const VertexLayout& layout, const AttribGroups& disabledGroups) noexcept
	{
		bgfx::VertexLayout bgfxLayout;
		bgfxLayout.begin();
		for (auto& attib : layout.attributes())
		{
			auto bgfx = bgfx::Attrib::Enum(attib.bgfx());
			if (AttribUtils::inGroups(bgfx, disabledGroups))
			{
				continue;
			}
			addVertexAttribute(bgfxLayout, attib);
		}
		bgfxLayout.end();
		return bgfxLayout;
	}

	bgfx::VertexLayout VaryingUtils::getBgfx(const VertexLayout& layout, const AttribDefines& defines) noexcept
	{
		AttribGroups disabledGroups;
		AttribUtils::getDisabledGroups(defines, disabledGroups);
		return getBgfx(layout, disabledGroups);
	}

	void VaryingUtils::read(VertexLayout& layout, const bgfx::VertexLayout& bgfxLayout) noexcept
	{
		std::map<uint16_t, VertexAttribute> items;
		for (auto i = 0; i < bgfx::Attrib::Count; i++)
		{
			const auto bgfxAttrib = static_cast<bgfx::Attrib::Enum>(i);
			if (!bgfxLayout.has(bgfxAttrib))
			{
				continue;
			}
			auto offset = bgfxLayout.getOffset(bgfxAttrib);
			bgfx::AttribType::Enum bgfxAttribType;
			uint8_t num;
			bool normalize, asInt;
			bgfxLayout.decode(bgfxAttrib, num, bgfxAttribType, normalize, asInt);
			auto& attrib = items.at(offset);
			attrib.set_bgfx(protobuf::BgfxAttrib::Enum(bgfxAttrib));
			attrib.set_num(num);
			attrib.set_bgfx_type(protobuf::BgfxAttribType::Enum(bgfxAttribType));
			attrib.set_normalize(normalize);
			attrib.set_as_int(asInt);
		}
		for (auto& [_, attrib] : items)
		{
			*layout.add_attributes() = attrib;
		}
	}

	expected<void, std::string> VaryingUtils::read(VertexLayout& layout, const nlohmann::ordered_json& json) noexcept
	{
		auto type = json.type();
		if (type == nlohmann::json::value_t::object)
		{
			for (auto& elm : json.items())
			{
				auto& attrib = *layout.add_attributes();
				auto result = read(attrib, elm.key(), elm.value());
				if (!result)
				{
					return result;
				}
			}
			return {};
		}
		if (type != nlohmann::json::value_t::array)
		{
			return {};
		}
		for (auto& elm : json)
		{
			auto& attrib = *layout.add_attributes();
			if (elm.is_object())
			{
				auto result = read(attrib, elm["name"], elm);
				if (!result)
				{
					return result;
				}
			}
			else
			{
				auto result = read(attrib, elm);
				if (!result)
				{
					return result;
				}
			}
		}
		return {};
	}

	bgfx::Attrib::Enum VaryingUtils::getUnusedAttrib(const FragmentLayout& layout, const std::vector<bgfx::Attrib::Enum>& used) noexcept
	{
		for (auto i = bgfx::Attrib::TexCoord0; i < bgfx::Attrib::TexCoord7; i = (bgfx::Attrib::Enum)(i + 1))
		{
			auto itr1 = std::find(used.begin(), used.end(), i);
			if (itr1 != used.end())
			{
				continue;
			}
			auto& attributes = layout.attributes();
			auto itr2 = std::find_if(attributes.begin(), attributes.end(), [i](auto& elm) { return elm.bgfx() == i; });
			if (itr2 == attributes.end())
			{
				return i;
			}
		}
		return bgfx::Attrib::Count;
	}

	expected<void, std::string> VaryingUtils::read(FragmentLayout& layout, const nlohmann::ordered_json& json) noexcept
	{
		auto type = json.type();
		if (type == nlohmann::json::value_t::object)
		{
			for (auto& elm : json.items())
			{
				auto& attrib = *layout.add_attributes();
				auto result = read(attrib, elm.key(), elm.value());
				if (!result)
				{
					return result;
				}
			}
			return {};
		}
		if (type != nlohmann::json::value_t::array)
		{
			return unexpected<std::string>{"invalid json type"};
		}
		for (auto& elm : json)
		{
			auto& attrib = *layout.add_attributes();
			auto type = elm.type();
			if (type == nlohmann::json::value_t::object)
			{
				auto result = read(attrib, elm["name"], elm);
				if (!result)
				{
					return result;
				}
			}
			else if (type == nlohmann::json::value_t::array)
			{
				auto result = read(attrib, elm[0], elm[1]);
				if (!result)
				{
					return result;
				}
			}
			else
			{
				auto result = read(attrib, elm);
				if (!result)
				{
					return result;
				}
			}
		}
		return {};
	}

	expected<void, std::string> VaryingUtils::read(VaryingDefinition& varying, const nlohmann::ordered_json& json) noexcept
	{
		constexpr std::string_view vertexJsonKey = "vertex";
		constexpr std::string_view fragmentJsonKey = "fragment";

		for (auto& elm : json.items())
		{
			if (elm.key().starts_with(vertexJsonKey))
			{
				auto result = read(*varying.mutable_vertex(), elm.value());
				if(!result)
				{ 
					return result;
				}
			}
			else if (elm.key().starts_with(fragmentJsonKey))
			{
				auto result = read(*varying.mutable_fragment(), elm.value());
				if (!result)
				{
					return result;
				}
			}
		}
		return {};
	}

	expected<void, std::string> VaryingUtils::read(VaryingDefinition& varying, const std::filesystem::path& path) noexcept
	{
		auto [input, format] = ProtobufUtils::createInputStream(path);
		if (format == ProtobufFormat::Json)
		{
			return read(varying, nlohmann::ordered_json::parse(input));
		}
		return ProtobufUtils::read(varying, input, format);
	}

	std::string VaryingUtils::getBgfxTypeName(bgfx::Attrib::Enum val) noexcept
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

	std::string VaryingUtils::getBgfxVarTypeName(uint8_t num) noexcept
	{
		if (num <= 1)
		{
			return "float";
		}
		return "vec" + std::to_string(num);
	}

	void VaryingUtils::writeBgfx(const VaryingDefinition& varying, std::ostream& out, const AttribGroups& disabledGroups) noexcept
	{
		static const std::string instrEnd = ";";
		std::vector<bgfx::Attrib::Enum> usedTypes;
		for (auto& attrib : varying.fragment().attributes())
		{
			auto bgfxAttrib = bgfx::Attrib::Enum(attrib.bgfx());
			if (AttribUtils::inGroups(bgfxAttrib, disabledGroups))
			{
				continue;
			}
			if (bgfxAttrib == bgfx::Attrib::Position || bgfxAttrib == bgfx::Attrib::Count)
			{
				bgfxAttrib = getUnusedAttrib(varying.fragment(), usedTypes);
			}
			usedTypes.push_back(bgfxAttrib);
			out << getBgfxVarTypeName(attrib.num()) << " v_" << attrib.name();
			out << " : " << getBgfxTypeName(bgfxAttrib);
			if (attrib.has_default_value())
			{
				auto& def = attrib.default_value();
				auto size = attrib.num();
				if (size == 1)
				{
					out << def.x();
				}
				else if(size > 0)
				{
					out << " = vec" << size << "(" << def.x();
					if (size > 1)
					{
						out << ", " << def.y();
					}
					if (size > 2)
					{
						out << ", " << def.z();
					}
					if (size > 3)
					{
						out << ", " << def.w();
					}
					out << ")";
				}
			}
			out << instrEnd << std::endl;
		}
		out << std::endl;
		for (auto& attrib : varying.vertex().attributes())
		{
			auto bgfxAttrib = bgfx::Attrib::Enum(attrib.bgfx());
			if (AttribUtils::inGroups(bgfxAttrib, disabledGroups))
			{
				continue;
			}
			out << getBgfxVarTypeName(attrib.num()) << " a_" << AttribUtils::getBgfxName(bgfxAttrib);
			out << " : " << getBgfxTypeName(bgfxAttrib) << instrEnd << std::endl;
		}

		// TODO: support instance attributes
		// vec4 i_data0 : TEXCOORD7;
		// https://bkaradzic.github.io/bgfx/tools.html#vertex-shader-attributes
	}

	void VaryingUtils::writeBgfx(const VaryingDefinition& varying, std::ostream& out, const AttribDefines& defines) noexcept
	{
		AttribGroups disabledGroups;
		AttribUtils::getDisabledGroups(defines, disabledGroups);
		writeBgfx(varying, out, disabledGroups);
	}

	void VaryingUtils::writeBgfx(const VaryingDefinition& varying, const std::filesystem::path& path) noexcept
	{
		std::ofstream out(path);
		writeBgfx(varying, out);
	}
}

namespace bgfx
{
	bool operator==(const VertexLayout& a, const VertexLayout& b) noexcept
	{
		return a.m_hash == b.m_hash;
	}

	bool operator!=(const VertexLayout& a, const VertexLayout& b) noexcept
	{
		return !operator==(a, b);
	}
}

std::string to_string(const bgfx::VertexLayout& bgfxLayout) noexcept
{
	darmok::VertexLayout layout;
	darmok::VaryingUtils::read(layout, bgfxLayout);
	return std::string("VertexLayout:\n") + layout.DebugString();
}

std::ostream& operator<<(std::ostream& out, const bgfx::VertexLayout& layout) noexcept
{
	return out << to_string(layout);
}

namespace std
{
	std::size_t hash<bgfx::VertexLayout>::operator()(const bgfx::VertexLayout& key) const noexcept
	{
		return key.m_hash;
	}
}