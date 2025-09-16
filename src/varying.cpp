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
	namespace AttribUtils
	{
		size_t getDisabledGroups(const AttribDefines& defines, AttribGroups& disabledGroups) noexcept
		{
			size_t count = 0;
			return count;
		}

		[[nodiscard]] AttribGroup getGroup(bgfx::Attrib::Enum attrib) noexcept
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

		bool inGroup(bgfx::Attrib::Enum attrib, AttribGroup group) noexcept
		{
			return getGroup(attrib) == group;
		}

		bool inGroups(bgfx::Attrib::Enum attrib, const AttribGroups& groups) noexcept
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

		std::optional<int> getIntSuffix(std::string_view name, std::string_view prefix) noexcept
		{
			if (!StringUtils::startsWith(name, prefix))
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

		bgfx::Attrib::Enum getBgfx(const std::string_view name) noexcept
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
			auto count = getIntSuffix(sname, "color");
			if (count != std::nullopt)
			{
				return (bgfx::Attrib::Enum)((int)bgfx::Attrib::Color0 + count.value());
			}
			count = getIntSuffix(sname, "texcoord");
			if (count == std::nullopt)
			{
				count = getIntSuffix(sname, "tex_coord");
			}
			if (count != std::nullopt)
			{
				return (bgfx::Attrib::Enum)((int)bgfx::Attrib::TexCoord0 + count.value());
			}

			return bgfx::Attrib::Count;
		}

		bgfx::AttribType::Enum getBgfxType(const std::string_view name) noexcept
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

		std::string getBgfxName(bgfx::Attrib::Enum val) noexcept
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

		std::string getBgfxTypeName(bgfx::AttribType::Enum val) noexcept
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
	}

	ConstVertexAttributeWrapper::ConstVertexAttributeWrapper(const Definition& def)
		: _def{ def }
	{
	}

	void ConstVertexAttributeWrapper::addToBgfx(bgfx::VertexLayout& layout) noexcept
	{
		auto bgfx = bgfx::Attrib::Enum(_def.bgfx());
		auto bgfxType = bgfx::AttribType::Enum(_def.bgfx_type());
		layout.add(bgfx, _def.num(), bgfxType, _def.normalize(), _def.as_int());
	}

	VertexAttributeWrapper::VertexAttributeWrapper(Definition& def)
		: ConstVertexAttributeWrapper{ def }
		, _def{ def }
	{
	}

	expected<void, std::string> VertexAttributeWrapper::read(const std::string& key) noexcept
	{
		auto bgfx = AttribUtils::getBgfx(key);
		if (bgfx == bgfx::Attrib::Count)
		{
			return unexpected<std::string>("invalid key: " + key);
		}
		_def.set_bgfx(protobuf::Bgfx::Attrib(bgfx));
		_def.set_num(3);
		_def.set_bgfx_type(protobuf::Bgfx::Float);

		auto group = AttribUtils::getGroup(bgfx);
		if (group == AttribGroup::Color)
		{
			_def.set_num(4);
			_def.set_bgfx_type(protobuf::Bgfx::Uint8);
			_def.set_normalize(true);
		}
		else if (group == AttribGroup::Texture)
		{
			_def.set_num(2);
		}
		else if (group == AttribGroup::Skinning)
		{
			_def.set_num(4);
		}
		return {};
	}

	expected<void, std::string>VertexAttributeWrapper::read(const std::string& key, const nlohmann::json& json) noexcept
	{
		auto bgfx = AttribUtils::getBgfx(key);
		if (bgfx == bgfx::Attrib::Count)
		{
			return unexpected<std::string>("invalid key: " + key);
		}
		_def.set_bgfx(protobuf::Bgfx::Attrib(bgfx));
		if (json.contains("type"))
		{
			auto typeStr = json["type"].get<std::string_view>();
			auto bgfxType = AttribUtils::getBgfxType(typeStr);
			if (bgfxType == bgfx::AttribType::Count)
			{
				return unexpected<std::string>("invalid type: " + key);
			}
			_def.set_bgfx_type(protobuf::Bgfx::AttribType(bgfxType));
		}
		if (json.contains("num"))
		{
			_def.set_num(json["num"]);
		}
		if (json.contains("normalize"))
		{
			_def.set_normalize(json["normalize"]);
		}
		if (json.contains("int"))
		{
			_def.set_as_int(json["int"]);
		}
		return {};
	}

	FragmentAttributeWrapper::FragmentAttributeWrapper(Definition& def)
		: _def{ def }
	{
	}

	expected<void, std::string> FragmentAttributeWrapper::read(const std::string& key) noexcept
	{
		auto bgfx = AttribUtils::getBgfx(key);
		if (bgfx == bgfx::Attrib::Count)
		{
			_def.set_name(key);
		}
		else
		{
			_def.set_name(AttribUtils::getBgfxName(bgfx));
		}
		_def.set_bgfx(protobuf::Bgfx::Attrib(bgfx));
		auto group = AttribUtils::getGroup(bgfx);
		if (group == AttribGroup::Color)
		{
			_def.set_num(4);
			auto def = _def.mutable_default_value();
			def->set_x(1);
			def->set_y(1);
			def->set_z(1);
			def->set_w(1);
		}
		else if (group == AttribGroup::Texture)
		{
			_def.set_num(2);
			auto def = _def.mutable_default_value();
			def->set_x(0);
			def->set_y(0);
		}
		else if (bgfx == bgfx::Attrib::Indices)
		{
			_def.set_num(4);
			auto def = _def.mutable_default_value();
			def->set_x(-1);
			def->set_y(-1);
			def->set_z(-1);
			def->set_w(-1);
		}
		else if (bgfx == bgfx::Attrib::Weight)
		{
			_def.set_num(4);
			auto def = _def.mutable_default_value();
			def->set_x(1);
			def->set_y(0);
			def->set_z(0);
			def->set_w(0);
		}
		else if (bgfx == bgfx::Attrib::Normal)
		{
			_def.set_num(3);
			auto def = _def.mutable_default_value();
			def->set_x(0);
			def->set_y(0);
			def->set_z(1);
		}
		else if (bgfx == bgfx::Attrib::Tangent)
		{
			_def.set_num(3);
			auto def = _def.mutable_default_value();
			def->set_x(1);
			def->set_y(0);
			def->set_z(0);
		}
		else if (bgfx == bgfx::Attrib::Bitangent)
		{
			_def.set_num(3);
			auto def = _def.mutable_default_value();
			def->set_x(0);
			def->set_y(1);
			def->set_z(0);
		}
		else
		{
			_def.set_num(3);
			auto def = _def.mutable_default_value();
			def->set_x(0);
			def->set_y(0);
			def->set_z(0);
		}
		return {};
	}

	expected<void, std::string> FragmentAttributeWrapper::read(const std::string& key, const nlohmann::json& json) noexcept
	{
		auto result = read(key);
		if (!result)
		{
			return result;
		}
		auto type = json.type();

		auto setDefaultArray = [this](const nlohmann::json& json)
			{
				auto def = _def.mutable_default_value();
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
				_def.set_num(size);
				return size;
			};

		if (type == nlohmann::json::value_t::array)
		{
			auto defaultValue = json.get<std::vector<float>>();
			setDefaultArray(defaultValue);
		}
		if (type != nlohmann::json::value_t::object)
		{
			_def.set_num(json);
			return {};
		}
		if (json.contains("default"))
		{
			auto defaultValue = json["default"].get<std::vector<float>>();
			setDefaultArray(defaultValue);
		}
		if (json.contains("num"))
		{
			_def.set_num(json["num"]);
		}
		return {};
	}

	ConstVertexLayoutWrapper::ConstVertexLayoutWrapper(const Definition& def)
		: _def{ def }
	{
	}

	bgfx::VertexLayout ConstVertexLayoutWrapper::getBgfx(const AttribGroups& disabledGroups) noexcept
	{
		bgfx::VertexLayout bgfxLayout;
		bgfxLayout.begin();
		for (auto& attib : _def.attributes())
		{
			auto bgfx = bgfx::Attrib::Enum(attib.bgfx());
			if (AttribUtils::inGroups(bgfx, disabledGroups))
			{
				continue;
			}
			ConstVertexAttributeWrapper{ attib }.addToBgfx(bgfxLayout);
		}
		bgfxLayout.end();
		return bgfxLayout;
	}

	bgfx::VertexLayout ConstVertexLayoutWrapper::getBgfx(const AttribDefines& defines) noexcept
	{
		AttribGroups disabledGroups;
		AttribUtils::getDisabledGroups(defines, disabledGroups);
		return getBgfx(disabledGroups);
	}

	VertexLayoutWrapper::VertexLayoutWrapper(Definition& def)
		: ConstVertexLayoutWrapper(def)
		, _def{ def }
	{
	}

	void VertexLayoutWrapper::read(const bgfx::VertexLayout& bgfxLayout) noexcept
	{
		std::map<uint16_t, Attribute> items;
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
			auto& attrib = items[offset];
			attrib.set_bgfx(protobuf::Bgfx::Attrib(bgfxAttrib));
			attrib.set_num(num);
			attrib.set_bgfx_type(protobuf::Bgfx::AttribType(bgfxAttribType));
			attrib.set_normalize(normalize);
			attrib.set_as_int(asInt);
		}
		for (auto& [_, attrib] : items)
		{
			*_def.add_attributes() = attrib;
		}
	}

	expected<void, std::string> VertexLayoutWrapper::read(const nlohmann::ordered_json& json) noexcept
	{
		auto type = json.type();
		if (type == nlohmann::json::value_t::object)
		{
			for (auto& elm : json.items())
			{
				auto& attrib = *_def.add_attributes();
				VertexAttributeWrapper wrapper{ attrib };
				auto result = wrapper.read(elm.key(), elm.value());
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
			auto& attrib = *_def.add_attributes();
			VertexAttributeWrapper wrapper{ attrib };

			if (elm.is_object())
			{
				auto result = wrapper.read(elm["name"], elm);
				if (!result)
				{
					return result;
				}
			}
			else
			{
				auto result = wrapper.read(elm);
				if (!result)
				{
					return result;
				}
			}
		}
		return {};
	}

	ConstFragmentLayoutWrapper::ConstFragmentLayoutWrapper(const Definition& def)
		: _def{ def }
	{
	}

	bgfx::Attrib::Enum ConstFragmentLayoutWrapper::getUnusedAttrib(const std::vector<bgfx::Attrib::Enum>& used) noexcept
	{
		for (auto i = bgfx::Attrib::TexCoord0; i < bgfx::Attrib::TexCoord7; i = (bgfx::Attrib::Enum)(i + 1))
		{
			auto itr1 = std::find(used.begin(), used.end(), i);
			if (itr1 != used.end())
			{
				continue;
			}
			auto& attributes = _def.attributes();
			auto itr2 = std::find_if(attributes.begin(), attributes.end(), [i](auto& elm) { return elm.bgfx() == i; });
			if (itr2 == attributes.end())
			{
				return i;
			}
		}
		return bgfx::Attrib::Count;
	}

	FragmentLayoutWrapper::FragmentLayoutWrapper(Definition& def)
		: ConstFragmentLayoutWrapper(def)
		, _def{ def }
	{
	}

	expected<void, std::string> FragmentLayoutWrapper::read(const nlohmann::ordered_json& json) noexcept
	{
		auto type = json.type();
		if (type == nlohmann::json::value_t::object)
		{
			for (auto& elm : json.items())
			{
				auto& attrib = *_def.add_attributes();
				FragmentAttributeWrapper wrapper{ attrib };
				auto result = wrapper.read(elm.key(), elm.value());
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
			auto& attrib = *_def.add_attributes();
			auto type = elm.type();
			FragmentAttributeWrapper wrapper{ attrib };
			if (type == nlohmann::json::value_t::object)
			{
				auto result = wrapper.read(elm["name"], elm);
				if (!result)
				{
					return result;
				}
			}
			else if (type == nlohmann::json::value_t::array)
			{
				auto result = wrapper.read(elm[0], elm[1]);
				if (!result)
				{
					return result;
				}
			}
			else
			{
				auto result = wrapper.read(elm);
				if (!result)
				{
					return result;
				}
			}
		}
		return {};
	}

	ConstVaryingDefinitionWrapper::ConstVaryingDefinitionWrapper(const Definition& def)
		: _def{ def }
	{
	}

	void ConstVaryingDefinitionWrapper::writeBgfx(std::ostream& out, const AttribGroups& disabledGroups) noexcept
	{
		static const std::string instrEnd = ";";
		std::vector<bgfx::Attrib::Enum> usedTypes;
		for (auto& attrib : _def.fragment().attributes())
		{
			auto bgfxAttrib = bgfx::Attrib::Enum(attrib.bgfx());
			if (AttribUtils::inGroups(bgfxAttrib, disabledGroups))
			{
				continue;
			}
			if (bgfxAttrib == bgfx::Attrib::Position || bgfxAttrib == bgfx::Attrib::Count)
			{
				ConstFragmentLayoutWrapper wrapper{ _def.fragment() };
				bgfxAttrib = wrapper.getUnusedAttrib(usedTypes);
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
				else if (size > 0)
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
		for (auto& attrib : _def.vertex().attributes())
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

	void ConstVaryingDefinitionWrapper::writeBgfx(std::ostream& out, const AttribDefines& defines) noexcept
	{
		AttribGroups disabledGroups;
		AttribUtils::getDisabledGroups(defines, disabledGroups);
		writeBgfx(out, disabledGroups);
	}

	void ConstVaryingDefinitionWrapper::writeBgfx(const std::filesystem::path& path) noexcept
	{
		std::ofstream out{ path };
		writeBgfx(out);
	}

	std::string ConstVaryingDefinitionWrapper::getBgfxTypeName(bgfx::Attrib::Enum val) noexcept
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

	std::string ConstVaryingDefinitionWrapper::getBgfxVarTypeName(uint8_t num) noexcept
	{
		if (num <= 1)
		{
			return "float";
		}
		return "vec" + std::to_string(num);
	}

	VaryingDefinitionWrapper::VaryingDefinitionWrapper(Definition& def)
		: ConstVaryingDefinitionWrapper(def)
		, _def{ def }
	{
	}

	expected<void, std::string> VaryingDefinitionWrapper::read(const nlohmann::ordered_json& json) noexcept
	{
		constexpr std::string_view vertexJsonKey = "vertex";
		constexpr std::string_view fragmentJsonKey = "fragment";

		for (auto& elm : json.items())
		{
			if (elm.key().starts_with(vertexJsonKey))
			{
				VertexLayoutWrapper wrapper{ *_def.mutable_vertex() };
				auto result = wrapper.read(elm.value());
				if (!result)
				{
					return result;
				}
			}
			else if (elm.key().starts_with(fragmentJsonKey))
			{
				FragmentLayoutWrapper wrapper{ *_def.mutable_fragment() };
				auto result = wrapper.read(elm.value());
				if (!result)
				{
					return result;
				}
			}
		}
		return {};
	}

	expected<void, std::string> VaryingDefinitionWrapper::read(const std::filesystem::path& path) noexcept
	{
		auto [input, format] = protobuf::createInputStream(path);
		if (format == protobuf::Format::Json)
		{
			return read(nlohmann::ordered_json::parse(input));
		}
		return protobuf::read(_def, input, format);
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
	darmok::protobuf::VertexLayout layout;
	darmok::VertexLayoutWrapper{ layout }.read(bgfxLayout);
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